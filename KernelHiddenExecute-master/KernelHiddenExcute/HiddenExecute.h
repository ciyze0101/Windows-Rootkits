#pragma once
#include "Head.h"

/*********************************************************
description:
notice!!!	run in IRQL >= DPC_LEVEL
			call apis maybe due to lower irql

in order to hidden real code in the non-mapped physical pages,and clear original codes
before call the hidden functions,we should map the physical pages to correct position.
if we have to call windows api in our hidden functions,we should call the transfer functions(not hidden)

features of transfer functions:check & change irql,restore & rewrite page table(pte),call specific apis

reserve the physical pages by Api:MmMarkPhysicalMemoryAsBad,prevent the allocation of our physical pages space

**********************************************************/

#define SECTION_NAME_HIDDEN			".hidden"

#define HIDDEN_IRQL					DISPATCH_LEVEL
#define ChangeIrql(x)				WriteCR8(x)
#define ClearPageTableFlag(x)		ClearCR3Flag(x)
#define HIDDEN_PAGE_RECORD_LENGTH	0x1000
#define MAX_HIDDEN_PAGE_COUNT		126					//      (4096 - 40) / (4 * 8) = 126.75

typedef struct _MMPTE
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
		/* 0x0000 */ unsigned __int64 PageFrameNumber : 36; /* bit position: 12 */
		/* 0x0000 */ unsigned __int64 reserved1 : 4; /* bit position: 48 */
		/* 0x0000 */ unsigned __int64 SoftwareWsIndex : 11; /* bit position: 52 */
		/* 0x0000 */ unsigned __int64 NoExecute : 1; /* bit position: 63 */
	}; /* bitfield */
} MMPTE, *PMMPTE; /* size: 0x0008 */

typedef struct _SPECIFIC_HIDDEN_PAGE_RECORD
{
	PVOID		pHiddenBase;
	PMMPTE		pPTE;
	ULONG64		OriginalPfn;
	ULONG64		HiddenPfn;
}SPECIFIC_HIDDEN_PAGE_RECORD, *PSPECIFIC_HIDDEN_PAGE_RECORD;

typedef struct _HIDDEN_PAGE_RECORD
{
	BOOL							IsHidden;
	KIRQL							OriginalIrql;
	BOOL							IsIrqlChanged;
	KSPIN_LOCK						SpinLock;
	ULONG64							Count;
	SPECIFIC_HIDDEN_PAGE_RECORD		Record[MAX_HIDDEN_PAGE_COUNT - 1];
}HIDDEN_PAGE_RECORD, *PHIDDEN_PAGE_RECORD;



PHYSICAL_OP_CR3			g_PhysicalOpCR3 = { 0 };
BOOL					g_IsHiddenOpInit = FALSE;

NTSYSAPI NTSTATUS MmMarkPhysicalMemoryAsBad(IN PPHYSICAL_ADDRESS, IN OUT PLARGE_INTEGER);
NTSYSAPI NTSTATUS MmMarkPhysicalMemoryAsGood(IN PPHYSICAL_ADDRESS, IN OUT PLARGE_INTEGER);

/*********************************************************
function:		InitializeHiddenPageRecordStructure
description:	initialize the structure:initialize spin lock & set count to zero
calls:			ExAllocatePool
				KeInitializeSpinLock
				CreatePhysicalOpCR3BySystemCR3
**********************************************************/
NTSTATUS InitializeHiddenPageRecordStructure(PHIDDEN_PAGE_RECORD* ppHiddenPageRecord)
{
	//check the init state
	if (g_IsHiddenOpInit)
		return STATUS_UNSUCCESSFUL;

	PHIDDEN_PAGE_RECORD pHiddenPageRecord = NULL;
	pHiddenPageRecord = (PHIDDEN_PAGE_RECORD)ExAllocatePool(NonPagedPool, HIDDEN_PAGE_RECORD_LENGTH);
	*ppHiddenPageRecord = pHiddenPageRecord;

	MyPrint(_TitleAndFunc"pHiddenPageRecord:%16IX\n", pHiddenPageRecord);
	if (pHiddenPageRecord != NULL)
	{
		//set count to zero
		pHiddenPageRecord->Count = 0;
		//initialize spin lock
		KeInitializeSpinLock(&pHiddenPageRecord->SpinLock);

		//initialize physical memory context
		CreatePhysicalOpCR3BySystemCR3(GetCR3ByPID(4), &g_PhysicalOpCR3);

		//change init state
		g_IsHiddenOpInit = TRUE;

		return STATUS_SUCCESS;
	}
	else {
		return STATUS_UNSUCCESSFUL;
	}
}

