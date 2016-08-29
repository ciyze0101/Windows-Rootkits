#pragma once
#include "CommonR0.h"
#include "ProcessManagerRing0.h"



NTSTATUS EventToKernelEvent(PULONG_PTR InputBuffer,ULONG_PTR InSize);
NTSTATUS  RegisterProcessFilter();


VOID
	ProcessCallBackWin7(PEPROCESS  EProcess,HANDLE  ProcessId,PPS_CREATE_NOTIFY_INFO  CreateInfo);



FORCEINLINE
	VOID
	AcquireResourceExclusive(IN OUT PERESOURCE Resource);
FORCEINLINE
	VOID
	ReleaseResource(IN OUT PERESOURCE Resource);

VOID DestroyResource();