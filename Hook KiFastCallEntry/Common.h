#pragma once
#include "Monitor.h"


//½ø³Ì¼à¿Ø
#define CTL_MONITORPROCESS \
	CTL_CODE(FILE_DEVICE_UNKNOWN,0x830,METHOD_NEITHER,FILE_ANY_ACCESS)
#define CTL_GETRPROCESSINFOR \
	CTL_CODE(FILE_DEVICE_UNKNOWN,0x831,METHOD_NEITHER,FILE_ANY_ACCESS)
#define CTL_SUSMONITOR \
	CTL_CODE(FILE_DEVICE_UNKNOWN,0x832,METHOD_NEITHER,FILE_ANY_ACCESS)
#define CTL_SET_WHITE \
	CTL_CODE(FILE_DEVICE_UNKNOWN,0x833,METHOD_NEITHER,FILE_ANY_ACCESS)
#define CTL_SET_BLACK \
	CTL_CODE(FILE_DEVICE_UNKNOWN,0x834,METHOD_NEITHER,FILE_ANY_ACCESS)



#define CTL_GETFUNCBUFFER \
	CTL_CODE(FILE_DEVICE_UNKNOWN,0x840,METHOD_NEITHER,FILE_ANY_ACCESS)
#define CTL_SETFUNCEVENT \
	CTL_CODE(FILE_DEVICE_UNKNOWN,0x841,METHOD_NEITHER,FILE_ANY_ACCESS)



typedef struct _FUNC_DATA_{
	ULONG pid;
	ULONG tid;
	ULONG sid;
	UCHAR argc;
	UCHAR bFromUser;
	UCHAR bFromSSDT;
	ULONG args[16];
	LARGE_INTEGER time;
	//LIST_ENTRY ListEntry;
} FUNCDATA,*PFUNCDATA,**PPFUNCDATA;



typedef struct _MSG_
{
	ULONG  ulCreate;
	WCHAR wzProcessPath[512];
}MSG,*PMSG;


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


VOID InitGlobalVariable();


BOOLEAN  GetProcessPathBySectionObject(PEPROCESS EProcess,WCHAR* wzProcessPath);
BOOLEAN GetPathByFileObject(PFILE_OBJECT FileObject, WCHAR* wzPath);

KIRQL 
	SecCloseProtect();
VOID 
	SecOpenProtect(KIRQL Irql);

typedef struct _CONTROL_AREA32
{
	PVOID Segment;
	LIST_ENTRY DereferenceList;
	ULONG32 NumberOfSectionReferences;
	ULONG32 NumberOfPfnReferences;
	ULONG32 NumberOfMappedViews;
	ULONG32 NumberOfSystemCacheViews;
	ULONG32 NumberOfUserReferences;
	union
	{
		ULONG LongFlags;
		ULONG Flags;
	} u;
	PFILE_OBJECT FilePointer; 
} CONTROL_AREA32, *PCONTROL_AREA32;
typedef  struct _CONTROL_AREA64
{
	PVOID Segment;
	LIST_ENTRY DereferenceList;
	ULONG64 NumberOfSectionReferences;
	ULONG64 NumberOfPfnReferences;
	ULONG64 NumberOfMappedViews;
	ULONG64 NumberOfUserReferences;
	union
	{
		ULONG LongFlags;
		ULONG Flags;
	} u;
	PFILE_OBJECT FilePointer;
}CONTROL_AREA64,*PCONTROL_AREA64;



