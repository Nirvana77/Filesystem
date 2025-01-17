#ifndef Payload_h__
#define Payload_h__

struct T_Payload;
typedef struct T_Payload Payload;


#ifndef Payload_BufferSize
	#define Payload_BufferSize 256
#endif

#include "../Buffer.h"
#include "../uuid.h"
#include "../EventHandler.h"

#ifdef __linux__


	#include <unistd.h>
	#include <sys/fcntl.h>
	#include <arpa/inet.h>
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <sys/ioctl.h>
	#include <netinet/in.h>
	#include <net/if.h>

#endif

#define Payload_AddreessSize (1 + 6)

#define Payload_TypePosistion (8)
#define Payload_SourcePosistion (Payload_TypePosistion + 1)
#define Payload_DestinationPosistion (Payload_SourcePosistion + Payload_AddreessSize)
#define Payload_UUIDPosistion (Payload_DestinationPosistion + Payload_AddreessSize)
#define Payload_MethodPosistion (Payload_SourcePosistion + Payload_AddreessSize*2 + UUID_DATA_SIZE)

#define Payload_TimeoutAlgorithm(t) (t*2)

typedef struct
{
	void* m_Context;
	int (*m_Receive)(void* _Context, Payload* _Message, Payload* _Replay);
	int (*m_Send)(void* _Context, Payload** _PaylodePtr);
} Payload_FuncOut;

typedef struct
{
	void* m_Context;
	int (*m_OnRead)(void* _Context, Buffer* _Buffer);
	int (*m_OnWrite)(void* _Context, Buffer* _Buffer);
} Payload_FuncIn;

static inline void Payload_FuncOut_Set(Payload_FuncOut* _FuncOut, int (*_Receive)(void* _Context, Payload* _Message, Payload* _Replay), int (*_Send)(void* _Context, Payload** _PaylodePtr), void* _Context)
{
	_FuncOut->m_Context = _Context;
	_FuncOut->m_Receive = _Receive;
	_FuncOut->m_Send = _Send;
}

static inline void Payload_FuncOut_Clear(Payload_FuncOut* _FuncOut)
{
	memset(_FuncOut, 0, sizeof(Payload_FuncOut));
}

typedef enum
{
	Payload_State_Init = 0,
	Payload_State_Sented = 1,
	Payload_State_Sending = 2,
	Payload_State_Resived = 3,
	Payload_State_Removed = 4,
	Payload_State_Replay = 5,
	Payload_State_Destroyed = 6,
	Payload_State_Failed = 7,
	Payload_State_Timeout = 8
} Payload_State;

typedef enum
{
	Payload_Type_Min = 0,

	Payload_Type_Broadcast = 0,
	Payload_Type_BroadcastRespons = 1,
	Payload_Type_ACK = 2,
	Payload_Type_UnSafe = 3,
	Payload_Type_Safe = 4,
	Payload_Type_Respons = 5,

	Payload_Type_Max = 5
} Payload_Type;

typedef enum
{
	Payload_Message_Type_Min = 0,

	Payload_Message_Type_None = 0,
	Payload_Message_Type_String = 1,
	
	Payload_Message_Type_Max = 1
} Payload_Message_Type;

typedef struct
{
	Payload_Message_Type m_Type;
	UInt16 m_Size;
	union
	{
		char m_Str[64]; //Make this a deff
	} m_Method;
	
	
} Payload_Message;

typedef enum
{
	Payload_Address_Type_Min = 0,

	Payload_Address_Type_NONE = 0,
	Payload_Address_Type_IP = 1,
	Payload_Address_Type_MAC = 2,

	Payload_Address_Type_Max = 2
} Payload_Address_Type;

typedef struct
{
	Payload_Address_Type m_Type;
	union
	{
		UInt8 IP[4];
		UInt8 MAC[6];
	} m_Address;
	
} Payload_Address;

struct T_Payload
{
	Bool m_Allocated;
	UInt8 m_UUID[UUID_DATA_SIZE];

	Payload_State m_State;

	UInt16 m_Size;
	UInt64 m_Time;
	UInt64 m_Timeout;

	Payload_Type m_Type;

	Payload_Message m_Message;

	Payload_Address m_Src;
	Payload_Address m_Des;

	EventHandler m_EventHandler;

	Buffer m_Data;
};

int Payload_InitializePtr(UInt8 _UUID[UUID_DATA_SIZE], Payload** _PayloadPtr);
int Payload_Initialize(Payload* _Payload, UInt8 _UUID[UUID_DATA_SIZE]); 

