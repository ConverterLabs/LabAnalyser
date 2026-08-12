#ifndef LABANALYSER_MATLAB_TCPDUMMYCLASS_H
#define LABANALYSER_MATLAB_TCPDUMMYCLASS_H

#include "TCPClientBare.cpp"

#include <memory>
#include <string>

// Retains the historical internal type name while giving socket ownership a
// single RAII boundary. No background thread is required for the synchronous
// MATLAB C API.
class TCPDummy
{
public:
    explicit TCPDummy(const std::string& port)
        : client_(new TCPClient("127.0.0.1", port))
    {
    }

    TCPClient* GetClient() const { return client_.get(); }

private:
    std::unique_ptr<TCPClient> client_;
};

#endif
