// The Router has only one primary function, to send a string containing the 
// address and port of the Simulator once a connection has been established.
// (ex:   192.168.1.10:4000)
// What it needs to do:
//   - Open a listening socket on the required ports.  (4241 or 4242 are the router ports)
//   - Once a connection is established, send the response string.
//   - Close the connection.  This signals the client to open a connection to the
//     simulator using the address it just received.
//   - Return to the first step to handle any further connection attempts.
//
// Each Router port will be divided into a separate thread.  This is because the
// receiving function will halt the thread until a connection is made,
// and multiple threads are required for multiple ports to work simultaneously.
// Separate threads are also required so it doesn't interfere with the running loop
// of the main thread, which is responsible for managing the windows, updating
// information and taking user input.
//
// Each thread will contain its own copy of the response data so that the data
// sections do not overlap.

#ifndef ROUTER_H
#define ROUTER_H

#include "Components.h"
#include "SocketClass3.h"


class RouterThread
{
public:
	RouterThread();
	~RouterThread();

	char LogBuffer[2048];  //Holds a constructed log message generated by this thread

	unsigned long ThreadID;      //ID of the created thread

	char RecBuf[64];     //Holds the receiving data.  The router should not be receiving any data, so the buffer will be uselessly short.

	long RecBytes;       //The number of bytes received from the last message
	long TotalRecBytes;  //The total number of bytes received since the server was launched

	long SendBytes;       //The number of bytes sent on the last message
	long TotalSendBytes;  //The total number of bytes sent since the server was launched

	bool isExist;         //If true, the router thread exists.  If it doesn't, the main program should attempt to reactivate it.
	bool isActive;        //If true, the thread is active and running.
	int Status;           //This maintains the activity Status, determining whether it needs to Init, Restart, etc.
	char SimTarget[128];  //The constructed response string containing the simulator address and port

	int HomePort;
	int TargetPort;
	char BindAddress[128];     //Address to listen on
	char HomePortStr[12];     //The port to use for listening
	char TargetPortStr[12];   //The target port of the simulator

	SocketClass sc;       //Controls the socket connection for this thread.

	int InternalIndex;    //A user defined value to identify this router instance index in order to assist in diagnostics
	int GlobalThreadID;   //A unique application-defined thread ID for debugging purposes.

	void RunMainLoop(void);
	void SetHomePort(int port);
	void SetTargetPort(int port);
	void SetBindAddress(const char *address);
	int InitThread(int instanceindex, int globalThreadID);
	void OnConnect(const char *destAddress);  //Called once a connection has been made
	void Shutdown(void);   //Force a complete shutdown (thread too)
	int ResolvePort(const char *destAddress, int port);  //Returns 1 if successful, 0 if nothing found
	void ResetValues(bool fullRestart);
	void Restart(void);

	char * LogMessageL(const char *format, ...);  //Logs a message to the local buffer instead of the main thread
};

extern RouterThread Router;

#endif //ROUTER_H