int Payload_WriteAddress(Payload_Address* _Address, Buffer* _Buffer);
int Payload_ReadAddress(Payload_Address* _Address, Buffer* _Buffer);

void Payload_FilAddress(Payload_Address* _Des, Payload_Address* _Src);
void Payload_FilMessage(Payload_Message* _Des, Payload_Message* _Src);
Bool Payload_ComperAddresses(Payload_Address* _A, Payload_Address* _B)
{
	if(_A->m_Type != _B->m_Type)
		return False;

	return memcmp(&_A->m_Address, &_B->m_Address, sizeof(_A->m_Address)) == 0 ? True : False;
}

int Payload_WriteMessage(Payload_Message* _Message, Buffer* _Buffer);
int Payload_ReadMessage(Payload_Message* _Message, Buffer* _Buffer);

void Payload_Copy(Payload* _Des, Payload* _Src);

static inline void Payload_DeepCopy(Payload* _Des, Payload* _Src)
{
	Payload_Copy(_Des, _Src);
	_Des->m_State = _Src->m_State;
	_Des->m_Time = _Src->m_Time;

	memcpy(_Des->m_UUID, _Src->m_UUID, UUID_DATA_SIZE);

}

//! This is only for DEBUGING!
static inline void Payload_Print(Payload* _Payload, const char* _Str)
{
	String str;
	String_Initialize(&str, 128);

	String_Sprintf(&str, "Payload(%s): %lu\n", _Str, _Payload->m_Time);
	String_Sprintf(&str, "State: %i\n", _Payload->m_State);
	String_Sprintf(&str, "Type: %i\n", _Payload->m_Type);
	
	char uuid_str[UUID_FULLSTRING_SIZE];
	uuid_ToString(_Payload->m_UUID, uuid_str);
	String_Sprintf(&str, "UUID: %s\n", uuid_str);

	if(_Payload->m_Src.m_Type == Payload_Address_Type_IP)
		String_Sprintf(&str, "SRC: %i.%i.%i.%i\n", _Payload->m_Src.m_Address.IP[0], _Payload->m_Src.m_Address.IP[1], _Payload->m_Src.m_Address.IP[2], _Payload->m_Src.m_Address.IP[3]);
	else
		String_Sprintf(&str, "SRC: %x-%x-%x-%x-%x-%x\n", _Payload->m_Src.m_Address.MAC[0], _Payload->m_Src.m_Address.MAC[1], _Payload->m_Src.m_Address.MAC[2], _Payload->m_Src.m_Address.MAC[3], _Payload->m_Src.m_Address.MAC[4], _Payload->m_Src.m_Address.MAC[5]);
		

	if(_Payload->m_Des.m_Type == Payload_Address_Type_IP)
		String_Sprintf(&str, "DES: %i.%i.%i.%i\n", _Payload->m_Des.m_Address.IP[0], _Payload->m_Des.m_Address.IP[1], _Payload->m_Des.m_Address.IP[2], _Payload->m_Des.m_Address.IP[3]);
	else
		String_Sprintf(&str, "DES: %x-%x-%x-%x-%x-%x\n", _Payload->m_Des.m_Address.MAC[0], _Payload->m_Des.m_Address.MAC[1], _Payload->m_Des.m_Address.MAC[2], _Payload->m_Des.m_Address.MAC[3], _Payload->m_Des.m_Address.MAC[4], _Payload->m_Des.m_Address.MAC[5]);

	if(_Payload->m_Message.m_Type != Payload_Message_Type_None)
	{
		if(_Payload->m_Message.m_Type == Payload_Message_Type_String)
			String_Sprintf(&str, "Method(%u): %s\n", _Payload->m_Message.m_Size, _Payload->m_Message.m_Method.m_Str);
	}

	String_Sprintf(&str, "Data: %ib\n", _Payload->m_Data.m_BytesLeft);
	
	for(int i = 0; i < _Payload->m_Data.m_BytesLeft; i++)
		String_Sprintf(&str, "%x%s", _Payload->m_Data.m_ReadPtr[i], i + 1 < _Payload->m_Data.m_BytesLeft ? " " : "");
	
	String_Sprintf(&str, "\n");
	String_Sprintf(&str, "\n");
	String_Sprintf(&str, "\n");
	
	FILE* f = NULL;
	File_Open("payload_dump.txt", File_Mode_ApendCreate, &f);
	
	File_WriteAll(f, (const unsigned char*)str.m_Ptr, str.m_Length);

	File_Close(f);
	String_Dispose(&str);
}