/*********************************************************
function:		FreeHiddenPageRecordStructure
description:	free the structure and set pHiddenPageRecord to zero
calls:			ExFreePool
**********************************************************/
NTSTATUS FreeHiddenPageRecordStructure(PHIDDEN_PAGE_RECORD pHiddenPageRecord)
{
	//check the init state
	if (!g_IsHiddenOpInit)
		return STATUS_UNSUCCESSFUL;

	if (pHiddenPageRecord != NULL)
	{
		//free hidden operation structure
		ExFreePool(pHiddenPageRecord);
		pHiddenPageRecord = NULL;

		//free physical operation structure
		FreePhysicalOpCR3(&g_PhysicalOpCR3);

		//change the init state
		g_IsHiddenOpInit = FALSE;

		return STATUS_SUCCESS;
	}
	else {
		return STATUS_UNSUCCESSFUL;
	}
}

/*********************************************************
function:		pPTEPFNtoPhysicalAddress
description:	translate pfn number to physical address
**********************************************************/
PVOID pPTEPFNtoPhysicalAddress(ULONG64 PFN)
{
	return (PVOID)(PFN << 12);
}

/*********************************************************
function:		pPhysicalAddresstoPTEPFN
description:	translate physical address to pfn number
**********************************************************/
ULONG64 pPhysicalAddresstoPTEPFN(PVOID PhysicalAddressBase)
{
	return (ULONG64)PhysicalAddressBase >> 12;
}

/*********************************************************
function:		pGetSpecificAddresspPTEPhysical
description:	get the pointer of pPTEPhysical
calls:			ContextVirtualToPhysical
ClearPageTableFlag
ContextPhysicalToVirtual
**********************************************************/
PMMPTE pGetSpecificAddresspPTEPhysical(ULONG64 CR3, PVOID pPageBase)
{
	//analyse the params
	PMMVA		pAddressInfo = (PMMVA)&pPageBase;
	PVOID		pPML4T = (PVOID)ClearCR3Flag(CR3);
	PMMPTE		pPML4E = NULL;
	PVOID		pPDPT = NULL;
	PMMPTE		pPDPTE = NULL;
	PVOID		pPDT = NULL;
	PMMPTE		pPDE = NULL;
	PVOID		pPT = NULL;
	PMMPTE		pPTE = NULL;

	//switch to physical context
	ContextVirtualToPhysical(&g_PhysicalOpCR3);

	pPML4E = (PMMPTE)((ULONG64)pPML4T + pAddressInfo->PML4T*ENTRY_SIZE);
	if (!pPML4E->Valid)
		goto Lable_Error;
	pPDPT = (PVOID)ClearPageTableFlag(*(PULONG64)pPML4E);

	pPDPTE = (PMMPTE)((ULONG64)pPDPT + pAddressInfo->PDPT*ENTRY_SIZE);
	if (pPDPTE->LargePage || !pPDPTE->Valid)
		goto Lable_Error;
	pPDT = (PVOID)ClearPageTableFlag(*(PULONG64)pPDPTE);

	pPDE = (PMMPTE)((ULONG64)pPDT + pAddressInfo->PDT*ENTRY_SIZE);
	if (pPDE->LargePage || !pPDE->Valid)
		goto Lable_Error;
	pPT = (PVOID)ClearPageTableFlag(*(PULONG64)pPDE);

	pPTE = (PMMPTE)((ULONG64)pPT + pAddressInfo->PT*ENTRY_SIZE);
	if (!pPTE->Valid)
		goto Lable_Error;

	ContextPhysicalToVirtual(&g_PhysicalOpCR3);
	//look up the page table finished
	MyPrint(_TitleAndFunc"pPTE:%16IX\n", pPTE);
	return pPTE;

Lable_Error:
	ContextPhysicalToVirtual(&g_PhysicalOpCR3);
	return NULL;
}

