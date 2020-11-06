#pragma once
#include "Head.h"

BOOL	g_IsPhysicalOpInit = FALSE;
HANDLE	g_SectionHandle = NULL;

#define	PAGE_TABLE_SIZE			0x1000
#define	CR3_FLAG_ALL_BITS			0xFFF0000000000FFF
#define	PAGE_TABLE_PML4T_FLAG	0x867	//1000 0110 0111
#define	PAGE_TABLE_PDPT_FLAG	0x9E7	//1001 1110 0111
#define VA_SYSTEM_START			0xFFFF080000000000 //IA64
#define MAX_ENTRY_COUNT			512
#define ENTRY_SIZE				sizeof(ULONG64)

typedef struct _PHYSICAL_OP_CR3
{
	PVOID	pAllocVA_PML4T;
	PVOID	pAllocPA_PML4T;

	PVOID	pAllocVA_PDPT;
	PVOID	pAllocPA_PDPT;

	PVOID	pSystemPML4TMap;

	ULONG64	CR3Generated;
	ULONG64 CR3System;

	ULONG64 CR3BeforeSwitch;
	BOOL	IsContextSwitched;
	BOOL	IsIrqlChanged;
	KIRQL	OriginalIrql;//available if the IsIrqlChanged is true
}PHYSICAL_OP_CR3, *PPHYSICAL_OP_CR3;



typedef struct _MMPDPTE
{
	struct /* bitfield */
	{
		/* 0x0000 */ unsigned __int64 Valid : 1; /* bit position: 0 */
		/* 0x0000 */ unsigned __int64 Dirty1 : 1; /* bit position: 1 */
		/* 0x0000 */ unsigned __int64 Owner : 1; /* bit position: 2 */
		/* 0x0000 */ unsigned __int64 WriteThrough : 1; /* bit position: 3 */
		/* 0x0000 */ unsigned __int64 CacheDisable : 1; /* bit position: 4 */
		/* 0x0000 */ unsigned __int64 Accessed : 1; /* bit position: 5 */
		/* 0x0000 */ unsigned __int64 Dirty : 1; /* bit position: 6 */
		/* 0x0000 */ unsigned __int64 LargePage : 1; /* bit position: 7 */
		/* 0x0000 */ unsigned __int64 Global : 1; /* bit position: 8 */
		/* 0x0000 */ unsigned __int64 CopyOnWrite : 1; /* bit position: 9 */
		/* 0x0000 */ unsigned __int64 Unused : 1; /* bit position: 10 */
		/* 0x0000 */ unsigned __int64 Write : 1; /* bit position: 11 */
		/* 0x0000 */ unsigned __int64 reserved0 : 18; /* bit position: 12 */
		/* 0x0000 */ unsigned __int64 PageFrameNumber : 18; /* bit position: 30 */
		/* 0x0000 */ unsigned __int64 reserved1 : 4; /* bit position: 48 */
		/* 0x0000 */ unsigned __int64 SoftwareWsIndex : 11; /* bit position: 52 */
		/* 0x0000 */ unsigned __int64 NoExecute : 1; /* bit position: 63 */
	}; /* bitfield */
} MMPDPTE, *PMMPDPTE; /* size: 0x0008 */

