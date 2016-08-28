#pragma once

#include "Monitor.h"
#include <ntimage.h>
typedef unsigned char  BYTE, *PBYTE;
#define  LOBYTE(w) ((BYTE)(SHORT)(w))
#define  HIBYTE(w) ((BYTE)((SHORT)(w)>>8))
#define HIWORD(w) ((SHORT)((ULONG)(w))>>16)
#define HIDWORD(w) ((ULONG)(((LONGLONG)(w)>>32)&0xFFFFFFFF))


typedef HANDLE HWND;


typedef struct _KSERVICE_TABLE_DESCRIPTOR {
	PULONG_PTR Base;
	PULONG Count;
	ULONG Limit;
	PUCHAR Number;
} KSERVICE_TABLE_DESCRIPTOR, *PKSERVICE_TABLE_DESCRIPTOR;

int (__stdcall *OriZwDisplayStringAddress)(int);

NTSTATUS _KisInstallKiFastCallEntryHook();
NTSTATUS _HookZwDisplayString();
ULONG __stdcall _FakeZwDisplayString(PCUNICODE_STRING StringTest);
ULONG __stdcall _GetKiFastCallEntryRetuanAddress(PVOID VirtualAddress);


NTSTATUS __stdcall _KisGetKiFastCallEntryMoveEbxAddress(ULONG StartAddress, ULONG Size, ULONG *MoveEbxAddress);
ULONG __stdcall _KisGetKiFastCallEntryHookAddress(ULONG StartAddress, ULONG Length, ULONG *MmUserProbeAddress1);
NTSTATUS __stdcall _KisHookKiFastCallEntryPoint(ULONG HookAddress);

VOID _KifakeFastCallEntry();

ULONG __stdcall HookKiFastCallEntry(signed __int64 *a1, signed __int64 a2);

NTSTATUS _KisHookKiFastCallEntry();
ULONG __stdcall _KisSyscallFilter(ULONG FuncAddress,ULONG edx  ,ULONG Index ,ULONG Argc, PVOID SSDTBase);
//VOID __stdcall Filter(ULONG ServiceId, ULONG TableBase, ULONG Argc, ULONG StackAddr)

VOID SetFakeFunction();


NTSYSAPI
	PIMAGE_NT_HEADERS
	NTAPI
	RtlImageNtHeader(PVOID Base);

NTSTATUS FuncMonitorDeviceControl(ULONG_PTR  uIoControlCode,PVOID InputBuffer,ULONG_PTR uInSize,PVOID OutputBuffer,ULONG_PTR uOutSize);

PVOID 
	GetFunctionAddressByNameFromNtosExport(WCHAR *wzFunctionName);
PVOID GetKeShadowServiceDescriptorTable32();
PVOID GetKeShadowServiceDescriptorTable64();

typedef struct _XBoxData_ {
	ULONG pid;
	ULONG tid;
	ULONG sid;
	UCHAR argc;
	UCHAR bFromUser;
	UCHAR bFromSSDT;
	ULONG args[16];
	LARGE_INTEGER time;
	LIST_ENTRY ListEntry;
} XBoxData, *PXBoxData, **PPXBoxData;

NTSTATUS FuncEventToKernelEvent(PULONG_PTR InputBuffer,ULONG_PTR InSize);


#define SDOW_MAX_NUM 850
#define SSDT_MAX_NUM 500
#define FSD_MAX_NUM 56
#define KBD_MAX_NUM 28
#define IDT_MAX_NUM 256
#define MAX_REASON 350
#define OPCODE_LEN 10
#define SYSMODULE_MAX 256

#define SSDT_MAX_HOOK_INDEX 30

enum _SSDT_HOOK_INDEX
{
	SSDT_NTOPENPROCESS_INDEX = 0,
	SSDT_NTCREATETHREAD_INDEX = 1,
	SSDT_NTOPENTHREAD_INDEX = 2,
	SSDT_NTWRITEVIRTUALMEMORY_INDEX = 3,
	SSDT_NTDUPLICATEOBJECT_INDEX = 4,
	SSDT_NTSETSYSTEMINFORMATION_INDEX = 5,
	SSDT_NTQUEUEAPCTHREAD_INDEX  = 6 ,
	SSDT_NTDEBUGACTIVEPROCESS_INDEX = 7,
	SSDT_NTREMOVEPROCESSDEBUG_INDEX = 8

}SSDT_HOOK_INDEX;