/*********************************************************
function:		pGetSpecificAddressPhysicalForR3
description:	just for E language
**********************************************************/
#define PHYSICAL_OFFSET	0x8000000000
#define NUM_1G			0x40000000
#define NUM_2M			0x200000
PVOID pGetSpecificAddressPhysicalForR3(ULONG64 CR3, PVOID pVirtual)
{
	//analyse the params
	PMMVA		pAddressInfo = (PMMVA)&pVirtual;
	PVOID		pPML4T = (PVOID)ClearCR3Flag(CR3);
	PMMPTE		pPML4E = NULL;
	PVOID		pPDPT = NULL;
	PMMPTE		pPDPTE = NULL;
	PVOID		pPDT = NULL;
	PMMPTE		pPDE = NULL;
	PVOID		pPT = NULL;
	PMMPTE		pPTE = NULL;
	PVOID		pPhysicalBase = NULL;
	PVOID		pPhysical = NULL;
	PVOID		pPhysicalR3 = NULL;

	pPML4E = (PMMPTE)((ULONG64)pPML4T + pAddressInfo->PML4T*ENTRY_SIZE + PHYSICAL_OFFSET);
	if (!pPML4E->Valid)
		return NULL;
	pPDPT = (PVOID)ClearPageTableFlag(*(PULONG64)pPML4E);

	pPDPTE = (PMMPTE)((ULONG64)pPDPT + pAddressInfo->PDPT*ENTRY_SIZE + PHYSICAL_OFFSET);
	if (!pPDPTE->Valid)
		return NULL;
	if (pPDPTE->LargePage)
		goto Lable_PDPTE_LargePage;
	pPDT = (PVOID)ClearPageTableFlag(*(PULONG64)pPDPTE);

	pPDE = (PMMPTE)((ULONG64)pPDT + pAddressInfo->PDT*ENTRY_SIZE + PHYSICAL_OFFSET);
	if (!pPDE->Valid)
		return NULL;
	if (pPDE->LargePage)
		goto Lable_PDE_LargePage;
	pPT = (PVOID)ClearPageTableFlag(*(PULONG64)pPDE);

	pPTE = (PMMPTE)((ULONG64)pPT + pAddressInfo->PT*ENTRY_SIZE + PHYSICAL_OFFSET);
	if (!pPTE->Valid)
		return NULL;
	pPhysicalBase = (PVOID)ClearPageTableFlag(*(PULONG64)pPTE);

	pPhysical = (PVOID)((ULONG64)pPhysicalBase + pAddressInfo->Offset);
	pPhysicalR3 = (PVOID)((ULONG64)pPhysical + PHYSICAL_OFFSET);
	return pPhysicalR3;

Lable_PDPTE_LargePage:
	pPhysicalBase = (PVOID)(((PMMPDPTE)pPDPTE)->PageFrameNumber*NUM_1G);
	pPhysical = (PVOID)((ULONG64)pPhysicalBase + ((PMMVA_PDPTE_LARGE)pAddressInfo)->Offset);
	pPhysicalR3 = (PVOID)((ULONG64)pPhysical + PHYSICAL_OFFSET);
	return pPhysicalR3;

Lable_PDE_LargePage:
	pPhysicalBase = (PVOID)(((PMMPDE)pPDE)->PageFrameNumber*NUM_2M);
	pPhysical = (PVOID)((ULONG64)pPhysicalBase + ((PMMVA_PDE_LARGE)pAddressInfo)->Offset);
	pPhysicalR3 = (PVOID)((ULONG64)pPhysical + PHYSICAL_OFFSET);
	return pPhysicalR3;
}
/*********************************************************
function:		AddHiddenPageRecord
description:	add a record element to the structure
				include get pte physical address of the specific page and record the pfn
				reserve a physical page by api:MmAllocateNonCachedMemory and MmMarkPhysicalMemoryAsBad
				then free the page

calls:			KeAcquireSpinLock
				pGetSpecificAddresspPTEPhysical
				ContextVirtualToPhysical
				ContextPhysicalToVirtual
				MmAllocateNonCachedMemory
				MmGetPhysicalAddress
				pPhysicalAddresstoPTEPFN
				MmFreeNonCachedMemory
				MmMarkPhysicalMemoryAsBad
				KeReleaseSpinLock
**********************************************************/
NTSTATUS AddHiddenPageRecord(ULONG64 CR3, PVOID pHiddenPageBase, PHIDDEN_PAGE_RECORD pHiddenPageRecord)
{
	KIRQL		EntryIrql;
	NTSTATUS	Status = STATUS_UNSUCCESSFUL;
	//prevent mulit-thread change the page record count
	KeAcquireSpinLock(&pHiddenPageRecord->SpinLock, &EntryIrql);

	//check count
	MyPrint(_TitleAndFunc"pHiddenPageRecord->Count:%16IX\n", pHiddenPageRecord->Count);
	if (pHiddenPageRecord->Count == MAX_HIDDEN_PAGE_COUNT)
		goto Lable_Error;

	//add pPTE record
	PSPECIFIC_HIDDEN_PAGE_RECORD	pCurrentRecord = &pHiddenPageRecord->Record[pHiddenPageRecord->Count];

	pCurrentRecord->pPTE = pGetSpecificAddresspPTEPhysical(CR3, pHiddenPageBase);
	MyPrint(_TitleAndFunc"pCurrentRecord->pPTE:%16IX\n", pCurrentRecord->pPTE);
	if (pCurrentRecord->pPTE == NULL)
		goto Lable_Error;
	
	//add hidden virtual address record
	pCurrentRecord->pHiddenBase = pHiddenPageBase;
	MyPrint(_TitleAndFunc"pCurrentRecord->pHiddenBase:%16IX\n", pCurrentRecord->pHiddenBase);

	//add original pfn record
	ContextVirtualToPhysical(&g_PhysicalOpCR3);
	pCurrentRecord->OriginalPfn = pCurrentRecord->pPTE->PageFrameNumber;
	ContextPhysicalToVirtual(&g_PhysicalOpCR3);

	MyPrint(_TitleAndFunc"pCurrentRecord->OriginalPfn:%16IX\n", pCurrentRecord->OriginalPfn);

	//add hidden pfn record 
	//allocate memory
	//record the physical address
	//then free the memory and mark it as bad
	PVOID	TemporaryVirtual = MmAllocateNonCachedMemory(PAGE_SIZE);
	if (TemporaryVirtual == NULL)
		goto Lable_Error;

	PHYSICAL_ADDRESS	TemporaryPhysical = MmGetPhysicalAddress(TemporaryVirtual);
	LARGE_INTEGER		PhysicalLength = { 0 };
	PhysicalLength.QuadPart = PAGE_SIZE;

	pCurrentRecord->HiddenPfn = pPhysicalAddresstoPTEPFN((PVOID)(TemporaryPhysical.QuadPart));
	MyPrint(_TitleAndFunc"pCurrentRecord->HiddenPfn:%16IX\n", pCurrentRecord->HiddenPfn);

	MmFreeNonCachedMemory(TemporaryVirtual, PAGE_SIZE);
	
	Status = MmMarkPhysicalMemoryAsBad(&TemporaryPhysical, &PhysicalLength);

	//copy codes to the new non-mapped physical address
	ContextVirtualToPhysical(&g_PhysicalOpCR3);
	RtlCopyMemory((PVOID)(TemporaryPhysical.QuadPart),
		pCurrentRecord->pHiddenBase,
		PAGE_SIZE
	);
	ContextPhysicalToVirtual(&g_PhysicalOpCR3);

	//check the mark state
	if (!NT_SUCCESS(Status))
		goto Lable_Error;

	//the last step:count +1
	pHiddenPageRecord->Count++;

	//release spin lock
	KeReleaseSpinLock(&pHiddenPageRecord->SpinLock, EntryIrql);
	return STATUS_SUCCESS;

Lable_Error:
	KeReleaseSpinLock(&pHiddenPageRecord->SpinLock, EntryIrql);
	return STATUS_UNSUCCESSFUL;
}

