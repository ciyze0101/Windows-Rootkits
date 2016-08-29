#include "common.h"


extern WIN_VERSION WinVersion;
extern KIRQL Irql;
VOID WPOFF()
{
	ULONG_PTR cr0 = 0;
	Irql = KeRaiseIrqlToDpcLevel();
	cr0 =__readcr0();
	cr0 &= 0xfffffffffffeffff;
	__writecr0(cr0);
	//	_disable();                      //这句话 屏蔽也没有啥
}

VOID WPON()
{
	ULONG_PTR cr0=__readcr0();
	cr0 |= 0x10000;
	//	_enable();                      //这句话 屏蔽也没有啥
	__writecr0(cr0);
	KeLowerIrql(Irql);
}
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





BOOLEAN SafeCopyMemory(PVOID DestiAddress, PVOID SourAddress, SIZE_T SizeOfCopy)
{

	PMDL  Mdl = NULL;
	PVOID SafeAddress = NULL;
	if(!MmIsAddressValid(DestiAddress) ||
		!MmIsAddressValid(SourAddress) )
		return FALSE;

	Mdl = IoAllocateMdl(SourAddress, (ULONG)SizeOfCopy, FALSE, FALSE, NULL);
	if(!Mdl) 
	{
		return FALSE;
	}
	__try
	{
		MmProbeAndLockPages(Mdl, KernelMode, IoReadAccess );
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		IoFreeMdl(Mdl);
		return FALSE;
	}
	SafeAddress = MmGetSystemAddressForMdlSafe(Mdl,NormalPagePriority );
	//锁定内存页，方式被page out出去
	if(!SafeAddress)
	{
		return FALSE;
	}
	RtlCopyMemory(DestiAddress,SafeAddress,SizeOfCopy );
	MmUnlockPages(Mdl);
	IoFreeMdl(Mdl);
	return TRUE;

}