typedef struct _MMPDE
{
	struct /* bitfield */
	{
		/* 0x0000 */ unsigned __int64 Valid : 1; /* bit position: 0 */
		/* 0x0000 */ unsigned __int64 Dirty1 : 1; /* bit position: 1 */
		/* 0x0000 */ unsigned __int64 Owner : 1; /* bit position: 2 */
		/* 0x0000 */ unsigned __int64 WriteThrough : 1; /* bit position: 3 */
		/* 0x0000 */ unsigned __int64 CacheDisable : 1; /* bit position: 4 */
		/* 0x0000 */ unsigned __int64 Accessed : 1; /* bit position: 5 */
		/* 0x0000 */ unsigned __int64 Dirty : 1; /* bit position: 6 */
		/* 0x0000 */ unsigned __int64 LargePage : 1; /* bit position: 7 */
		/* 0x0000 */ unsigned __int64 Global : 1; /* bit position: 8 */
		/* 0x0000 */ unsigned __int64 CopyOnWrite : 1; /* bit position: 9 */
		/* 0x0000 */ unsigned __int64 Unused : 1; /* bit position: 10 */
		/* 0x0000 */ unsigned __int64 Write : 1; /* bit position: 11 */
		/* 0x0000 */ unsigned __int64 reserved0 : 9; /* bit position: 12 */
		/* 0x0000 */ unsigned __int64 PageFrameNumber : 27; /* bit position: 21 */
		/* 0x0000 */ unsigned __int64 reserved1 : 4; /* bit position: 48 */
		/* 0x0000 */ unsigned __int64 SoftwareWsIndex : 11; /* bit position: 52 */
		/* 0x0000 */ unsigned __int64 NoExecute : 1; /* bit position: 63 */
	}; /* bitfield */
} MMPDE, *PMMPDE; /* size: 0x0008 */

typedef struct _MMVA
{
	struct /* bitfield */
	{
		/* 0x0000 */ unsigned __int64 Offset : 12;
		/* 0x0000 */ unsigned __int64 PT : 9;
		/* 0x0000 */ unsigned __int64 PDT : 9;
		/* 0x0000 */ unsigned __int64 PDPT : 9;
		/* 0x0000 */ unsigned __int64 PML4T : 9;
		/* 0x0000 */ unsigned __int64 Partition : 16; //User:0x0000 System:0xFFFF
	}; /* bitfield */
} MMVA, *PMMVA; /* size: 0x0008 */

typedef struct _MMVA_PDPTE_LARGE
{
	struct /* bitfield */
	{
		/* 0x0000 */ unsigned __int64 Offset : 30;
		/* 0x0000 */ unsigned __int64 PDPT : 9;
		/* 0x0000 */ unsigned __int64 PML4T : 9;
		/* 0x0000 */ unsigned __int64 Partition : 16; //User:0x0000 System:0xFFFF
	}; /* bitfield */
} MMVA_PDPTE_LARGE, *PMMVA_PDPTE_LARGE; /* size: 0x0008 */

typedef struct _MMVA_PDE_LARGE
{
	struct /* bitfield */
	{
		/* 0x0000 */ unsigned __int64 Offset : 21;
		/* 0x0000 */ unsigned __int64 PDT : 9;
		/* 0x0000 */ unsigned __int64 PDPT : 9;
		/* 0x0000 */ unsigned __int64 PML4T : 9;
		/* 0x0000 */ unsigned __int64 Partition : 16; //User:0x0000 System:0xFFFF
	}; /* bitfield */
} MMVA_PDE_LARGE, *PMMVA_PDE_LARGE; /* size: 0x0008 */

/*********************************************************
function:		OpenPhysicalMemory
				pMapPhysicalMemoryPre
				MapPhysicalMemory
				UnmapPhysicalMemory
description:	use map view of section to map physical address to virtual address
**********************************************************/
HANDLE OpenPhysicalMemory()
{
	UNICODE_STRING		physmemString;
	OBJECT_ATTRIBUTES	attributes;
	WCHAR				physmemName[] = L"\\device\\physicalmemory";
	NTSTATUS			status;
	HANDLE				physmem;
	RtlInitUnicodeString(&physmemString, physmemName);
	InitializeObjectAttributes(&attributes, &physmemString, OBJ_CASE_INSENSITIVE, NULL, NULL);
	status = ZwOpenSection(&physmem, SECTION_ALL_ACCESS, &attributes);
	if (!NT_SUCCESS(status))
	{
		return NULL;
	}
	return physmem;
}
BOOLEAN pMapPhysicalMemoryPre(HANDLE hMemory, PDWORD64 pDwAddress, PSIZE_T pSize, PDWORD64 pDwVirtualAddress)
{
	NTSTATUS ntStatus;

	LARGE_INTEGER viewBase;
	*pDwVirtualAddress = 0;
	viewBase.QuadPart = *pDwAddress;
	ntStatus = ZwMapViewOfSection(hMemory, (HANDLE)-1, (void**)pDwVirtualAddress, 0L, *pSize, &viewBase, pSize, ViewShare, 0, PAGE_READWRITE | PAGE_NOCACHE);
	if (!NT_SUCCESS(ntStatus))
		return FALSE;
	//*pDwAddress = viewBase.QuadPart;
	return TRUE;
}
PVOID MapPhysicalMemory(PVOID PA, SIZE_T Size)
{
	ULONGLONG DwAddress = (ULONG64)PA;
	ULONGLONG DwVirtualAddress = 0;
	BOOLEAN status = pMapPhysicalMemoryPre(g_SectionHandle, &DwAddress, &Size, &DwVirtualAddress);
	return (status == TRUE) ? (PVOID)DwVirtualAddress : NULL;
}
BOOLEAN UnmapPhysicalMemory(PVOID VA)
{
	if (!ZwUnmapViewOfSection((HANDLE)-1, VA))
		return TRUE;
	else
		return FALSE;
}