/*********************************************************
function:		RemoveAndRestoreAllHiddenPageRecord
description:	remove and restore all the records of the structure
				include mark hidden physical memory as good,
				change mapping relations,
				and invalid the specific TLB

calls:			KeAcquireSpinLock
				pPTEPFNtoPhysicalAddress
				MmMarkPhysicalMemoryAsGood
				ContextVirtualToPhysical
				ContextPhysicalToVirtual
				__invlpg
				KeReleaseSpinLock
**********************************************************/
NTSTATUS RemoveAndRestoreAllHiddenPageRecord(PHIDDEN_PAGE_RECORD pHiddenPageRecord)
{
	KIRQL		EntryIrql;
	NTSTATUS	Status = STATUS_UNSUCCESSFUL;
	//prevent mulit-thread change the page record count
	KeAcquireSpinLock(&pHiddenPageRecord->SpinLock, &EntryIrql);

	//assert we have elements
	if (pHiddenPageRecord->Count == 0)
		goto Lable_Error;

	//restore all records and mark all the hidden physical memory as good
	PSPECIFIC_HIDDEN_PAGE_RECORD	pCurrentRecord = NULL;
	PHYSICAL_ADDRESS				CurrentHiddenPhysical = { 0 };
	LARGE_INTEGER					PhysicalLength = { 0 };

	PhysicalLength.QuadPart = PAGE_SIZE;

	for (int i = 0; i < pHiddenPageRecord->Count; i++)
	{
		pCurrentRecord = &pHiddenPageRecord->Record[i];
		CurrentHiddenPhysical.QuadPart = (ULONG64)pPTEPFNtoPhysicalAddress(pCurrentRecord->HiddenPfn);

		//mark it as good
		MmMarkPhysicalMemoryAsGood(&CurrentHiddenPhysical, &PhysicalLength);

		//restore all page mapping relations
		ContextVirtualToPhysical(&g_PhysicalOpCR3);
		pCurrentRecord->pPTE->PageFrameNumber = pCurrentRecord->OriginalPfn;
		ContextPhysicalToVirtual(&g_PhysicalOpCR3);

		//invalid the TLB of current hidden address
		__invlpg(pCurrentRecord->pHiddenBase);
	}

	//set count to zero
	pHiddenPageRecord->Count = 0;

	//release spin lock
	KeReleaseSpinLock(&pHiddenPageRecord->SpinLock, EntryIrql);
	return STATUS_SUCCESS;

Lable_Error:
	KeReleaseSpinLock(&pHiddenPageRecord->SpinLock, EntryIrql);
	return STATUS_UNSUCCESSFUL;
}