typedef struct  _SEGMENT_OBJECT32  
{
	struct _CONTROL_AREA32* ControlArea;
	ULONG32  TotalNumberOfPtes;
	ULONG32  NonExtendedPtes;
	ULONG32  WritableUserReferences;
	ULONG64  SizeOfSegment;
	ULONG32  SegmentPteTemplate;
	ULONG32  NumberOfCommittedPages;
	PVOID    ExtendInfo;
	PVOID    SystemImageBase;
	PVOID    BasedAddress;
	ULONG64  Unknown;
	PVOID    PrototypePte;
	ULONG32  ThePtes;
}SEGMENT_OBJECT32,*PSEGMENT_OBJECT32;
typedef struct  _SEGMENT_OBJECT64
{
	struct _CONTROL_AREA64* ControlArea;
	ULONG32  TotalNumberOfPtes;
	ULONG32  SegmentFlags;
	ULONG64  NumberOfCommittedPages;
	ULONG64  SizeOfSegment;
	union  
	{
		ULONG64 ExtendInfo;
		ULONG64 BasedAddress;
	}Unknow;
	ULONG64 SegmentLock;
	ULONG64 u1;
	ULONG64 u2;
	ULONG64 PrototypePte;
	ULONG64 ThePtes;
}SEGMENT_OBJECT64,*PSEGMENT_OBJECT64;




typedef struct _SECTION_OBJECT32 
{
	PVOID StartingVa;
	PVOID EndingVa;
	PVOID Parent;
	PVOID LeftChild;
	PVOID RightChild;
	struct _SEGMENT_OBJECT32* Segment; 
}SECTION_OBJECT32,*PSECTION_OBJECT32;
typedef struct _SECTION_OBJECT64
{
	PVOID StartingVa;
	PVOID EndingVa;
	PVOID Parent;
	PVOID LeftChild;
	PVOID RightChild;
	struct _SEGMENT_OBJECT64* Segment; 
}SECTION_OBJECT64,*PSECTION_OBJECT64;




typedef enum _SECTION_INFORMATION_CLASS {
	SectionBasicInformation,
	SectionImageInformation,
	MaxSectionInfoClass  // MaxSectionInfoClass should always be the last enum
} SECTION_INFORMATION_CLASS;


typedef struct _SECTION_IMAGE_INFORMATION {
	PVOID TransferAddress;
	ULONG ZeroBits;
	SIZE_T MaximumStackSize;
	SIZE_T CommittedStackSize;
	ULONG SubSystemType;
	union {
		struct {
			USHORT SubSystemMinorVersion;
			USHORT SubSystemMajorVersion;
		};
		ULONG SubSystemVersion;
	};
	ULONG GpValue;
	USHORT ImageCharacteristics;
	USHORT DllCharacteristics;
	USHORT Machine;
	BOOLEAN ImageContainsCode;
	BOOLEAN Spare1;
	ULONG LoaderFlags;
	ULONG ImageFileSize;
	ULONG Reserved[ 1 ];
} SECTION_IMAGE_INFORMATION, *PSECTION_IMAGE_INFORMATION;



extern
	NTSTATUS
	NTAPI
	ZwQuerySection (
	__in HANDLE SectionHandle,
	__in SECTION_INFORMATION_CLASS SectionInformationClass,
	__out_bcount(SectionInformationLength) PVOID SectionInformation,
	__in SIZE_T SectionInformationLength,
	__out_opt PSIZE_T ReturnLength
	);


typedef  enum _SYSTEM_INFORMATION_CLASS  
{
	SystemBasicInformation,  
	SystemProcessorInformation,  
	SystemPerformanceInformation,    
	SystemTimeOfDayInformation,    
	SystemPathInformation,    
	SystemProcessInformation,    
	SystemCallCountInformation,    
	SystemDeviceInformation,    
	SystemProcessorPerformanceInformation,    
	SystemFlagsInformation,    
	SystemCallTimeInformation,    
	SystemModuleInformation,    
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
	SystemRangeStartInformation ,   
	SystemVerifierInformation ,   
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

typedef unsigned char BYTE;

typedef unsigned char *LPBYTE;

typedef unsigned char *PBYTE;

typedef long BOOL;

typedef ULONG DWORD;

typedef ULONG *PDWORD;

typedef unsigned short WORD;

typedef unsigned int UINT;

typedef UINT_PTR WPARAM;

typedef LONG_PTR LPARAM;

typedef ULONG FARADDRESS;

typedef void* HDESK;

typedef void* LPVOID;
#define CALLBACK __stdcall