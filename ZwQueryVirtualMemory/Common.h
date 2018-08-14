#pragma once
#include "ZwQueryVirtualMemory.h"


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
ULONG_PTR KeGetObjectType(PVOID Object);
BOOLEAN IsRealProcess(PEPROCESS EProcess) ;
CHAR ChangePreMode(PETHREAD EThread);
VOID RecoverPreMode(PETHREAD EThread, CHAR PreMode);
VOID InitGlobalVariable();//初始化一些偏移
BOOLEAN NtPathToDosPathW(WCHAR* wzFullNtPath,WCHAR* wzFullDosPath);
extern
    NTSTATUS
    NTAPI
    ZwQueryDirectoryObject (
    __in HANDLE DirectoryHandle,
    __out_bcount_opt(Length) PVOID Buffer,
    __in ULONG Length,
    __in BOOLEAN ReturnSingleEntry,
    __in BOOLEAN RestartScan,
    __inout PULONG Context,
    __out_opt PULONG ReturnLength
    );

typedef struct _OBJECT_DIRECTORY_INFORMATION
{
    UNICODE_STRING Name;
    UNICODE_STRING TypeName;
} OBJECT_DIRECTORY_INFORMATION, *POBJECT_DIRECTORY_INFORMATION;


ULONG
    NtQueryDosDevice(WCHAR* wzDosDevice,WCHAR* wzNtDevice,
    ULONG ucchMax);


