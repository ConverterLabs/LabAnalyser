#ifndef LABANALYSER_MATLAB_TCPCLIENT_H
#define LABANALYSER_MATLAB_TCPCLIENT_H

#ifndef _EXPORT_
#define EXPORT extern "C" __declspec(dllimport)
#else
#define EXPORT extern "C" __declspec(dllexport)
#endif

EXPORT int Connect(char* port);
EXPORT int Disconnect(char* port);
EXPORT int IsConnected(char* port);

EXPORT char* ReadReceivedStringData(char* port);
EXPORT int ReceiveDoubleData(char* Id, char* Command, char* port);
EXPORT char* ReadReceivedDoubleData(double* Data, char* port);

EXPORT int SendDoubleData(char* Id, double* Data, char* port);
EXPORT int SendStringData(char* Id, char* Data, char* port);

#endif
