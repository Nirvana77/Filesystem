#ifndef Filesystem_Server_h__
#define Filesystem_Server_h__

struct T_Filesystem_Server;
typedef struct T_Filesystem_Server Filesystem_Server;

#define Filesystem_Server_VERSION 1u

#include "../String.h"
#include "../File.h"
#include "../Folder.h"
#include "../Json.h"
#include "../Portability.h"
#include "../StateMachine.h"
#include "../TCP/TCPServer.h"
#include "../Buffer.h"

typedef struct
{
	String m_IP;
	UInt16 m_Port;

} Filesystem_ServerSettings;

struct T_Filesystem_Server
{
	Bool m_Allocated;
	StateMachine_Task* m_Task;
	StateMachine* m_Worker;
	
	String m_Path;
	String m_FilesytemPath;

	json_t* m_Json;
	Filesystem_ServerSettings m_Settings;

	TCPServer m_TCPServer;
	LinkedList m_Sockets;
	Buffer m_Buffer;
};

int Filesystem_Server_InitializePtr(StateMachine* _Worker, const char* _Path, Filesystem_Server** _ServerPtr);
int Filesystem_Server_Initialize(Filesystem_Server* _Server, StateMachine* _Worker, const char* _Path);

void Filesystem_Server_Work(UInt64 _MSTime, void* _Context);

void Filesystem_Server_Dispose(Filesystem_Server* _Server);

#endif // Filesystem_Server_h__