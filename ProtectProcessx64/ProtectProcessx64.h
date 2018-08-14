/***************************************************************************************
* AUTHOR : MZ
* DATE   : 2016-5-7
* MODULE : ProtectProcessx64.H
*
* IOCTRL Sample Driver
*
* Description:
*        Demonstrates communications between USER and KERNEL.
*
****************************************************************************************
* Copyright (C) 2010 MZ.
****************************************************************************************/

#ifndef CXX_PROTECTPROCESSX64_H
#define CXX_PROTECTPROCESSX64_H

#include <ntifs.h>

#define PROCESS_TERMINATE         0x0001  
#define PROCESS_VM_OPERATION      0x0008  
#define PROCESS_VM_READ           0x0010  
#define PROCESS_VM_WRITE          0x0020  

NTSTATUS DriverEntry(IN PDRIVER_OBJECT pDriverObj, IN PUNICODE_STRING pRegistryString);

VOID DriverUnload(IN PDRIVER_OBJECT pDriverObj);

typedef struct _LDR_DATA_TABLE_ENTRY64
{
    LIST_ENTRY64    InLoadOrderLinks;
    LIST_ENTRY64    InMemoryOrderLinks;
    LIST_ENTRY64    InInitializationOrderLinks;
    PVOID            DllBase;
    PVOID            EntryPoint;
    ULONG            SizeOfImage;
    UNICODE_STRING    FullDllName;
    UNICODE_STRING     BaseDllName;
    ULONG            Flags;
    USHORT            LoadCount;
    USHORT            TlsIndex;
    PVOID            SectionPointer;
    ULONG            CheckSum;
    PVOID            LoadedImports;
    PVOID            EntryPointActivationContext;
    PVOID            PatchInformation;
    LIST_ENTRY64    ForwarderLinks;
    LIST_ENTRY64    ServiceTagLinks;
    LIST_ENTRY64    StaticLinks;
    PVOID            ContextInformation;
    ULONG64            OriginalBase;
    LARGE_INTEGER    LoadTime;
} LDR_DATA_TABLE_ENTRY64, *PLDR_DATA_TABLE_ENTRY64;

extern 
    UCHAR *
    PsGetProcessImageFileName(
    __in PEPROCESS Process
    );
char*
    GetProcessImageNameByProcessID(ULONG ulProcessID);

NTSTATUS ProtectProcess(BOOLEAN Enable);

OB_PREOP_CALLBACK_STATUS 
    preCall(PVOID RegistrationContext, POB_PRE_OPERATION_INFORMATION pOperationInformation);

#endif    