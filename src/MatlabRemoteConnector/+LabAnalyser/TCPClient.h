int Connect(char* port);
int Disconnect(char* port);
int IsConnected(char* port);


char* ReadReceivedStringData(char* port);
int   ReceiveDoubleData(char* Id, char* Command, char* port);
char* ReadReceivedDoubleData(double * Data, char* port);

int SendDoubleData(char* Id, double * Data, char* port);
int SendStringData(char* Id, char * Data, char* port);
