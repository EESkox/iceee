// Thread for the HTTP listening server
//
//

#ifndef HTTPDISTRIBUTE_H
#define HTTPDISTRIBUTE_H

#include "SocketClass3.h"

#include <list>
#include <vector>
#include <string>
#include <map>
#include "Components.h"

//DWORD WINAPI HTTPDistributeThreadProc(LPVOID lpParam);

const int MAXHTTPSLOT = 8;

typedef std::vector<std::vector<std::string> > MULTISTRING;

extern Platform_CriticalSection http_cs;

class HTTPDistribute
{
public:
	HTTPDistribute();
	~HTTPDistribute();

	char LogBuffer[2048];  //Holds a constructed log message generated by this thread
	sockaddr remotename;

	unsigned long ThreadID;      //ID of the created thread

	char RecBuf[2048];   //Holds the receiving data, most likely the HTTP GET request for a file.
	char SendBuf[4096];  //Holds the data that will be sent back to the client
	std::string FileNameRequest;      //The full file name request as extracted by the GET command, including domain, path, and file.
	std::string FileNameAsset;        //The path and file from the GET request (without the domain)
	std::string FileNameLocal;        //The file name of the resolved file to read from the disk (may contain paths adjusted for platform)
	std::string FileNameRequestBare;  //The file name of the request, no path.
	
	//char FullFileName[256];  //Holds the full file name extracted by the GET command, including path
	//char ReadFileName[256];  //Holds the file name of the file to read from disk

	long RecBytes;       //The number of bytes received from the last message
	long TotalRecBytes;  //The total number of bytes received since the server was launched

	long SendBytes;       //The number of bytes sent on the last message
	long TotalSendBytes;  //The total number of bytes sent since the server was launched

	bool isExist;         //If true, the thread exists.  If it doesn't, the main program should attempt to reactivate it.
	bool isActive;        //If true, the is active and running.
	bool inUse;           //If true, thread is processing a download request
	int Status;           //This maintains the activity Status, determining whether it needs to Init, Restart, etc.

	static const int ERROR_STRICT404 = -1;
	static const int ERROR_CUSTOM404 = -2;
	bool SendFile;        //If true, a file is being sent
	bool Finished;        //Send operation has finished
	bool Disconnect;
	long DataPos;         //The data position to read from
	long FileSize;        //The total size of the file
	long DataRemain;      //Data remaining to be sent.
	FILE *DataFile;       //File to read data from
	long ChunkNum;        //Current chunk that is being sent

	int RecCurrentPos;    //Current position to save incoming data from recv.

	SocketClass sc;       //Controls the socket connection for this thread.

	int InternalIndex;    //A user defined value to identify this instance index in order to assist in diagnostics
	int MessageCountRec;

	unsigned long LastAction;
	unsigned long ExpireDelay;

	void RunMainLoop(void);
	int InitThread(int instanceindex);
	void OnConnect(void);  //Called once a connection has been made
	void ResetValues(bool fullRestart);
	void CheckAutoResponse(void);

	void SendNextFileChunk(void);
	void LogInvalidRequest(void);
	void ProcessHTTPRequest(void);
	void HandleHTTP_GET(char *dataStart, MULTISTRING &header);
	void HandleHTTP_POST(char *dataStart, MULTISTRING &header);
	bool IsEmptyFileRequest(void);
	void PrepareFileNames(void);
	int OpenLocalFileName(void);
	int FillErrorMessage(int errCode);
	int FillClientHas(void);
	int FillClientNeeds(void);
	int FillAPI(void);
	bool NeedFile(bool verifyExist, std::string &checksum);

	void CloseFileHandle(void);
	void DisconnectClient(const char *debugReason);
	bool QualifyDelete(void);

	char * LogMessageL(int logLevel, const char *format, ...);  //Logs a message to the local buffer instead of the main thread
};

class FileChecksum
{
public:
	typedef std::map<std::string, std::string> CHECKSUM_MAP;
	CHECKSUM_MAP mChecksumData;
	void LoadFromFile(const char *filename);
	bool MatchChecksum(const std::string &filename, const std::string &checksum);
};

class HTTPDistributeManager
{
public:
	HTTPDistributeManager();
	~HTTPDistributeManager();

	typedef std::list<HTTPDistribute>::iterator ITERATOR;

	HTTPDistribute* GetNewDistributeSlot(void);
	void CheckInactiveDistribute(void);
	void ShutDown(void);
	int GetSlotCount(void);

	std::list<HTTPDistribute> mDistributeList;
	bool mDebugThreadCollision;
	unsigned long mNextInactiveScan;
	int mDroppedConnections;
	unsigned long mTotalSendBytes;
	unsigned long mTotalRecBytes;
};


const char *GetValueOfKey(MULTISTRING &extract, const char *key);

extern FileChecksum g_FileChecksum;
extern HTTPDistributeManager g_HTTPDistributeManager;

#endif //HTTPDISTRIBUTE_H
