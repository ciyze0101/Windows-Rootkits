#pragma once
#include "SSSDTManager.h"


typedef struct _SYSTEM_SERVICE_TABLE64{
	PVOID  		ServiceTableBase; 
	PVOID  		ServiceCounterTableBase; 
	ULONG64  	NumberOfServices; 
	PVOID  		ParamTableBase; 
} SYSTEM_SERVICE_TABLE64, *PSYSTEM_SERVICE_TABLE64;

typedef struct _SYSTEM_SERVICE_TABLE32 {
	PVOID   ServiceTableBase;
	PVOID   ServiceCounterTableBase;
	ULONG32 NumberOfServices;
	PVOID   ParamTableBase;
} SYSTEM_SERVICE_TABLE32, *PSYSTEM_SERVICE_TABLE32;

VOID WPOFF();
VOID WPON();



//获取版本信息
typedef enum WIN_VERSION {
    WINDOWS_XP,
    WINDOWS_7_7600,
    WINDOWS_7_7601,
    WINDOWS_8_9200,
    WINDOWS_8_9600,
    WINDOWS_10_10240,
    WINDOWS_10_10586,
    WINDOWS_10_14393,
    WINDOWS_10_15063,
    WINDOWS_10_16299,
    WINDOWS_10_17134,
    WINDOWS_UNKNOW
} WIN_VERSION;

//Windows版本信息
WIN_VERSION GetWindowsVersion();
PVOID 
	GetFunctionAddressByName(WCHAR *wzFunction);
typedef 
	NTSTATUS 
	(*pfnRtlGetVersion)(OUT PRTL_OSVERSIONINFOW lpVersionInformation);

BOOLEAN SafeCopyMemory(PVOID DestiAddress, PVOID SourAddress, SIZE_T SizeOfCopy);


typedef struct _KLDR_DATA_TABLE_ENTRY64 {
	LIST_ENTRY64 InLoadOrderLinks;
	ULONG64 __Undefined1;
	ULONG64 __Undefined2;
	ULONG64 __Undefined3;
	ULONG64 NonPagedDebugInfo;
	ULONG64 DllBase;
	ULONG64 EntryPoint;
	ULONG SizeOfImage;
	UNICODE_STRING64 FullDllName;
	UNICODE_STRING64 BaseDllName;
	ULONG   Flags;
	USHORT  LoadCount;
	USHORT  __Undefined5;
	ULONG64 __Undefined6;
	ULONG   CheckSum;
	ULONG   __padding1;
	ULONG   TimeDateStamp;
	ULONG   __padding2;
} KLDR_DATA_TABLE_ENTRY64, *PKLDR_DATA_TABLE_ENTRY64;

typedef struct _KLDR_DATA_TABLE_ENTRY32 {
	LIST_ENTRY32 InLoadOrderLinks;
	ULONG __Undefined1;
	ULONG __Undefined2;
	ULONG __Undefined3;
	ULONG NonPagedDebugInfo;
	ULONG DllBase;
	ULONG EntryPoint;
	ULONG SizeOfImage;
	UNICODE_STRING32 FullDllName;
	UNICODE_STRING32 BaseDllName;
	ULONG Flags;
	USHORT LoadCount;
	USHORT __Undefined5;
	ULONG  __Undefined6;
	ULONG  CheckSum;
	ULONG  TimeDateStamp;
} KLDR_DATA_TABLE_ENTRY32, *PKLDR_DATA_TABLE_ENTRY32;





//LDR
typedef struct _LDR_DATA_TABLE_ENTRY32 {
	LIST_ENTRY32 InLoadOrderLinks;
	LIST_ENTRY32 InMemoryOrderLinks;
	LIST_ENTRY32 InInitializationOrderLinks;
	ULONG DllBase;
	ULONG EntryPoint;
	ULONG SizeOfImage;
	UNICODE_STRING32 FullDllName;
	UNICODE_STRING32 BaseDllName;
	ULONG Flags;
	USHORT LoadCount;
	USHORT TlsIndex;
	union {
		LIST_ENTRY32 HashLinks;
		struct {
			ULONG SectionPointer;
			ULONG  CheckSum;
		};
	};
	union {
		struct {
			ULONG  TimeDateStamp;
		};
		struct {
			ULONG LoadedImports;
		};
	};



} LDR_DATA_TABLE_ENTRY32, *PLDR_DATA_TABLE_ENTRY32;

typedef struct _LDR_DATA_TABLE_ENTRY64 {
	LIST_ENTRY64 InLoadOrderLinks;
	LIST_ENTRY64 InMemoryOrderLinks;
	LIST_ENTRY64 InInitializationOrderLinks;
	ULONG64 DllBase;
	ULONG64 EntryPoint;
	ULONG SizeOfImage;
	UNICODE_STRING64 FullDllName;
	UNICODE_STRING64 BaseDllName;
	ULONG   Flags;
	USHORT  LoadCount;
	USHORT TlsIndex;
	union {
		LIST_ENTRY64 HashLinks;
		struct {
			ULONG64 SectionPointer;
			ULONG   CheckSum;
		};
	};
	union {
		struct {
			ULONG   TimeDateStamp;
		};
		struct {
			ULONG64 LoadedImports;
		};
	};



} LDR_DATA_TABLE_ENTRY64, *PLDR_DATA_TABLE_ENTRY64;

#ifdef _WIN64
#define PKLDR_DATA_TABLE_ENTRY  PKLDR_DATA_TABLE_ENTRY64
#define PLDR_DATA_TABLE_ENTRY   PLDR_DATA_TABLE_ENTRY64
#else
#define PKLDR_DATA_TABLE_ENTRY  PKLDR_DATA_TABLE_ENTRY32
#define PLDR_DATA_TABLE_ENTRY   PLDR_DATA_TABLE_ENTRY32
#endif
