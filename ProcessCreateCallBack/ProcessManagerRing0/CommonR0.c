#include "CommonR0.h"

extern WIN_VERSION WinVersion;

extern ULONG_PTR ObjectHeaderSize;
extern ULONG_PTR ObjectTypeOffsetOf_Object_Header;

extern ULONG_PTR ObjectTableOffsetOf_EPROCESS;
WIN_VERSION GetWindowsVersion()
{
	RTL_OSVERSIONINFOEXW osverInfo = {sizeof(osverInfo)}; 
	pfnRtlGetVersion RtlGetVersion = NULL;
	WIN_VERSION WinVersion;
	WCHAR wzRtlGetVersion[] = L"RtlGetVersion";
	RtlGetVersion = GetFunctionAddressByName(wzRtlGetVersion);    //Ntoskrnl.exe  导出表
	if (RtlGetVersion)
	{
		RtlGetVersion((PRTL_OSVERSIONINFOW)&osverInfo); 
	} 
	else 
	{
		PsGetVersion(&osverInfo.dwMajorVersion, &osverInfo.dwMinorVersion, &osverInfo.dwBuildNumber, NULL);   //Documet
	}
	DbgPrint("Build Number: %d\r\n", osverInfo.dwBuildNumber);
	if (osverInfo.dwMajorVersion == 5 && osverInfo.dwMinorVersion == 1) 
	{
		DbgPrint("WINDOWS_XP\r\n");
		WinVersion = WINDOWS_XP;
	}
	else if (osverInfo.dwMajorVersion == 6 && osverInfo.dwMinorVersion == 1)
	{
		DbgPrint("WINDOWS 7\r\n");
		WinVersion = WINDOWS_7;
	}
	else if (osverInfo.dwMajorVersion == 6 && 
		osverInfo.dwMinorVersion == 2 &&
		osverInfo.dwBuildNumber == 9200)
	{
		DbgPrint("WINDOWS 8\r\n");
		WinVersion = WINDOWS_8;
	}
	else if (osverInfo.dwMajorVersion == 6 && 
		osverInfo.dwMinorVersion == 3 && 
		osverInfo.dwBuildNumber == 9600)
	{
		DbgPrint("WINDOWS 8.1\r\n");
		WinVersion = WINDOWS_8_1;
	}
	else
	{
		DbgPrint("WINDOWS_UNKNOW\r\n");
		WinVersion = WINDOWS_UNKNOW;
	}
	return WinVersion;
}
PVOID 
	GetFunctionAddressByName(WCHAR *wzFunction)
{
	UNICODE_STRING uniFunction;  
	PVOID AddrBase = NULL;
	if (wzFunction && wcslen(wzFunction) > 0)
	{
		RtlInitUnicodeString(&uniFunction, wzFunction);      //常量指针
		AddrBase = MmGetSystemRoutineAddress(&uniFunction);  //在System 进程  第一个模块  Ntosknrl.exe  ExportTable
	}
	return AddrBase;
}







ULONG_PTR KeGetObjectType(PVOID Object)
{
	ULONG_PTR ObjectType = NULL;
	pfnObGetObjectType        ObGetObjectType = NULL;    

	if (!MmIsAddressValid ||!Object||!MmIsAddressValid(Object))
	{
		return NULL;
	}

	if (WinVersion==WINDOWS_XP)
	{
		ULONG SizeOfObjectHeader = 0, ObjectTypeOffset = 0, ObjectTypeAddress = 0;

		ObjectTypeAddress = (ULONG_PTR)Object - ObjectHeaderSize + ObjectTypeOffsetOf_Object_Header;

		if (MmIsAddressValid((PVOID)ObjectTypeAddress))
		{ 
			ObjectType = *(ULONG_PTR*)ObjectTypeAddress;
		}
	}
	else 
	{
		//高版本使用函数

		ObGetObjectType = (pfnObGetObjectType)GetFunctionAddressByName(L"ObGetObjectType");


		if (ObGetObjectType)
		{
			ObjectType = ObGetObjectType(Object);
		}
	}

	return ObjectType;
}

BOOLEAN IsProcessDie(PEPROCESS EProcess)
{
	BOOLEAN bDie = FALSE;

	if (MmIsAddressValid &&
		EProcess && 
		MmIsAddressValid(EProcess) &&
		MmIsAddressValid((PVOID)((ULONG_PTR)EProcess + ObjectTableOffsetOf_EPROCESS)))
	{
		PVOID ObjectTable = *(PVOID*)((ULONG_PTR)EProcess + ObjectTableOffsetOf_EPROCESS );

		if (!ObjectTable||!MmIsAddressValid(ObjectTable) )
		{
			DbgPrint("Process is Die\r\n");
			bDie = TRUE;
		}
	}
	else
	{
		DbgPrint("Process is Die2\r\n");
		bDie = TRUE;
	}
	return bDie;
}