enum _SHADOW_SSDT_HOOK_INDEX
{
	SHADOWSSDT_NTUSERFINDWINDOWSEX_INDEX = 0,
	SHADOWSSDT_NTUSERQUERYWINDOW_INDEX = 1 ,
	SHADOWSSDT_NTUSERSETWINDOWSHOOKEX_INDEX =  2,
	SHADOWSSDT_NTUSERBUILDHWNDLIST_INDEX = 3,
	SHADOWSSDT_NTUSERWINDOWFROMPOINT_INDEX = 4,
	SHADOWSSDT_NTUSERSETPARENT_INDEX = 5,
	SHADOWSSDT_NTUSERPOSTMESSAGE_INDEX = 6,
	SHADOWSSDT_NTUSERMESSAGECALL_INDEX = 7,
	SHADOWSSDT_NTUSERDESTROYWINDOW_INDEX = 8,
	SHADOWSSDT_NTUSERSHOWWINDOW_INDEX = 9,
	SHADOWSSDT_NTUSERSETWINDOWLONG_INDEX = 10 ,
	SHADOWSSDT_NTUSERINTERNALGETWINDOWTEXT_INDEX = 11,
	SHADOWSSDT_NTUSERSETWINDOWPOS_INDEX = 12 ,
	SHADOWSSDT_NTUSERGETCLASSNAME_INDEX = 13 ,
	SHADOWSSDT_NTUSERGETWINDOWPLACEMENT_INDEX = 14,
	SHADOWSSDT_NTUSERGETFOREGROUNDWINDOW_INDEX    = 15

}SHADOW_SSDT_HOOK_INDEX;

#define SSDT_FILTER_NUM 500
#define SHADOWSSDT_FILTER_NUM 850

#define SECTIONOBJECT 0x138 //_EPROCESS中SectionObject的offset
#define SEGMENT 0x014 //_SECTION_OBJECT中Segment的//
#define CONTROLAREA 0x000 //_SEGMENT中ControlArea的offset
#define FILEPOINTER 0x024 //_CONTROL_AREA中FilePointer的offset



//TEB初始化
typedef struct _INITIAL_TEB {
	struct {
		PVOID OldStackBase;
		PVOID OldStackLimit;
	} OldInitialTeb;
	PVOID StackBase;
	PVOID StackLimit;
	PVOID StackAllocationBase;
} INITIAL_TEB, *PINITIAL_TEB;



typedef struct _SYSCALL_FILTER_INFO_TABLE
{
	ULONG ulKiFastCallEntry;
	ULONG ulNewRetKiFastCallEntry;

	UCHAR HookCode[13];
	ULONG ulCopyLength;

	ULONG ulSSDTAddr;
	ULONG ulSHADOWSSDTAddr;

	ULONG ulSSDTNum;
	ULONG ulSHADOWSSDTNum;

	ULONG FastCallType;

	ULONG SavedSSDTTable[SSDT_FILTER_NUM];                  //SSDT原始函数地址表
	ULONG ProxySSDTTable[SSDT_FILTER_NUM];            //SSDT代理函数地址表
	ULONG SSDTServiceIndex[SSDT_MAX_HOOK_INDEX];
	ULONG SSDTSwitchTable[SSDT_FILTER_NUM];              //SSDT Hook开关表

	ULONG SavedShadowSSDTTable[SHADOWSSDT_FILTER_NUM];            //ShadowSSDT原始函数地址表
	ULONG ProxyShadowSSDTTable[SHADOWSSDT_FILTER_NUM];      //ShadowSSDT代理函数地址表
	ULONG ShadowSSDTServiceIndex[SSDT_MAX_HOOK_INDEX];
	ULONG ShadowSSDTSwitchTable[SHADOWSSDT_FILTER_NUM];  //ShadowSSDT Hook开关表


	ULONG ulEPROCESSPidOffset;	

}SYSCALL_FILTER_INFO_TABLE,*PSYSCALL_FILTER_INFO_TABLE;


