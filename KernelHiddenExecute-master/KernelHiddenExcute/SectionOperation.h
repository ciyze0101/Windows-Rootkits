#pragma once
#include "Head.h"

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

PIMAGE_SECTION_HEADER GetSegmentHeadPointer(PDRIVER_OBJECT pDriverObj, PCHAR pSegName)
{
	PLDR_DATA_TABLE_ENTRY64		entry = (PLDR_DATA_TABLE_ENTRY64)pDriverObj->DriverSection;
	PUCHAR						pJumpDrvBase = (PUCHAR)entry->DllBase;
	PIMAGE_DOS_HEADER			pDosHead;
	PIMAGE_NT_HEADERS			pNtHead;
	PIMAGE_SECTION_HEADER		pSecHead;
	BOOL						bFinded = FALSE;

	pDosHead = (PIMAGE_DOS_HEADER)pJumpDrvBase;
	if (pDosHead->e_magic != IMAGE_DOS_SIGNATURE)
	{
		MyPrint(("[" PRINT_NAME "] DosHead Error\n"));
		return 0;
	}
	pNtHead = (PIMAGE_NT_HEADERS)\
		((LONG_PTR)pDosHead + pDosHead->e_lfanew);
	if (pNtHead->Signature != IMAGE_NT_SIGNATURE)
	{
		MyPrint(("[" PRINT_NAME "] NtHead Error\n"));
		return 0;
	}
	pSecHead = IMAGE_FIRST_SECTION(pNtHead);
	for (int i = 0; i < pNtHead->FileHeader.NumberOfSections; i++)
	{
		if (strcmp((const char*)(pSecHead->Name), pSegName) == 0)
		{
			bFinded = TRUE;
			break;
		}
		pSecHead++;
	}
	if (bFinded == FALSE)
	{
		MyPrint(("[" PRINT_NAME "] SecHead Error\n"));
		return 0;
	}

	return pSecHead;
}

ULONG64 GetDriverBaseAddress(PDRIVER_OBJECT pDriverObj, PCHAR pSegName)
{
	PLDR_DATA_TABLE_ENTRY64		entry = (PLDR_DATA_TABLE_ENTRY64)pDriverObj->DriverSection;
	PUCHAR						pJumpDrvBase = (PUCHAR)entry->DllBase;
	return (ULONG64)pJumpDrvBase;
}
ULONG64 GetSegmentAddressPointer(PDRIVER_OBJECT pDriverObj, PCHAR pSegName)
{
	PIMAGE_SECTION_HEADER pSecHead = GetSegmentHeadPointer(pDriverObj, pSegName);
	return (ULONG64)&(pSecHead->VirtualAddress);
}
ULONG64 GetSegmentLengthPointer(PDRIVER_OBJECT pDriverObj, PCHAR pSegName)
{
	PIMAGE_SECTION_HEADER pSecHead = GetSegmentHeadPointer(pDriverObj, pSegName);
	return (ULONG64)&(pSecHead->Misc.VirtualSize);
}
ULONG64 GetSegmentRawDataAddressPointer(PDRIVER_OBJECT pDriverObj, PCHAR pSegName)
{
	PIMAGE_SECTION_HEADER pSecHead = GetSegmentHeadPointer(pDriverObj, pSegName);
	return (ULONG64)&(pSecHead->PointerToRawData);
}
ULONG64 GetSegmentRawDataLengthPointer(PDRIVER_OBJECT pDriverObj, PCHAR pSegName)
{
	PIMAGE_SECTION_HEADER pSecHead = GetSegmentHeadPointer(pDriverObj, pSegName);
	return (ULONG64)&(pSecHead->SizeOfRawData);
}

ULONG64 GetSegmentStartAddress(PDRIVER_OBJECT pDriverObj, PCHAR pSegName)
{
	ULONG64 pDriverBase = GetDriverBaseAddress(pDriverObj, pSegName);
	ULONG64 pSegmentAddress = GetSegmentAddressPointer(pDriverObj, pSegName);
	return pDriverBase + *(PULONG32)pSegmentAddress;
}

ULONG64 GetSegmentEndAddress(PDRIVER_OBJECT pDriverObj, PCHAR pSegName)
{
	ULONG64 pDriverBase = GetDriverBaseAddress(pDriverObj, pSegName);
	ULONG64 pSegmentAddress = GetSegmentAddressPointer(pDriverObj, pSegName);
	ULONG64 pSegmentLength = GetSegmentLengthPointer(pDriverObj, pSegName);
	return pDriverBase + *(PULONG32)pSegmentAddress + *(PULONG32)pSegmentLength;
}

ULONG64 GetSegmentLength(PDRIVER_OBJECT pDriverObj, PCHAR pSegName)
{
	ULONG64 pDriverBase = GetDriverBaseAddress(pDriverObj, pSegName);
	ULONG64 pSegmentAddress = GetSegmentAddressPointer(pDriverObj, pSegName);
	ULONG64 pSegmentLength = GetSegmentLengthPointer(pDriverObj, pSegName);
	return *(PULONG32)pSegmentLength;
}