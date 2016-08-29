#pragma once
#include "ProcessManagerRing0.h"



VOID RemoveNodeFromActiveProcessLinks(ULONG_PTR ProcessId);


VOID WPOFF();
VOID WPON();

VOID EraseObjectFromHandleTable1(ULONG_PTR ProcessId);
NTSTATUS EnumTable11(ULONG_PTR uTableCode,ULONG_PTR ProcessId);
NTSTATUS EnumTable12(ULONG_PTR uTableCode,ULONG_PTR ProcessId);
NTSTATUS EnumTable13(ULONG_PTR uTableCode,ULONG_PTR ProcessId);


BOOLEAN ClearPspCidTable(PEPROCESS EProcess, ULONG_PTR PrcessId);