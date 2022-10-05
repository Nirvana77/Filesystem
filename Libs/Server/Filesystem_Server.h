#ifndef Filesystem_Server_h__
#define Filesystem_Server_h__

struct T_Filesystem_Server;
typedef struct T_Filesystem_Server Filesystem_Server;

#define Filesystem_Server_VERSION 1u

#include "../String.h"
#include "../File.h"
#include "../Folder.h"
#include "../Json.h"
#include <curl/curl.h>

typedef struct
{
	String m_IP;
	u_int8_t m_Port;

} Filesystem_ServerSettings;

struct T_Filesystem_Server
{
	Bool m_Allocated;
	String m_Path;
	String m_FilesytemPath;

	json_t* m_Json;
	Filesystem_ServerSettings m_Settings;

	CURL* m_Curl;
	
};

int Filesystem_Server_InitializePtr(const char* _Path, Filesystem_Server** _ServerPtr);
int Filesystem_Server_Initialize(Filesystem_Server* _Server, const char* _Path);

void Filesystem_Server_Dispose(Filesystem_Server* _Server);

#endif // Filesystem_Server_h__
