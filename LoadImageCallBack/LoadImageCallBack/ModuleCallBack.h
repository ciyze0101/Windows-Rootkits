/***************************************************************************************
* AUTHOR : MZ
* DATE   : 2015-8-13
* MODULE : ModuleCallBack.H
*
* IOCTRL Sample Driver
*
* Description:
*		Demonstrates communications between USER and KERNEL.
*
****************************************************************************************
* Copyright (C) 2010 MZ.
****************************************************************************************/

#ifndef CXX_MODULECALLBACK_H
#define CXX_MODULECALLBACK_H


#include <ntifs.h>
#include <devioctl.h>
#include <ntimage.h>

VOID UnicodeToChar(PUNICODE_STRING uniSource ,CHAR *szDest);
VOID LoadImageNotifyRoutine(PUNICODE_STRING FullImageName,HANDLE ProcessId,PIMAGE_INFO ImageInfor);
NTSTATUS
	DriverEntry(IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING RegisterPath);


BOOLEAN  EnumImportTable(PUNICODE_STRING uniDestFileFullPath);
ULONG_PTR RVAToOffset(PIMAGE_NT_HEADERS NTHeader, ULONG_PTR ulRVA);
VOID UnloadDriver(PDRIVER_OBJECT  DriverObject);
ULONG_PTR RVAToOffset(PIMAGE_NT_HEADERS NTHeader, ULONG_PTR ulRVA);
#define MAX_TRANSFER_SIZE 0x1000

extern 
	UCHAR *
	PsGetProcessImageFileName(
	__in PEPROCESS Process
	);

NTSTATUS MAPFILE_AND_GETBASE(PUNICODE_STRING pDriverName);
BOOLEAN
	EnumIATByFileMapping(PUNICODE_STRING uniDestFileFullPath);
NTSTATUS  ForceDeleteFileHandle(UNICODE_STRING  uniFileFullPath);
void MyProcessThread(PVOID pContext);

BOOLEAN GetPathByFileObject(PFILE_OBJECT FileObject, WCHAR* wzPath);

typedef enum _SYSTEM_HANDLE_TYPE
{
	OB_TYPE_UNKNOWN =0,	
	OB_TYPE_TYPE,			
	OB_TYPE_DIRECTORY,	
	OB_TYPE_SYMBOLIC_LINK,
	OB_TYPE_TOKEN,		
	OB_TYPE_PROCESS,	
	OB_TYPE_THREAD,		
	OB_TYPE_JOB,			
	OB_TYPE_DEBUG_OBJECT,	
	OB_TYPE_EVENT,		
	OB_TYPE_EVENT_PAIR,		
	OB_TYPE_MUTANT,		
	OB_TYPE_CALLBACK,	
	OB_TYPE_SEMAPHORE,	
	OB_TYPE_TIMER,		
	OB_TYPE_PROFILE,		
	OB_TYPE_KEYED_EVENT,	
	OB_TYPE_WINDOWS_STATION,
	OB_TYPE_DESKTOP,		
	OB_TYPE_SECTION,		
	OB_TYPE_KEY,			
	OB_TYPE_PORT,			
	OB_TYPE_WAITABLE_PORT,	
	OB_TYPE_ADAPTER,		
	OB_TYPE_CONTROLLER,	
	OB_TYPE_DEVICE,	
	OB_TYPE_DRIVER,		
	OB_TYPE_IOCOMPLETION,
	OB_TYPE_FILE,		
	OB_TYPE_WMIGUID		
}SYSTEM_HANDLE_TYPE;


typedef enum _SYSTEM_INFORMATION_CLASS
{
	SystemBasicInformation,
	SystemProcessorInformation,             // obsolete...delete
	SystemPerformanceInformation,
	SystemTimeOfDayInformation,
	SystemPathInformation,
	SystemProcessInformation,
	SystemCallCountInformation,
	SystemDeviceInformation,
	SystemProcessorPerformanceInformation,
	SystemFlagsInformation,
	SystemCallTimeInformation,
	SystemModuleInformation,//
	SystemLocksInformation,
	SystemStackTraceInformation,
	SystemPagedPoolInformation,
	SystemNonPagedPoolInformation,
	SystemHandleInformation,
	SystemObjectInformation,
	SystemPageFileInformation,
	SystemVdmInstemulInformation,
	SystemVdmBopInformation,
	SystemFileCacheInformation,
	SystemPoolTagInformation,
	SystemInterruptInformation,
	SystemDpcBehaviorInformation,
	SystemFullMemoryInformation,
	SystemLoadGdiDriverInformation,
	SystemUnloadGdiDriverInformation,
	SystemTimeAdjustmentInformation,
	SystemSummaryMemoryInformation,
	SystemMirrorMemoryInformation,
	SystemPerformanceTraceInformation,
	SystemObsolete0,
	SystemExceptionInformation,
	SystemCrashDumpStateInformation,
	SystemKernelDebuggerInformation,
	SystemContextSwitchInformation,
	SystemRegistryQuotaInformation,
	SystemExtendServiceTableInformation,
	SystemPrioritySeperation,
	SystemVerifierAddDriverInformation,
	SystemVerifierRemoveDriverInformation,
	SystemProcessorIdleInformation,
	SystemLegacyDriverInformation,
	SystemCurrentTimeZoneInformation,
	SystemLookasideInformation,
	SystemTimeSlipNotification,
	SystemSessionCreate,
	SystemSessionDetach,
	SystemSessionInformation,
	SystemRangeStartInformation,
	SystemVerifierInformation,
	SystemVerifierThunkExtend,
	SystemSessionProcessInformation,
	SystemLoadGdiDriverInSystemSpace,
	SystemNumaProcessorMap,
	SystemPrefetcherInformation,
	SystemExtendedProcessInformation,
	SystemRecommendedSharedDataAlignment,
	SystemComPlusPackage,
	SystemNumaAvailableMemory,
	SystemProcessorPowerInformation,
	SystemEmulationBasicInformation,
	SystemEmulationProcessorInformation,
	SystemExtendedHandleInformation,
	SystemLostDelayedWriteInformation,
	SystemBigPoolInformation,
	SystemSessionPoolTagInformation,
	SystemSessionMappedViewInformation,
	SystemHotpatchInformation,
	SystemObjectSecurityMode,
	SystemWatchdogTimerHandler,
	SystemWatchdogTimerInformation,
	SystemLogicalProcessorInformation,
	SystemWow64SharedInformation,
	SystemRegisterFirmwareTableInformationHandler,
	SystemFirmwareTableInformation,
	SystemModuleInformationEx,
	SystemVerifierTriageInformation,
	SystemSuperfetchInformation,
	SystemMemoryListInformation,
	SystemFileCacheInformationEx,
	MaxSystemInfoClass 
} SYSTEM_INFORMATION_CLASS;




typedef struct _SYSTEM_HANDLE_TABLE_ENTRY_INFOR
{
	USHORT	UniqueProcessId;
	USHORT	CreatorBackTraceIndex;
	UCHAR	ObjectTypeIndex;
	UCHAR	HandleAttributes;
	USHORT	HandleValue;
	PVOID	Object;
	ULONG	GrantedAccess;
} SYSTEM_HANDLE_TABLE_ENTRY_INFOR, *PSYSTEM_HANDLE_TABLE_ENTRY_INFOR;

typedef struct _SYSTEM_HANDLE_INFOR
{
	ULONG NumberOfHandles;
	SYSTEM_HANDLE_TABLE_ENTRY_INFOR Handles[1];
} SYSTEM_HANDLE_INFOR, *PSYSTEM_HANDLE_INFOR;

#define ObjectNameInformation 1
#endif	
