#pragma once
#include "ProcessManagerDrv.h"




extern
	PPEB
	PsGetProcessPeb(PEPROCESS Process);


BOOLEAN IsProcessDie(PEPROCESS EProcess);

NTSTATUS EnumProcessByOpenProcess(PVOID OutputBuffer,ULONG_PTR uOutSize);   
NTSTATUS EnumProcessByActiveList(PPROCESS_INFORMATION_OWN OutputBuffer,ULONG_PTR OutSize);
NTSTATUS EnumProcessByPspCidTable(PPROCESS_INFORMATION_OWN OutputBuffer,ULONG_PTR OutSize);


ULONG_PTR GetPspCidTableAddress()  ;
VOID ScanHandleTableToEnumProcess(PPROCESS_INFORMATION_OWN OutputBuffer, ULONG_PTR ulCnt);


NTSTATUS EnumTable1(ULONG_PTR uTableCode, PPROCESS_INFORMATION_OWN OutputBuffer, ULONG_PTR ulCnt);
NTSTATUS EnumTable2(ULONG_PTR uTableCode, PPROCESS_INFORMATION_OWN OutputBuffer, ULONG_PTR ulCnt);
NTSTATUS EnumTable3(ULONG_PTR uTableCode,PPROCESS_INFORMATION_OWN OutputBuffer, ULONG_PTR ulCnt);

VOID InsertProcess(PEPROCESS EProcess, PPROCESS_INFORMATION_OWN OutputBuffer, ULONG_PTR ulCnt);

ULONG_PTR KeGetObjectType(PVOID Object);
typedef ULONG_PTR 
	(*pfnObGetObjectType)(PVOID pObject);

BOOLEAN PsGetDosName(PEPROCESS ProcessObject, PUNICODE_STRING *DosName);

NTSTATUS  
	PsReferenceProcessFilePointer (  
	IN PEPROCESS Process,  
	OUT PVOID *pFilePointer  
	);  

extern 
	UCHAR *
	PsGetProcessImageFileName(
	__in PEPROCESS Process
	);
extern
NTSTATUS
	NTAPI
	ZwQueryInformationProcess (
	__in HANDLE ProcessHandle,
	__in PROCESSINFOCLASS ProcessInformationClass,
	__out_bcount(ProcessInformationLength) PVOID ProcessInformation,
	__in ULONG ProcessInformationLength,
	__out_opt PULONG ReturnLength
	);

BOOLEAN UnicodeToChar(CHAR** szDest,PUNICODE_STRING uniSource);


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


BOOLEAN  GetProcessPathBySectionObject(PEPROCESS EProcess,WCHAR* wzProcessPath);
BOOLEAN GetPathByFileObject(PFILE_OBJECT FileObject, WCHAR* wzPath);