typedef BOOL (*NTUSERPOSTTHREADMESSAGE)
	(
	DWORD idThread, 
	UINT Msg, 
	WPARAM wParam, 
	LPARAM lParam
	);



typedef NTSTATUS (*NTDUPLICATEOBJECT)
	(
	IN HANDLE SourceProcessHandle,
	IN HANDLE SourceHandle,
	IN HANDLE TargetProcessHandle,
	OUT PHANDLE TargetHandle OPTIONAL,
	IN ACCESS_MASK DesiredAccess,
	IN ULONG Attributes,
	IN ULONG Options
	);

typedef NTSTATUS (*NTCREATEDEBUGOBJECT)
	(
	OUT PHANDLE  DebugObjectHandle,  
	IN ACCESS_MASK  DesiredAccess,  
	IN POBJECT_ATTRIBUTES  ObjectAttributes,  
	IN ULONG  Flags   
	);

typedef NTSTATUS (*NTQUEUEAPCTHREAD)
	(
	IN HANDLE ThreadHandle,
	IN PKNORMAL_ROUTINE ApcRoutine,
	IN PVOID ApcContext OPTIONAL,
	IN PVOID Argument1 OPTIONAL,
	IN PVOID Argument2 OPTIONAL
	);

typedef NTSTATUS (*NTSETTHREADCONTEXT)
	(
	IN HANDLE hThread,
	IN PCONTEXT pContext
	);



typedef NTSTATUS (*HOOKPROC)(int code, WPARAM wParam, LPARAM lParam);


typedef NTSTATUS (*NTSETSYSTEMINFORMATION)(
	__in SYSTEM_INFORMATION_CLASS SystemInformationClass,
	__inout PVOID SystemInformation,
	__in ULONG SystemInformationLength);


NTSTATUS FakeNtSetSystemInformation(
	__in SYSTEM_INFORMATION_CLASS SystemInformationClass,
	__inout PVOID SystemInformation,
	__in ULONG SystemInformationLength);

typedef NTSTATUS (*NTOPENPROCESS) (
	__out PHANDLE ProcessHandle,
	__in ACCESS_MASK DesiredAccess,
	__in POBJECT_ATTRIBUTES ObjectAttributes,
	__in_opt PCLIENT_ID ClientId);


NTSTATUS FakeNtOpenProcess (
	__out PHANDLE ProcessHandle,
	__in ACCESS_MASK DesiredAccess,
	__in POBJECT_ATTRIBUTES ObjectAttributes,
	__in_opt PCLIENT_ID ClientId
	);


NTSTATUS FakeNtCreateThread(
	__out PHANDLE ThreadHandle,
	__in ACCESS_MASK DesiredAccess,
	__in_opt POBJECT_ATTRIBUTES ObjectAttributes,
	__in HANDLE ProcessHandle,
	__out PCLIENT_ID ClientId,
	__in PCONTEXT ThreadContext,
	__in PINITIAL_TEB InitialTeb,
	__in BOOLEAN CreateSuspended
	);

typedef NTSTATUS (*NTCREATETHREAD)(
	__out PHANDLE ThreadHandle,
	__in ACCESS_MASK DesiredAccess,
	__in_opt POBJECT_ATTRIBUTES ObjectAttributes,
	__in HANDLE ProcessHandle,
	__out PCLIENT_ID ClientId,
	__in PCONTEXT ThreadContext,
	__in PINITIAL_TEB InitialTeb,
	__in BOOLEAN CreateSuspended);

typedef NTSTATUS (*NTOPENTHREAD) (
	__out PHANDLE ThreadHandle,
	__in ACCESS_MASK DesiredAccess,
	__in POBJECT_ATTRIBUTES ObjectAttributes,
	__in_opt PCLIENT_ID ClientId);

NTSTATUS FakeNtOpenThread (
	__out PHANDLE ThreadHandle,
	__in ACCESS_MASK DesiredAccess,
	__in POBJECT_ATTRIBUTES ObjectAttributes,
	__in_opt PCLIENT_ID ClientId
	);