/*********************************************************
function:		GetCR3Flag
description:	get cr3 flag, only save the flag bits
**********************************************************/
ULONG64 GetCR3Flag(ULONG64 CR3)
{
	return (CR3 & CR3_FLAG_ALL_BITS);
}

/*********************************************************
function:		ClearCR3Flag
description:	clear cr3 flag, only clear the flag bits
**********************************************************/
ULONG64 ClearCR3Flag(ULONG64 CR3)
{
	return (CR3 & ~CR3_FLAG_ALL_BITS);
}

/*********************************************************
function:		pPrintPhysicalOpStructure
description:	print the structure elements
**********************************************************/
VOID pPrintPhysicalOpStructure(PPHYSICAL_OP_CR3 pPhysicalOpCR3)
{
	MyPrint(_TitleAndFunc"[PrintStart]\n");

	MyPrint(_TitleAndFunc"pPhysicalOpCR3->pAllocVA_PML4T:%16IX\n", pPhysicalOpCR3->pAllocVA_PML4T);
	MyPrint(_TitleAndFunc"pPhysicalOpCR3->pAllocPA_PML4T:%16IX\n", pPhysicalOpCR3->pAllocPA_PML4T);

	MyPrint(_TitleAndFunc"pPhysicalOpCR3->pAllocVA_PDPT:%16IX\n", pPhysicalOpCR3->pAllocVA_PDPT);
	MyPrint(_TitleAndFunc"pPhysicalOpCR3->pAllocPA_PDPT:%16IX\n", pPhysicalOpCR3->pAllocPA_PDPT);

	MyPrint(_TitleAndFunc"pPhysicalOpCR3->pSystemPML4TMap:%16IX\n", pPhysicalOpCR3->pSystemPML4TMap);

	MyPrint(_TitleAndFunc"pPhysicalOpCR3->CR3Generated:%16IX\n", pPhysicalOpCR3->CR3Generated);

	MyPrint(_TitleAndFunc"[PrintEnd]\n");
}

/*********************************************************
function:		pFreePhysicalOpPageTableMemory
description:	to free the allocated memory (PML4T and PDPT page table) with null pointer check
**********************************************************/
NTSTATUS pFreePhysicalOpPageTableMemory(PPHYSICAL_OP_CR3 pPhysicalOpCR3)
{
	if (pPhysicalOpCR3->pAllocVA_PML4T != NULL)
	{
		MmFreeNonCachedMemory(pPhysicalOpCR3->pAllocVA_PML4T, PAGE_TABLE_SIZE);
		pPhysicalOpCR3->pAllocVA_PML4T = NULL;
		pPhysicalOpCR3->pAllocPA_PML4T = NULL;
	}

	if (pPhysicalOpCR3->pAllocVA_PDPT != NULL)
	{
		MmFreeNonCachedMemory(pPhysicalOpCR3->pAllocVA_PDPT, PAGE_TABLE_SIZE);
		pPhysicalOpCR3->pAllocVA_PDPT = NULL;
		pPhysicalOpCR3->pAllocPA_PDPT = NULL;
	}
		
	return STATUS_SUCCESS;
}


