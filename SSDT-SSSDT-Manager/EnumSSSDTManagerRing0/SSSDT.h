#pragma once
#include "SSSDTManager.h"
#include "common.h"

PVOID GetKeShadowServiceDescriptorTable32();
PVOID GetKeShadowServiceDescriptorTable64();


PVOID GetSSSDTFunctionAddress64(ULONG ulIndex);
PVOID GetSSSDTFunctionAddress32(ULONG ulIndex);

BOOLEAN GetSysModuleByLdrDataTable(WCHAR* wzModuleName);
BOOLEAN GetSysModuleByLdrDataTable1(PVOID Address,WCHAR* wzModuleName);



VOID  UnHookSSSDTWin7(ULONG ulIndex, ULONG OriginalFunctionAddress);
VOID  UnHookSSSDTWinXP(ULONG ulIndex, ULONG OriginalFunctionAddress);


BOOLEAN ResumeSSSDTInlineHook(ULONG ulIndex,UCHAR* szOriginalFunctionCode);