typedef NTSTATUS (*NTWRITEVIRTUALMEMORY)(
	__in HANDLE ProcessHandle,
	__in_opt PVOID BaseAddress,
	__in_bcount(BufferSize) CONST VOID *Buffer,
	__in SIZE_T BufferSize,
	__out_opt PSIZE_T NumberOfBytesWritten);

NTSTATUS FakeNtWriteVirtualMemory(
	__in HANDLE ProcessHandle,
	__in_opt PVOID BaseAddress,
	__in_bcount(BufferSize) CONST VOID *Buffer,
	__in SIZE_T BufferSize,
	__out_opt PSIZE_T NumberOfBytesWritten
	);


typedef NTSTATUS (*NTDUPLICATEOBJECT)(
	__in HANDLE SourceProcessHandle,
	__in HANDLE SourceHandle,
	__in_opt HANDLE TargetProcessHandle,
	__out_opt PHANDLE TargetHandle,
	__in ACCESS_MASK DesiredAccess,
	__in ULONG HandleAttributes,
	__in ULONG Options);

NTSTATUS FakeNtDuplicateObject (
	__in HANDLE SourceProcessHandle,
	__in HANDLE SourceHandle,
	__in_opt HANDLE TargetProcessHandle,
	__out_opt PHANDLE TargetHandle,
	__in ACCESS_MASK DesiredAccess,
	__in ULONG HandleAttributes,
	__in ULONG Options);

typedef NTSTATUS (*NTDEBUGACTIVEPROCESS)  ( IN HANDLE  ProcessHandle,  
	IN HANDLE  DebugObjectHandle   
	);

NTSTATUS FakeNtDebugActiveProcess  (IN HANDLE  ProcessHandle,  
	IN HANDLE  DebugObjectHandle   
	);

typedef NTSTATUS (*NTREMOVEPROCESSDEBUG)  ( IN HANDLE  ProcessHandle,  
	IN HANDLE  DebugObjectHandle   
	);

NTSTATUS FakeNtRemoveProcessDebug  ( IN HANDLE  ProcessHandle,  
	IN HANDLE  DebugObjectHandle   
	) ;

/************************窗口过滤相关函数********************************/

typedef HWND (*NTUSERFINDWINDOWEX)(
	__in HWND hwndParent, 
	__in HWND hwndChild, 
	__in PUNICODE_STRING pstrClassName OPTIONAL, 
	__in PUNICODE_STRING pstrWindowName OPTIONAL, 
	__in DWORD dwType);

HWND FakeNtUserFindWindowEx(
	__in HWND hwndParent, 
	__in HWND hwndChild, 
	__in PUNICODE_STRING pstrClassName OPTIONAL, 
	__in PUNICODE_STRING pstrWindowName OPTIONAL, 
	__in DWORD dwType);


typedef UINT_PTR (*NTUSERQUERYWINDOW)(
	IN ULONG WindowHandle,
	IN ULONG TypeInformation);

UINT_PTR FakeNtUserQueryWindow(
	IN ULONG WindowHandle,
	IN ULONG TypeInformation);



typedef
	NTSTATUS (*NtQUEUEAPCTHREAD)(
	IN HANDLE ThreadHandle,
	IN PKNORMAL_ROUTINE ApcRoutine,
	IN PVOID ApcContext OPTIONAL,
	IN PVOID Argument1 OPTIONAL,
	IN PVOID Argument2 OPTIONAL
	);

NTSTATUS
	NTAPI FakeNtQueueApcThread(
	IN HANDLE ThreadHandle,
	IN PKNORMAL_ROUTINE ApcRoutine,
	IN PVOID ApcContext OPTIONAL,
	IN PVOID Argument1 OPTIONAL,
	IN PVOID Argument2 OPTIONAL
	);


typedef HANDLE (*NTUSERSETWINDOWSHOOKEX)(IN HANDLE Mod,
	IN PUNICODE_STRING UnsafeModuleName,
	IN DWORD ThreadId,
	IN INT HookId,
	IN HOOKPROC HookProc,
	IN BOOL Ansi);