static inline void Payload_SetMessageType(Payload* _Payload, Payload_Message_Type _Type, void* _Value, int _Size)
{

	switch (_Type)
	{
		case Payload_Message_Type_String:
		{
			strncpy(_Payload->m_Message.m_Method.m_Str, (const char*)_Value, _Size + 1);
			
			_Payload->m_Message.m_Type = _Type;
			_Payload->m_Message.m_Size = _Size;
		} break;
		case Payload_Message_Type_None:
		{
			_Payload->m_Message.m_Type = _Type;
			_Payload->m_Message.m_Size = 0;
			
		} break;
	}
}

static inline void GetIP(UInt8 _Address[4])
{
	int n;
	struct ifreq ifr;
	char* array = "eth0";

	n = socket(AF_INET, SOCK_DGRAM, 0);
	//Type of address to retrieve - IPv4 IP address
	ifr.ifr_addr.sa_family = AF_INET;
	//Copy the interface name in the ifreq structure
	strncpy(ifr.ifr_name , array , IFNAMSIZ - 1);
	ioctl(n, SIOCGIFADDR, &ifr);
	close(n);
	//display result
	char str[16] = "";
	sprintf(str, "%s", inet_ntoa(( (struct sockaddr_in *)&ifr.ifr_addr )->sin_addr) );

	int index = 0;
	UInt8 value = 0;
	for (int i = 0; i < strlen(str); i++)
	{
		if(str[i] == '.')
		{
			_Address[index++] = value;
			value = 0;
		}
		else
		{
			if(value > 10 || i == 1 || i == 5 || i == 9 || i == 13)
				value *= 10;

			value += (int)str[i] - 48;
		}
	}
	
	_Address[index++] = value;
}

static inline int GetMAC(UInt8 _Address[6])
{
	struct ifreq s;
	int fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);

	strcpy(s.ifr_name, "eth0");
	if (0 == ioctl(fd, SIOCGIFHWADDR, &s)) {
		int j = 0;
		for (int i = 0; i < 6; ++i)
			_Address[j++] =s.ifr_addr.sa_data[i] - 48;
		return 0;
	}

	return 1;
}

static inline Bool CommperIP(UInt8 _A[4], UInt8 _B[4])
{
	for (int i = 0; i < 4; i++)
	{
		if(_A[i] != _B[i])
			return False;
	}
	
	return True;
}

static inline Bool CommperMAC(UInt8 _A[6], UInt8 _B[6])
{
	for (int i = 0; i < 6; i++)
	{
		if(_A[i] != _B[i])
			return False;
	}
	
	return True;
}

static inline void Payload_GetIP(Payload_Address* _Addrass, char _Str[4*3 + 3 + 1])
{
	if (_Addrass->m_Type != Payload_Address_Type_IP)
		return;
	int j = 0;
	for (int i = 0; i < 4; i++)
	{
		int value = _Addrass->m_Address.IP[i];
		for(int k = j + 2; k >= j; k--)
		{
			_Str[k] = (char)(value % 10) + 48;
			value /= 10;
		}
		if(i + 1 < 4)
			_Str[j+3] = '.';
		j += 3 + 1; 
	}
	_Str[4*3 + 3] = 0;
}

static inline void Payload_GetMac(Payload_Address* _Addrass, char _Str[6*2 + 5 + 1])
{
	if (_Addrass->m_Type != Payload_Address_Type_MAC)
		return;
	
	sprintf(_Str, "%2x-%2x-%2x-%2x-%2x-%2x", _Addrass->m_Address.MAC[0], _Addrass->m_Address.MAC[1], _Addrass->m_Address.MAC[2], _Addrass->m_Address.MAC[3], _Addrass->m_Address.MAC[4], _Addrass->m_Address.MAC[5]);
}

static inline int Payload_StrToIP(Payload_Address* _Addrass, const char* _Str)
{
	int length = strlen(_Str);
	if(length > 4*3 + 3 + 1)
		return -1;
	
	_Addrass->m_Type = Payload_Address_Type_IP;
	int index = 0;

	for (int i = 0; i < length; i++)
	{
		int value = 0;
		while (_Str[i] != '.' && i < length)
		{
			value += (int)(_Str[i] - 48);
			value *= 10;
			i++;
		}
		value /= 10;
		_Addrass->m_Address.IP[index++] = value;
	}
	
	

	return 0;
}

void Payload_Dispose(Payload* _Payload);
#endif // Payload_h__