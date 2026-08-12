#include "../TCPClient.h"

#include <winsock2.h>
#include <ws2tcpip.h>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <cstring>
#include <exception>
#include <iostream>
#include <mutex>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

namespace {

class SocketRuntime
{
public:
    SocketRuntime()
    {
        WSADATA data{};
        if (WSAStartup(MAKEWORD(2, 2), &data) != 0)
            throw std::runtime_error("WSAStartup failed");
    }

    ~SocketRuntime()
    {
        WSACleanup();
    }
};

void require(bool condition, const char* message)
{
    if (!condition)
        throw std::runtime_error(message);
}

void receiveAll(SOCKET socket, char* destination, std::size_t size)
{
    std::size_t received = 0;
    while (received < size) {
        const int result = recv(socket, destination + received,
                                static_cast<int>(size - received), 0);
        if (result <= 0)
            throw std::runtime_error("server receive failed");
        received += static_cast<std::size_t>(result);
    }
}

void sendAll(SOCKET socket, const char* source, std::size_t size)
{
    std::size_t sent = 0;
    while (sent < size) {
        const int result = send(socket, source + sent,
                                static_cast<int>(size - sent), 0);
        if (result <= 0)
            throw std::runtime_error("server send failed");
        sent += static_cast<std::size_t>(result);
    }
}

uint32_t readUint32(const std::vector<char>& bytes, std::size_t offset)
{
    uint32_t value = 0;
    std::memcpy(&value, bytes.data() + offset, sizeof(value));
    return value;
}

std::vector<char> receiveFrame(SOCKET socket)
{
    uint32_t size = 0;
    receiveAll(socket, reinterpret_cast<char*>(&size), sizeof(size));
    require(size >= 16 && size <= 1024 * 1024, "invalid request size");
    std::vector<char> frame(size);
    std::memcpy(frame.data(), &size, sizeof(size));
    receiveAll(socket, frame.data() + sizeof(size), size - sizeof(size));
    return frame;
}

void verifyFrame(const std::vector<char>& frame,
                 const char* command,
                 const char* id,
                 const std::vector<char>& payload)
{
    const uint32_t idLength = static_cast<uint32_t>(std::strlen(id) + 1);
    require(readUint32(frame, 0) == frame.size(), "totalSize mismatch");
    require(std::memcmp(frame.data() + 4, command, 3) == 0, "command mismatch");
    require(readUint32(frame, 7) == idLength, "idLength mismatch");
    require(readUint32(frame, 11) == payload.size(), "payloadLength mismatch");
    require(std::memcmp(frame.data() + 15, id, idLength) == 0, "ID mismatch");
    require(frame.size() == 15 + idLength + payload.size(), "frame length mismatch");
    if (!payload.empty()) {
        require(std::memcmp(frame.data() + 15 + idLength,
                            payload.data(), payload.size()) == 0,
                "payload mismatch");
    }
}

void sendNumericReply(SOCKET socket, double value)
{
    const char type = 0;
    const uint32_t elements = 1;
    sendAll(socket, &type, sizeof(type));
    sendAll(socket, reinterpret_cast<const char*>(&elements), sizeof(elements));
    sendAll(socket, reinterpret_cast<const char*>(&value), sizeof(value));
}

void sendStringReply(SOCKET socket, const std::string& value)
{
    const char type = 1;
    const uint32_t elements = static_cast<uint32_t>(value.size() + 1);
    std::vector<char> payload(static_cast<std::size_t>(elements) * sizeof(double), 0);
    std::memcpy(payload.data(), value.data(), value.size());
    sendAll(socket, &type, sizeof(type));
    sendAll(socket, reinterpret_cast<const char*>(&elements), sizeof(elements));
    sendAll(socket, payload.data(), payload.size());
}

class ContractServer
{
public:
    ContractServer()
    {
        listener_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        require(listener_ != INVALID_SOCKET, "listener creation failed");

        sockaddr_in address{};
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        address.sin_port = 0;
        require(bind(listener_, reinterpret_cast<sockaddr*>(&address), sizeof(address)) == 0,
                "listener bind failed");
        require(listen(listener_, 1) == 0, "listener listen failed");

        int addressLength = sizeof(address);
        require(getsockname(listener_, reinterpret_cast<sockaddr*>(&address), &addressLength) == 0,
                "getsockname failed");
        port_ = ntohs(address.sin_port);
        thread_ = std::thread(&ContractServer::run, this);
    }