/*********************************************************
function:		pAllocPhysicalOpPageTableMemory
description:	to allocate memory (PML4T and PDPT page table)
				if the allocate procduce failed,it can free all the allocated pages
calls:			MmAllocateNonCachedMemory
				MmGetPhysicalAddress
				pFreePhysicalOpPageTableMemory
**********************************************************/
NTSTATUS pAllocPhysicalOpPageTableMemory(PPHYSICAL_OP_CR3 pPhysicalOpCR3)
{
	//PML4T
	pPhysicalOpCR3->pAllocVA_PML4T = MmAllocateNonCachedMemory(PAGE_TABLE_SIZE);
	//check allocate state
	if (pPhysicalOpCR3->pAllocVA_PML4T == NULL)
		goto Lable_Error;
	pPhysicalOpCR3->pAllocPA_PML4T = (PVOID)MmGetPhysicalAddress(pPhysicalOpCR3->pAllocVA_PML4T).QuadPart;


	//PDPT
	pPhysicalOpCR3->pAllocVA_PDPT = MmAllocateNonCachedMemory(PAGE_TABLE_SIZE);
	//check allocate state
	if (pPhysicalOpCR3->pAllocVA_PDPT == NULL)
		goto Lable_Error;
	pPhysicalOpCR3->pAllocPA_PDPT = (PVOID)MmGetPhysicalAddress(pPhysicalOpCR3->pAllocVA_PDPT).QuadPart;


	return STATUS_SUCCESS;
Lable_Error:
	//free allocated memory
	pFreePhysicalOpPageTableMemory(pPhysicalOpCR3);
	return STATUS_UNSUCCESSFUL;
}

/*********************************************************
function:		pMapPML4T
description:	map the system cr3(pml4t) to virtual address
calls:			ClearCR3Flag
				OpenPhysicalMemory
				MapPhysicalMemory
**********************************************************/
NTSTATUS pMapSystemPML4T(ULONG64 SystemCR3, PPHYSICAL_OP_CR3 pPhysicalOpCR3)
{
	ULONG64 SystemCR3NonFlag = ClearCR3Flag(SystemCR3);
	PVOID	pSystemPML4T = (PVOID)SystemCR3NonFlag;

	if (g_SectionHandle == NULL)
		g_SectionHandle = OpenPhysicalMemory();

	pPhysicalOpCR3->pSystemPML4TMap = MapPhysicalMemory(pSystemPML4T, PAGE_TABLE_SIZE);

	return pPhysicalOpCR3->pSystemPML4TMap == NULL ? STATUS_UNSUCCESSFUL : STATUS_SUCCESS;
}

/*********************************************************
function:		pUnmapSystemPML4T
description:	unmap the system cr3(pml4t)
calls:			OpenPhysicalMemory
				UnmapPhysicalMemory
**********************************************************/
NTSTATUS pUnmapSystemPML4T(PPHYSICAL_OP_CR3 pPhysicalOpCR3)
{
	if (g_SectionHandle == NULL)
		g_SectionHandle = OpenPhysicalMemory();

	BOOL	State = UnmapPhysicalMemory(pPhysicalOpCR3->pSystemPML4TMap);
	if (State)
		pPhysicalOpCR3->pSystemPML4TMap = NULL;

	if (g_SectionHandle != NULL)
		ZwClose(g_SectionHandle);

	return !State ? STATUS_UNSUCCESSFUL : STATUS_SUCCESS;
}