HANDLE  FakeNtUserSetWindowsHookEx(HANDLE Mod,
	PUNICODE_STRING UnsafeModuleName,
	DWORD ThreadId,
	INT HookId,
	HOOKPROC HookProc,
	BOOL Ansi);


typedef NTSTATUS (*NTUSERBUILDHWNDLIST)(
	IN HDESK hdesk, 
	IN HWND hwndNext, 
	IN ULONG fEnumChildren, 
	IN DWORD idThread, 
	IN UINT cHwndMax, 
	OUT HWND *phwndFirst, 
	OUT ULONG* pcHwndNeeded);



NTSTATUS FakeNtUserBuildHwndList(
	IN HDESK hdesk, 
	IN HWND hwndNext, 
	IN ULONG fEnumChildren, 
	IN DWORD idThread, 
	IN UINT cHwndMax, 
	OUT HWND *phwndFirst, 
	OUT ULONG* pcHwndNeeded);

typedef ULONG (*NTUSERWINDOWFROMPOINT)(LONG x, LONG y);

ULONG FakeNtUserWindowFromPoint(LONG x, LONG y);

typedef HWND (*NTUSERSETPARENT)(HWND hWndChild,HWND hWndNewParent);

HWND FakeNtUserSetParent(HWND hWndChild,HWND hWndNewParent);

typedef BOOL (*NTUSERPOSTMESSAGE)(HWND hWnd,UINT Msg,WPARAM wParam,LPARAM lParam);


BOOL FakeNtUserPostMessage(HWND hWnd,UINT Msg,WPARAM wParam,LPARAM lParam);

typedef ULONG (*NTUSERMESSAGECALL)(
	HWND hwnd, 
	UINT msg,
	WPARAM wParam,
	LPARAM lParam,
	PULONG xParam,
	ULONG xpfnProc,
	BOOL bAnsi
	);

ULONG FakeNtUserMessageCall(
	HWND hwnd, 
	UINT msg,
	WPARAM wParam,
	LPARAM lParam,
	PULONG xParam,
	ULONG xpfnProc,
	BOOL bAnsi
	);

typedef BOOL (*NTUSERDESTROYWINDOW)(HWND hWnd);

BOOL FakeNtUserDestroyWindow( HWND hWnd );

typedef BOOL (*NTUSERSHOWWINDOW)(HWND hWnd,long nCmdShow);

BOOL FakeNtUserShowWindow(HWND hWnd,long nCmdShow);

typedef long (*NTUSERSETWINDOWLONG)(HWND hWnd, ULONG Index, long NewValue, BOOL Ansi);

long FakeNtUserSetWindowLong(HWND hWnd, ULONG Index, long NewValue, BOOL Ansi);

typedef INT (*NTUSERINTERNALGETWINDOWTEXT)(HWND hWnd, LPWSTR lpString, INT nMaxCount);

INT CALLBACK FakeNtUserInternalGetWindowText(HWND hWnd, LPWSTR lpString, INT nMaxCount);


typedef BOOL (*NTUSERSETWINDOWPOS)(
	HWND hWnd,
	HWND hWndInsertAfter,
	int X,
	int Y,
	int cx,
	int cy,
	UINT uFlags);

BOOL CALLBACK
	FakeNtUserSetWindowPos(
	HWND hWnd,
	HWND hWndInsertAfter,
	int X,
	int Y,
	int cx,
	int cy,
	UINT uFlags);

typedef (*NTUSERGETCLASSNAME)(IN HWND hWnd,
	OUT PUNICODE_STRING ClassName,
	IN BOOL Ansi);

INT CALLBACK
	FakeNtUserGetClassName (IN HWND hWnd,
	OUT PUNICODE_STRING ClassName,
	IN BOOL Ansi);

typedef BOOL (*NTUSERGETWINDOWPLACEMENT)(HWND hWnd,
	void* lpwndpl);


BOOL CALLBACK
	FakeNtUserGetWindowPlacement(HWND hWnd,
	void* lpwndpl);

typedef ULONG (*NTUSERGETFOREGROUNDWINDOW)(VOID);


ULONG FakeNtUserGetForegroundWindow(VOID);
/*****************************************************************************************************/
