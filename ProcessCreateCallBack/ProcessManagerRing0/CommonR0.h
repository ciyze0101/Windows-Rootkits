#pragma once
#include "ProcessManagerRing0.h"




typedef struct _HANDLE_TABLE64
{
	PVOID64 TableCode;
	PVOID64 QuotaProcess;
	PVOID64 UniqueProcessID;
	PVOID64 HandleLock;
	LIST_ENTRY HandleTableList;
	PVOID64    HandleContentionEvent;
	PVOID64    DebugInfo;
	ULONG      ExtraInfoPages;
	ULONG      Flags;
	ULONG      FirstFreeHandle;
	PVOID64    LastFreeHandleEntry;
	ULONG      HandleCount;
	ULONG      NextHandleNeedingPool;
	ULONG      HandleCountHighWatermark;
}HANDLE_TABLE64,*PHANDLE_TABLE64;



typedef struct _HANDLE_TABLE32
{
	PVOID TableCode;
	PVOID QuotaProcess;
	PVOID UniqueProcessID;
	ULONG HandleLock[4];
	LIST_ENTRY HandleTableList;
	PVOID    HandleContentionEvent;
	PVOID    DebugInfo;
	ULONG    ExtraInfoPages;
	ULONG    FirstFree;
	ULONG    LastFree;
	ULONG    NextHandleNeedingPool;
	ULONG    HandleCount;
	ULONG    Flags;
}HANDLE_TABLE32,*PHANDLE_TABLE32;

#ifdef _WIN64
#define PHANDLE_TABLE PHANDLE_TABLE64
#else
#define PHANDLE_TABLE PHANDLE_TABLE32
#endif





typedef struct _HANDLE_TABLE_ENTRY64 
{
	union {
		PVOID64 Object;
		ULONG ObAttributes;
		PVOID64 InfoTable;
		ULONG_PTR Value;
	};
	union {
		union {
			ULONG GrantedAccess;
			struct {
				USHORT GrantedAccessIndex;
				USHORT CreatorBackTraceIndex;
			};
		};
		ULONG NextFreeTableEntry;
	};

} HANDLE_TABLE_ENTRY64, *PHANDLE_TABLE_ENTRY64;


typedef struct _HANDLE_TABLE_ENTRY32 
{
	union {
		PVOID Object;
		ULONG ObAttributes;
		PVOID InfoTable;
		ULONG_PTR Value;
	};
	union {
		union {
			ULONG GrantedAccess;
			struct {
				USHORT GrantedAccessIndex;
				USHORT CreatorBackTraceIndex;
			};
		};
		ULONG NextFreeTableEntry;
	};

} HANDLE_TABLE_ENTRY32, *PHANDLE_TABLE_ENTRY32;


#ifdef _WIN64
#define PHANDLE_TABLE_ENTRY PHANDLE_TABLE_ENTRY64
#else
#define PHANDLE_TABLE_ENTRY PHANDLE_TABLE_ENTRY32
#endif

#ifdef _WIN64
#define HANDLE_TABLE_ENTRY HANDLE_TABLE_ENTRY64
#else
#define HANDLE_TABLE_ENTRY HANDLE_TABLE_ENTRY32
#endif




typedef struct _PROCESS_INFORMATION_ENTRY_
{
	char ProcessName[50];
	ULONG Pid;
	ULONG ParentId;
	WCHAR ProcessPath[260];
	ULONG_PTR Eprocess;
	BOOLEAN IsAccess;
	WCHAR Company[20];
}PROCESS_INFORMATION_ENTRY, *PPROCESS_INFORMATION_ENTRY;
typedef struct _PROCESS_INFORMATION_OWN
{
	ULONG_PTR    NumberOfEntry;
	PROCESS_INFORMATION_ENTRY Entry[1];
}PROCESS_INFORMATION_OWN, *PPROCESS_INFORMATION_OWN;



typedef enum WIN_VERSION {
	WINDOWS_UNKNOW,
	WINDOWS_XP,
	WINDOWS_7,
	WINDOWS_8,
	WINDOWS_8_1
} WIN_VERSION;

WIN_VERSION GetWindowsVersion();
PVOID 
	GetFunctionAddressByName(WCHAR *wzFunction);
typedef 
	NTSTATUS 
	(*pfnRtlGetVersion)(OUT PRTL_OSVERSIONINFOW lpVersionInformation);

ULONG_PTR KeGetObjectType(PVOID Object);
typedef ULONG_PTR 
	(*pfnObGetObjectType)(PVOID pObject);
BOOLEAN IsProcessDie(PEPROCESS EProcess);