/*********************************************************
function:		pFillGeneratedPML4TandPDPT
description:	fill the pml4t table,genarate the first large page entry and copy the system space map
				fill the pdpt table point to the physical address,every PDPTE point to a 1G-byte page(512G in total)
calls:			RtlCopyMemory
**********************************************************/
NTSTATUS pFillGeneratedPML4TandPDPT(PPHYSICAL_OP_CR3 pPhysicalOpCR3)
{
	//copy the system space map
	PVOID		pSystemStart = (PVOID)VA_SYSTEM_START;
	ULONG64		SystemPML4TStart = ((PMMVA)&pSystemStart)->PML4T;

	MyPrint(_TitleAndFunc"SystemPML4TStart:%16X\n", SystemPML4TStart);
	RtlCopyMemory((PVOID)((ULONG64)pPhysicalOpCR3->pAllocVA_PML4T + SystemPML4TStart*ENTRY_SIZE),
		(PVOID)((ULONG64)pPhysicalOpCR3->pSystemPML4TMap + SystemPML4TStart*ENTRY_SIZE),
		(MAX_ENTRY_COUNT - SystemPML4TStart)*ENTRY_SIZE
	);

	//make the first address point to my PDPT table
	*(PULONG64)pPhysicalOpCR3->pAllocVA_PML4T = (ULONG64)pPhysicalOpCR3->pAllocPA_PDPT | PAGE_TABLE_PML4T_FLAG;

	//fill the PDPT page table
	//add flag
	ULONG64 CurrentPDPTEntry = PAGE_TABLE_PDPT_FLAG;
	for (int i = 0; i < MAX_ENTRY_COUNT; i++)
	{
		//change pfn
		((PMMPDPTE)&CurrentPDPTEntry)->PageFrameNumber = i;
		//
		*(PULONG64)((ULONG64)pPhysicalOpCR3->pAllocVA_PDPT + i*ENTRY_SIZE) = CurrentPDPTEntry;
	}
	return STATUS_SUCCESS;
}
/*********************************************************
function:		CreatePhysicalOpCR3BySystemCR3
description:	to initialize the physical memory operation structure
calls:			pAllocPhysicalOpPageTableMemory
				pMapSystemPML4T
				pFillGeneratedPML4TandPDPT
				pMapPML4T
				GetCR3Flag
**********************************************************/
NTSTATUS CreatePhysicalOpCR3BySystemCR3(ULONG64 SystemCR3, PPHYSICAL_OP_CR3 pPhysicalOpCR3)
{
	//check the init state
	if (g_IsPhysicalOpInit)
		return STATUS_UNSUCCESSFUL;


	//allocate page table memory and fill the structure
	if (!NT_SUCCESS(pAllocPhysicalOpPageTableMemory(pPhysicalOpCR3)))
		return STATUS_UNSUCCESSFUL;

	//map pSystemPML4T to virtual address and fill the structure
	if (!NT_SUCCESS(pMapSystemPML4T(SystemCR3, pPhysicalOpCR3)))
		return STATUS_UNSUCCESSFUL;

	//fill PML4T and PDPT page table
	if (!NT_SUCCESS(pFillGeneratedPML4TandPDPT(pPhysicalOpCR3)))
		return STATUS_UNSUCCESSFUL;

	//generate new cr3 for reading the physical memory and add cr3 flag
	ULONG64	SystemCR3Flag = GetCR3Flag(SystemCR3);
	pPhysicalOpCR3->CR3Generated = (ULONG64)pPhysicalOpCR3->pAllocPA_PML4T | SystemCR3Flag;

	//fill the structure part:CR3System
	pPhysicalOpCR3->CR3System = SystemCR3;

	//print structure
	pPrintPhysicalOpStructure(pPhysicalOpCR3);

	g_IsPhysicalOpInit = TRUE;
	return STATUS_SUCCESS;
}


/*********************************************************
function:		FreePhysicalOpCR3
description:	to uninitialize the physical memory operation structure
calls:			pUnmapSystemPML4T
				pFreePhysicalOpPageTableMemory
				RtlZeroMemory
**********************************************************/
NTSTATUS FreePhysicalOpCR3(PPHYSICAL_OP_CR3 pPhysicalOpCR3)
{
	//check the init state
	if (!g_IsPhysicalOpInit)
		return STATUS_UNSUCCESSFUL;

	//unmap pSystemPML4T
	pUnmapSystemPML4T(pPhysicalOpCR3);

	//free allocated memory
	pFreePhysicalOpPageTableMemory(pPhysicalOpCR3);

	//clear generated cr3 and recorded system cr3
	pPhysicalOpCR3->CR3Generated = 0;
	pPhysicalOpCR3->CR3System = 0;

	//print structure
	pPrintPhysicalOpStructure(pPhysicalOpCR3);

	// set the structure to zero,avoid some bugs
	RtlZeroMemory((PVOID)pPhysicalOpCR3, sizeof(PHYSICAL_OP_CR3));


	g_IsPhysicalOpInit = FALSE;
	return STATUS_SUCCESS;
}

