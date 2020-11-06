#pragma once

#define CONVERT_RVA(base, offset) ((PVOID)((PUCHAR)(base) + (ULONG)(offset)))

typedef struct _RTL_MODULE_EXTENDED_INFO
{
	PVOID ImageBase;
	ULONG ImageSize;
	USHORT FileNameOffset;
	CHAR FullPathName[0x100];
} RTL_MODULE_EXTENDED_INFO, *PRTL_MODULE_EXTENDED_INFO;

PVOID MemLoadDriverByFilePath(PWCHAR wDriverPath, PWCHAR wDriverName);