/*********************************************************
function:		ContextOriginalToHidden
description:	switch to hidden code,so that we can call our hidden functions

calls:			KeGetCurrentIrql
				KeRaiseIrqlToDpcLevel
				_disable
				KeAcquireSpinLock
				ContextVirtualToPhysical
				ContextPhysicalToVirtual
				__invlpg
				KeReleaseSpinLock
**********************************************************/
NTSTATUS ContextOriginalToHidden(PHIDDEN_PAGE_RECORD pHiddenPageRecord)
{
	KIRQL		EntryIrql;
	NTSTATUS	Status = STATUS_UNSUCCESSFUL;

	//record original irql
	pHiddenPageRecord->OriginalIrql = KeGetCurrentIrql();
	MyPrint(_TitleAndFunc"pHiddenPageRecord->OriginalIrql:%16IX\n", pHiddenPageRecord->OriginalIrql);

	//assert irql >= dispatch level
	if (pHiddenPageRecord->OriginalIrql < DISPATCH_LEVEL)
	{
		pHiddenPageRecord->IsIrqlChanged = TRUE;
		KeRaiseIrqlToDpcLevel();
	}

	//disable task switch interrupt(maskable)
	_disable();

	//prevent mulit-thread change the page record count
	KeAcquireSpinLock(&pHiddenPageRecord->SpinLock, &EntryIrql);

	//assert we have elements
	if (pHiddenPageRecord->Count == 0)
		goto Lable_Error;

	//check the initialize state and current hidden state
	if (!g_IsHiddenOpInit || pHiddenPageRecord->IsHidden)
		goto Lable_Error;

	//restore all records and mark all the hidden physical memory as good
	PSPECIFIC_HIDDEN_PAGE_RECORD	pCurrentRecord = NULL;

	for (int i = 0; i < pHiddenPageRecord->Count; i++)
	{
		pCurrentRecord = &pHiddenPageRecord->Record[i];

		//change all page mapping relations
		ContextVirtualToPhysical(&g_PhysicalOpCR3);
		pCurrentRecord->pPTE->PageFrameNumber = pCurrentRecord->HiddenPfn;
		ContextPhysicalToVirtual(&g_PhysicalOpCR3);

		//invalid the TLB of current hidden address
		__invlpg(pCurrentRecord->pHiddenBase);
	}

	KeReleaseSpinLock(&pHiddenPageRecord->SpinLock, EntryIrql);

	//change the flag IsHidden
	pHiddenPageRecord->IsHidden = TRUE;
	return STATUS_SUCCESS;

Lable_Error:
	KeReleaseSpinLock(&pHiddenPageRecord->SpinLock, EntryIrql);
	return STATUS_UNSUCCESSFUL;
}