/*********************************************************
function:		ContextVirtualToPhysical
description:	raise irql and switch to generated cr3
**********************************************************/
NTSTATUS ContextVirtualToPhysical(PPHYSICAL_OP_CR3 pPhysicalOpCR3)
{
	//check the initialize state and current context
	if (!g_IsPhysicalOpInit || pPhysicalOpCR3->IsContextSwitched)
		return STATUS_UNSUCCESSFUL;

	//assert irql >= dispatch level
	pPhysicalOpCR3->OriginalIrql = KeGetCurrentIrql();
	if (pPhysicalOpCR3->OriginalIrql < DISPATCH_LEVEL)
	{
		pPhysicalOpCR3->IsIrqlChanged = TRUE;
		KeRaiseIrqlToDpcLevel();
	}


	//disable task switch interrupt(maskable)
	_disable();

	
	//record and switch cr3
	pPhysicalOpCR3->CR3BeforeSwitch = __readcr3();
	__writecr3(pPhysicalOpCR3->CR3Generated);

	//change the flag IsContextSwitched
	pPhysicalOpCR3->IsContextSwitched = TRUE;
	return STATUS_SUCCESS;
}

/*********************************************************
function:		ContextPhysicalToVirtual
description:	lower irql and switch to system cr3
**********************************************************/
NTSTATUS ContextPhysicalToVirtual(PPHYSICAL_OP_CR3 pPhysicalOpCR3)
{
	//check the initialize state and current context
	if (!g_IsPhysicalOpInit || !pPhysicalOpCR3->IsContextSwitched)
		return STATUS_UNSUCCESSFUL;

	//restore cr3
	__writecr3(pPhysicalOpCR3->CR3BeforeSwitch);

	//enable task switch interrupt(maskable)
	_enable();

	//restore irql
	if (pPhysicalOpCR3->IsIrqlChanged)
	{
		KeLowerIrql(pPhysicalOpCR3->OriginalIrql);

		//restore the flag IsIrqlChanged
		pPhysicalOpCR3->IsIrqlChanged = FALSE;
	}

	//change the flag IsContextSwitched
	pPhysicalOpCR3->IsContextSwitched = FALSE;
	return STATUS_SUCCESS;
}


/*********************************************************
function:		GetCR3ByEprocess
description:	get cr3 by eprocess
**********************************************************/
ULONG64 GetCR3ByEprocess(PEPROCESS pEProc)
{
	if (pEProc == NULL)
		return 0;

	//get dirbase
	ULONG64	DirBase = *(PULONG64)((ULONG64)pEProc + 0x028);
	return DirBase;
}

/*********************************************************
function:		GetEProcess
description:	get eprocess by pid
**********************************************************/
PEPROCESS GetEProcess(ULONG64 PID)
{
	PEPROCESS		pEProc = NULL;

	//check pid
	if (PID == 0)
		return 0;

	//get eprocess
	if (!NT_SUCCESS(PsLookupProcessByProcessId((HANDLE)PID, (PEPROCESS*)&pEProc)))
		return 0;

	//dereference
	if (pEProc != NULL)
		ObDereferenceObject((PVOID)pEProc);

	return pEProc;
}

/*********************************************************
function:		GetCR3ByPID
description:	get cr3 by pid
calls:			GetEProcess
				GetCR3ByEprocess
**********************************************************/
ULONG64 GetCR3ByPID(ULONG64 PID)
{
	return GetCR3ByEprocess(GetEProcess(PID));
}