    ~ContractServer()
    {
        if (listener_ != INVALID_SOCKET)
            closesocket(listener_);
        if (thread_.joinable())
            thread_.join();
    }

    uint16_t port() const { return port_; }

    void finish()
    {
        if (thread_.joinable())
            thread_.join();
        if (failure_)
            std::rethrow_exception(failure_);
    }

private:
    void run()
    {
        try {
            const SOCKET client = accept(listener_, nullptr, nullptr);
            require(client != INVALID_SOCKET, "server accept failed");

            verifyFrame(receiveFrame(client), "get", "numeric", {});
            sendNumericReply(client, 42.5);

            const double setValue = -7.25;
            std::vector<char> numericPayload(sizeof(setValue));
            std::memcpy(numericPayload.data(), &setValue, sizeof(setValue));
            verifyFrame(receiveFrame(client), "set", "numeric", numericPayload);

            const std::vector<char> textPayload{'m', 'a', 'n', 'u', 'a', 'l', '\0'};
            verifyFrame(receiveFrame(client), "set", "mode", textPayload);

            verifyFrame(receiveFrame(client), "get", "mode", {});
            sendStringReply(client, "manual");

            closesocket(client);
        } catch (...) {
            failure_ = std::current_exception();
        }
    }

    SOCKET listener_ = INVALID_SOCKET;
    uint16_t port_ = 0;
    std::thread thread_;
    std::exception_ptr failure_;
};

class SilentServer
{
public:
    explicit SilentServer(bool partialReply = false, bool closeAfterRequest = false)
        : partialReply_(partialReply), closeAfterRequest_(closeAfterRequest)
    {
        listener_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        require(listener_ != INVALID_SOCKET, "silent listener creation failed");

        sockaddr_in address{};
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        address.sin_port = 0;
        require(bind(listener_, reinterpret_cast<sockaddr*>(&address), sizeof(address)) == 0,
                "silent listener bind failed");
        require(listen(listener_, 1) == 0, "silent listener listen failed");

        int addressLength = sizeof(address);
        require(getsockname(listener_, reinterpret_cast<sockaddr*>(&address), &addressLength) == 0,
                "silent getsockname failed");
        port_ = ntohs(address.sin_port);
        thread_ = std::thread(&SilentServer::run, this);
    }

    ~SilentServer()
    {
        stop();
        if (thread_.joinable())
            thread_.join();
        if (listener_ != INVALID_SOCKET)
            closesocket(listener_);
    }

    uint16_t port() const { return port_; }

    void finish()
    {
        stop();
        if (thread_.joinable())
            thread_.join();
        if (failure_)
            std::rethrow_exception(failure_);
    }

private:
    void stop()
    {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            stopRequested_ = true;
        }
        condition_.notify_all();
    }

    void run()
    {
        try {
            const SOCKET client = accept(listener_, nullptr, nullptr);
            require(client != INVALID_SOCKET, "silent server accept failed");
            verifyFrame(receiveFrame(client), "get", "silent", {});

            if (closeAfterRequest_) {
                closesocket(client);
                return;
            }

            if (partialReply_) {
                const char type = 0;
                const uint32_t elements = 2;
                const double firstElement = 1.0;
                sendAll(client, &type, sizeof(type));
                sendAll(client, reinterpret_cast<const char*>(&elements), sizeof(elements));
                sendAll(client, reinterpret_cast<const char*>(&firstElement), sizeof(firstElement));
            }

            std::unique_lock<std::mutex> lock(mutex_);
            condition_.wait_for(lock, std::chrono::milliseconds(5000),
                                [this] { return stopRequested_; });
            lock.unlock();
            closesocket(client);
        } catch (...) {
            failure_ = std::current_exception();
        }
    }

    SOCKET listener_ = INVALID_SOCKET;
    uint16_t port_ = 0;
    std::thread thread_;
    std::mutex mutex_;
    std::condition_variable condition_;
    bool stopRequested_ = false;
    bool partialReply_ = false;
    bool closeAfterRequest_ = false;
    std::exception_ptr failure_;
};

} // namespace