/*********************************************************
function:		ContextOriginalToHidden
description:	switch to hidden code,so that we can call our hidden functions

calls:			KeAcquireSpinLock
				ContextVirtualToPhysical
				ContextPhysicalToVirtual
				__invlpg
				KeReleaseSpinLock
				_enable
				KeLowerIrql
**********************************************************/
NTSTATUS ContextHiddenToOriginal(PHIDDEN_PAGE_RECORD pHiddenPageRecord)
{
	KIRQL		EntryIrql;
	NTSTATUS	Status = STATUS_UNSUCCESSFUL;

	//prevent mulit-thread change the page record count
	KeAcquireSpinLock(&pHiddenPageRecord->SpinLock, &EntryIrql);

	//assert we have elements
	if (pHiddenPageRecord->Count == 0)
		goto Lable_Error;

	//check the initialize state and current hidden state
	if (!g_IsHiddenOpInit || !pHiddenPageRecord->IsHidden)
		goto Lable_Error;

	//restore all records and mark all the hidden physical memory as good
	PSPECIFIC_HIDDEN_PAGE_RECORD	pCurrentRecord = NULL;

	for (int i = 0; i < pHiddenPageRecord->Count; i++)
	{
		pCurrentRecord = &pHiddenPageRecord->Record[i];

		//change all page mapping relations
		ContextVirtualToPhysical(&g_PhysicalOpCR3);
		pCurrentRecord->pPTE->PageFrameNumber = pCurrentRecord->OriginalPfn;
		ContextPhysicalToVirtual(&g_PhysicalOpCR3);

		//invalid the TLB of current hidden address
		__invlpg(pCurrentRecord->pHiddenBase);
	}

	KeReleaseSpinLock(&pHiddenPageRecord->SpinLock, EntryIrql);

	//enable task switch interrupt(maskable)
	_enable();

	//restore irql
	MyPrint(_TitleAndFunc"pHiddenPageRecord->IsIrqlChanged:%16IX\n", pHiddenPageRecord->IsIrqlChanged);
	if (pHiddenPageRecord->IsIrqlChanged)
	{
		KeLowerIrql(pHiddenPageRecord->OriginalIrql);

		//restore the flag IsIrqlChanged
		pHiddenPageRecord->IsIrqlChanged = FALSE;
	}

	//change the flag IsHidden
	pHiddenPageRecord->IsHidden = FALSE;
	return STATUS_SUCCESS;

Lable_Error:
	KeReleaseSpinLock(&pHiddenPageRecord->SpinLock, EntryIrql);
	return STATUS_UNSUCCESSFUL;
}

/*********************************************************
function:		GetPagesCountByLength
description:	get pages count by length	
**********************************************************/
ULONG64 GetPagesCountByLength(ULONG64 Length)
{
	if ((Length & 0xFFF) == 0)
		return (Length >> 12);
	else
		return (Length >> 12) + 1;
}

/*********************************************************
function:		AddHiddenSection
description:	add hidden address by the offered section name
				if all the calls of AddHiddenPageRecord are successful,return STATUS_SUCCESS
calls:			GetSegmentStartAddress
				GetSegmentLength
				GetPagesCountByLength
				AddHiddenPageRecord
**********************************************************/
NTSTATUS AddHiddenSection(ULONG64 SystemCR3, PDRIVER_OBJECT pDriverObj, PCHAR pSegName, PHIDDEN_PAGE_RECORD pHiddenPageRecord)
{
	//analyse number of pages of the section 
	PVOID		pSectionStart = (PVOID)GetSegmentStartAddress(pDriverObj, pSegName);
	ULONG64		SectionLength = GetSegmentLength(pDriverObj, pSegName);
	ULONG64		PagesCount = GetPagesCountByLength(SectionLength);
	PVOID		pCurrentPage = NULL;
	NTSTATUS	Status = STATUS_UNSUCCESSFUL;

	MyPrint(_TitleAndFunc"pSectionStart:%16IX\n", pSectionStart);
	MyPrint(_TitleAndFunc"SectionLength:%16IX\n", SectionLength);
	MyPrint(_TitleAndFunc"PagesCount:%16IX\n", PagesCount);

	//call AddHiddenPageRecord to record the hidden info
	for (int i = 0; i < PagesCount; i++)
	{
		pCurrentPage = (PVOID)((ULONG64)pSectionStart + i*PAGE_SIZE);
		Status = AddHiddenPageRecord(SystemCR3, pCurrentPage, pHiddenPageRecord);

		//make sure all the records are successful
		if (!NT_SUCCESS(Status))
			return Status;
	}

	return STATUS_SUCCESS;
}


