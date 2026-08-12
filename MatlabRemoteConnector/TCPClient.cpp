#define _EXPORT_
#include "TCPClient.h"

#include "TCPDummyClass.h"

#include <cstdint>
#include <cstring>
#include <limits>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace {

constexpr std::size_t kRequestHeaderSize = 15;
constexpr std::size_t kReplyHeaderSize = 5;
constexpr std::size_t kDoubleSize = sizeof(double);
constexpr std::size_t kMaximumReplySize = 1024 * 1024;

struct PortState
{
    explicit PortState(const std::string& port)
        : connection(new TCPDummy(port))
    {
    }

    std::unique_ptr<TCPDummy> connection;
    std::vector<char> receivedData;
    uint32_t receivedElements = 0;
    bool receivedString = false;
};

std::map<std::string, std::unique_ptr<PortState>> states;
WindowsMutex statesMutex;

PortState* findState(const char* port)
{
    if (!port)
        return nullptr;
    const auto found = states.find(port);
    return found == states.end() ? nullptr : found->second.get();
}

void appendUint32(std::vector<uint8_t>& frame, uint32_t value)
{
    const uint8_t* bytes = reinterpret_cast<const uint8_t*>(&value);
    frame.insert(frame.end(), bytes, bytes + sizeof(value));
}

std::vector<uint8_t> createFrame(const char* command,
                                 const char* id,
                                 const void* payload,
                                 uint32_t payloadSize)
{
    if (!command || std::strlen(command) < 3 || !id || (!payload && payloadSize != 0))
        return {};

    const std::size_t idSize = std::strlen(id) + 1;
    const std::size_t totalSize = kRequestHeaderSize + idSize + payloadSize;
    if (idSize > std::numeric_limits<uint32_t>::max()
            || totalSize > std::numeric_limits<uint32_t>::max())
        return {};

    std::vector<uint8_t> frame;
    frame.reserve(totalSize);
    appendUint32(frame, static_cast<uint32_t>(totalSize));
    frame.insert(frame.end(), command, command + 3);
    appendUint32(frame, static_cast<uint32_t>(idSize));
    appendUint32(frame, payloadSize);
    frame.insert(frame.end(), id, id + idSize);
    const uint8_t* payloadBytes = static_cast<const uint8_t*>(payload);
    if (payloadSize)
        frame.insert(frame.end(), payloadBytes, payloadBytes + payloadSize);
    return frame;
}

bool sendFrame(PortState& state, const std::vector<uint8_t>& frame)
{
    TCPClient* client = state.connection ? state.connection->GetClient() : nullptr;
    return client && !frame.empty()
            && client->send(frame.data(), static_cast<uint32_t>(frame.size())) == frame.size();
}

bool receiveReply(PortState& state)
{
    TCPClient* client = state.connection ? state.connection->GetClient() : nullptr;
    if (!client)
        return false;

    char header[kReplyHeaderSize]{};
    if (client->receive(header, sizeof(header)) != sizeof(header))
        return false;

    uint32_t elements = 0;
    std::memcpy(&elements, header + 1, sizeof(elements));
    if (elements > (kMaximumReplySize - kReplyHeaderSize) / kDoubleSize)
        return false;

    state.receivedString = header[0] != 0;
    state.receivedElements = elements;
    state.receivedData.assign(static_cast<std::size_t>(elements) * kDoubleSize, 0);
    return state.receivedData.empty()
            || client->receive(state.receivedData.data(),
                               static_cast<uint32_t>(state.receivedData.size()))
                    == state.receivedData.size();
}

} // namespace

EXPORT char* ReadReceivedDoubleData(double* data, char* port)
{
    ScopedWindowsLock lock(statesMutex);
    PortState* state = findState(port);
    if (!state)
        return nullptr;

    if (state->receivedString)
        return state->receivedData.empty() ? nullptr : state->receivedData.data();

    if (!data && state->receivedElements != 0)
        return nullptr;
    if (!state->receivedData.empty())
        std::memcpy(data, state->receivedData.data(), state->receivedData.size());
    state->receivedData.clear();
    state->receivedElements = 0;
    return nullptr;
}

EXPORT char* ReadReceivedStringData(char* port)
{
    ScopedWindowsLock lock(statesMutex);
    PortState* state = findState(port);
    return !state || state->receivedData.empty() ? nullptr : state->receivedData.data();
}

EXPORT int ReceiveDoubleData(char* id, char* command, char* port)
{
    ScopedWindowsLock lock(statesMutex);
    PortState* state = findState(port);
    if (!state)
        return 0;

    state->receivedData.clear();
    state->receivedElements = 0;
    state->receivedString = false;
    const std::vector<uint8_t> frame = createFrame(command, id, nullptr, 0);
    if (!sendFrame(*state, frame) || !receiveReply(*state)) {
        states.erase(port);
        return 0;
    }
    return static_cast<int>(state->receivedElements);
}

EXPORT int SendDoubleData(char* id, double* data, char* port)
{
    ScopedWindowsLock lock(statesMutex);
    PortState* state = findState(port);
    if (!state || !data)
        return 0;
    if (!sendFrame(*state, createFrame("set", id, data, sizeof(*data))))
        states.erase(port);
    return 0;
}

EXPORT int SendStringData(char* id, char* data, char* port)
{
    ScopedWindowsLock lock(statesMutex);
    PortState* state = findState(port);
    if (!state || !data)
        return 0;
    const std::size_t payloadSize = std::strlen(data) + 1;
    if (payloadSize > std::numeric_limits<uint32_t>::max()
            || !sendFrame(*state, createFrame("set", id, data, static_cast<uint32_t>(payloadSize))))
        states.erase(port);
    return 0;
}

EXPORT int Connect(char* port)
{
    if (!port || !*port)
        return 1;

    ScopedWindowsLock lock(statesMutex);
    std::unique_ptr<PortState> state(new PortState(port));
    TCPClient* client = state->connection ? state->connection->GetClient() : nullptr;
    if (!client || client->Error || !client->running)
        return 1;
    states[port] = std::move(state);
    return 0;
}

EXPORT int Disconnect(char* port)
{
    if (!port)
        return 0;
    ScopedWindowsLock lock(statesMutex);
    states.erase(port);
    return 0;
}

EXPORT int IsConnected(char* port)
{
    ScopedWindowsLock lock(statesMutex);
    PortState* state = findState(port);
    TCPClient* client = state && state->connection ? state->connection->GetClient() : nullptr;
    return client && client->running && !client->Error ? 1 : 0;
}
