#ifndef LABANALYSER_MATLAB_TCPCLIENTBARE_CPP
#define LABANALYSER_MATLAB_TCPCLIENTBARE_CPP

#include <winsock2.h>
#include <ws2tcpip.h>

#include <cstdint>
#include <limits>
#include <string>

class WindowsMutex
{
public:
    WindowsMutex() { InitializeCriticalSection(&section_); }
    ~WindowsMutex() { DeleteCriticalSection(&section_); }

    WindowsMutex(const WindowsMutex&) = delete;
    WindowsMutex& operator=(const WindowsMutex&) = delete;

    void lock() { EnterCriticalSection(&section_); }
    void unlock() { LeaveCriticalSection(&section_); }

private:
    CRITICAL_SECTION section_{};
};

class ScopedWindowsLock
{
public:
    explicit ScopedWindowsLock(WindowsMutex& mutex) : mutex_(mutex) { mutex_.lock(); }
    ~ScopedWindowsLock() { mutex_.unlock(); }

private:
    WindowsMutex& mutex_;
};

class TCPClient
{
public:
    TCPClient(const std::string& host, const std::string& port)
    {
        WSADATA winsockData{};
        if (WSAStartup(MAKEWORD(2, 2), &winsockData) != 0) {
            Error = true;
            return;
        }
        winsockStarted_ = true;

        addrinfo hints{};
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;

        addrinfo* addresses = nullptr;
        if (getaddrinfo(host.c_str(), port.c_str(), &hints, &addresses) != 0) {
            Error = true;
            return;
        }

        for (addrinfo* address = addresses; address && !running; address = address->ai_next)
            running = connectWithTimeout(address, 2000);
        freeaddrinfo(addresses);
        Error = !running;
    }

    ~TCPClient()
    {
        close();
        if (winsockStarted_)
            WSACleanup();
    }

    TCPClient(const TCPClient&) = delete;
    TCPClient& operator=(const TCPClient&) = delete;

    std::size_t send(const void* message, uint32_t size)
    {
        if (!message && size != 0)
            return 0;

        ScopedWindowsLock lock(sendMutex_);
        const char* bytes = static_cast<const char*>(message);
        std::size_t sent = 0;
        const ULONGLONG deadline = GetTickCount64() + kIoTimeoutMilliseconds;
        while (sent < size && socket_ != INVALID_SOCKET) {
            if (!waitUntilReady(true, deadline))
                return 0;
            const std::size_t remaining = size - sent;
            const int chunkSize = remaining > static_cast<std::size_t>(std::numeric_limits<int>::max())
                    ? std::numeric_limits<int>::max()
                    : static_cast<int>(remaining);
            const int result = ::send(socket_, bytes + sent, chunkSize, 0);
            if (result == SOCKET_ERROR && WSAGetLastError() == WSAEWOULDBLOCK)
                continue;
            if (result <= 0)
                return 0;
            sent += static_cast<std::size_t>(result);
        }
        return sent;
    }

    std::size_t receive(char* destination, uint32_t size)
    {
        if ((!destination && size != 0) || socket_ == INVALID_SOCKET)
            return 0;

        std::size_t received = 0;
        const ULONGLONG deadline = GetTickCount64() + kIoTimeoutMilliseconds;
        while (received < size) {
            if (!waitUntilReady(false, deadline))
                return 0;
            const int result = recv(socket_, destination + received,
                                    static_cast<int>(size - received), 0);
            if (result == SOCKET_ERROR && WSAGetLastError() == WSAEWOULDBLOCK)
                continue;
            if (result <= 0)
                return 0;
            received += static_cast<std::size_t>(result);
        }
        return received;
    }

    bool running = false;
    bool Error = false;

private:
    bool connectWithTimeout(const addrinfo* address, long timeoutMilliseconds)
    {
        socket_ = socket(address->ai_family, address->ai_socktype, address->ai_protocol);
        if (socket_ == INVALID_SOCKET)
            return false;

        u_long nonBlocking = 1;
        if (ioctlsocket(socket_, FIONBIO, &nonBlocking) != 0) {
            closeSocketOnly();
            return false;
        }

        const int connectResult = connect(socket_, address->ai_addr,
                                          static_cast<int>(address->ai_addrlen));
        if (connectResult == SOCKET_ERROR && WSAGetLastError() != WSAEWOULDBLOCK) {
            closeSocketOnly();
            return false;
        }

        fd_set writable;
        FD_ZERO(&writable);
        FD_SET(socket_, &writable);
        timeval timeout{};
        timeout.tv_sec = timeoutMilliseconds / 1000;
        timeout.tv_usec = (timeoutMilliseconds % 1000) * 1000;
        const int selected = select(0, nullptr, &writable, nullptr, &timeout);

        int socketError = 0;
        int socketErrorSize = sizeof(socketError);
        const bool connected = selected > 0
                && getsockopt(socket_, SOL_SOCKET, SO_ERROR,
                              reinterpret_cast<char*>(&socketError), &socketErrorSize) == 0
                && socketError == 0;

        if (!connected) {
            closeSocketOnly();
            return false;
        }
        return true;
    }

    bool waitUntilReady(bool write, ULONGLONG deadline)
    {
        while (socket_ != INVALID_SOCKET) {
            const ULONGLONG now = GetTickCount64();
            if (now >= deadline)
                return false;

            const ULONGLONG remaining = deadline - now;
            timeval timeout{};
            timeout.tv_sec = static_cast<long>(remaining / 1000);
            timeout.tv_usec = static_cast<long>((remaining % 1000) * 1000);

            fd_set ready;
            fd_set failed;
            FD_ZERO(&ready);
            FD_ZERO(&failed);
            FD_SET(socket_, &ready);
            FD_SET(socket_, &failed);
            const int selected = select(0,
                                        write ? nullptr : &ready,
                                        write ? &ready : nullptr,
                                        &failed,
                                        &timeout);
            if (selected <= 0 || FD_ISSET(socket_, &failed))
                return false;
            if (FD_ISSET(socket_, &ready))
                return true;
        }
        return false;
    }

    void closeSocketOnly()
    {
        if (socket_ == INVALID_SOCKET)
            return;
        closesocket(socket_);
        socket_ = INVALID_SOCKET;
    }

    void close()
    {
        if (socket_ != INVALID_SOCKET)
            shutdown(socket_, SD_BOTH);
        closeSocketOnly();
        running = false;
    }

    SOCKET socket_ = INVALID_SOCKET;
    static constexpr ULONGLONG kIoTimeoutMilliseconds = 2000;
    bool winsockStarted_ = false;
    WindowsMutex sendMutex_;
};

#endif