int main()
{
    try {
        SocketRuntime sockets;
        ContractServer server;
        std::string port = std::to_string(server.port());

        require(Connect(&port[0]) == 0, "Connect must succeed");
        require(IsConnected(&port[0]) == 1, "connected port must be reported connected");

        char numericId[] = "numeric";
        char getCommand[] = "get";
        require(ReceiveDoubleData(numericId, getCommand, &port[0]) == 1,
                "numeric reply element count mismatch");
        double numericResult = 0.0;
        require(ReadReceivedDoubleData(&numericResult, &port[0]) == nullptr,
                "numeric reply must not return text");
        require(numericResult == 42.5, "numeric reply value mismatch");

        double setValue = -7.25;
        require(SendDoubleData(numericId, &setValue, &port[0]) == 0,
                "numeric set return changed");

        char modeId[] = "mode";
        char modeValue[] = "manual";
        require(SendStringData(modeId, modeValue, &port[0]) == 0,
                "string set return changed");

        require(ReceiveDoubleData(modeId, getCommand, &port[0]) == 7,
                "string reply element count mismatch");
        double unused[7]{};
        char* text = ReadReceivedDoubleData(unused, &port[0]);
        require(text != nullptr, "string reply pointer missing");
        require(std::string(text) == "manual", "string reply value mismatch");
        require(std::string(ReadReceivedStringData(&port[0])) == "manual",
                "direct string reader value mismatch");

        require(Disconnect(&port[0]) == 0, "Disconnect return changed");
        require(IsConnected(&port[0]) == 0, "disconnected port must be reported disconnected");
        require(Disconnect(&port[0]) == 0, "Disconnect must be idempotent");
        require(SendDoubleData(numericId, &setValue, &port[0]) == 0,
                "disconnected numeric set return changed");
        require(Connect(nullptr) == 1, "null port must be rejected safely");
        server.finish();

        SilentServer silentServer;
        std::string silentPort = std::to_string(silentServer.port());
        require(Connect(&silentPort[0]) == 0, "silent-server Connect must succeed");
        char silentId[] = "silent";
        const auto silentStart = std::chrono::steady_clock::now();
        require(ReceiveDoubleData(silentId, getCommand, &silentPort[0]) == 0,
                "silent server must produce no elements");
        const auto silentElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - silentStart).count();
        require(silentElapsed >= 1500 && silentElapsed < 3500,
                "silent receive did not honor the two-second I/O deadline");
        const auto retryStart = std::chrono::steady_clock::now();
        require(ReceiveDoubleData(silentId, getCommand, &silentPort[0]) == 0,
                "timed-out connection must have been discarded");
        const auto retryElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - retryStart).count();
        require(retryElapsed < 200, "discarded connection retry must return immediately");
        require(IsConnected(&silentPort[0]) == 0,
                "timed-out connection must be reported disconnected");
        require(Disconnect(&silentPort[0]) == 0, "silent-server Disconnect failed");
        silentServer.finish();

        SilentServer partialServer(true);
        std::string partialPort = std::to_string(partialServer.port());
        require(Connect(&partialPort[0]) == 0, "partial-server Connect must succeed");
        const auto partialStart = std::chrono::steady_clock::now();
        require(ReceiveDoubleData(silentId, getCommand, &partialPort[0]) == 0,
                "partial reply must produce no elements");
        const auto partialElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - partialStart).count();
        require(partialElapsed >= 1500 && partialElapsed < 3500,
                "partial reply did not honor the two-second I/O deadline");
        require(Disconnect(&partialPort[0]) == 0, "partial-server Disconnect failed");
        partialServer.finish();

        SilentServer closingServer(false, true);
        std::string closingPort = std::to_string(closingServer.port());
        require(Connect(&closingPort[0]) == 0, "closing-server Connect must succeed");
        const auto closingStart = std::chrono::steady_clock::now();
        require(ReceiveDoubleData(silentId, getCommand, &closingPort[0]) == 0,
                "closed connection must produce no elements");
        const auto closingElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - closingStart).count();
        require(closingElapsed < 500, "peer close must return without waiting for timeout");
        require(Disconnect(&closingPort[0]) == 0, "closing-server Disconnect failed");
        closingServer.finish();

        std::cout << "MatlabRemoteConnector contract tests passed\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "MatlabRemoteConnector contract test failed: " << error.what() << '\n';
        return 1;
    }
}
