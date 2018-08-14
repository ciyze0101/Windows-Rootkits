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
#ifdef _WIN64
    _disable();
#endif
}

VOID WPON()
{
	ULONG_PTR cr0=__readcr0();
	cr0 |= 0x10000;
#ifdef _WIN64
    _enable();
#endif
	__writecr0(cr0);
	KeLowerIrql(Irql);
}

WIN_VERSION GetWindowsVersion()
{
    RTL_OSVERSIONINFOEXW osverInfo = {sizeof(osverInfo)}; 
    pfnRtlGetVersion RtlGetVersion = NULL;
    WIN_VERSION WinVersion;
    WCHAR szRtlGetVersion[] = L"RtlGetVersion";

    RtlGetVersion = (pfnRtlGetVersion)GetFunctionAddressByName(szRtlGetVersion); 

    if (RtlGetVersion)
    {
        RtlGetVersion((PRTL_OSVERSIONINFOW)&osverInfo); 
    } 
    else 
    {
        PsGetVersion(&osverInfo.dwMajorVersion, &osverInfo.dwMinorVersion, &osverInfo.dwBuildNumber, NULL);
    }

    //x64位支持
    if(osverInfo.dwMajorVersion == 6 && osverInfo.dwMinorVersion == 1 && osverInfo.dwBuildNumber == 7600)
    {
        DbgPrint("WINDOWS 7\r\n");
        WinVersion = WINDOWS_7_7600;
    }
    else if(osverInfo.dwMajorVersion == 6 && osverInfo.dwMinorVersion == 1 && osverInfo.dwBuildNumber == 7601)
    {
        DbgPrint("WINDOWS 7\r\n");
        WinVersion = WINDOWS_7_7601;
    }
    else if(osverInfo.dwMajorVersion == 6 && osverInfo.dwMinorVersion == 2 && osverInfo.dwBuildNumber == 9200)
    {
        DbgPrint("WINDOWS 8\r\n");
        WinVersion = WINDOWS_8_9200;
    }
    else if(osverInfo.dwMajorVersion == 6 && osverInfo.dwMinorVersion == 3 && osverInfo.dwBuildNumber == 9600)
    {
        DbgPrint("WINDOWS 8.1\r\n");
        WinVersion = WINDOWS_8_9600;
    }
    else if(osverInfo.dwMajorVersion == 10 && osverInfo.dwMinorVersion == 0 && osverInfo.dwBuildNumber == 10240)
    {
        DbgPrint("WINDOWS 10 10240\r\n");
        WinVersion = WINDOWS_10_10240;
    }
    else if(osverInfo.dwMajorVersion == 10 && osverInfo.dwMinorVersion == 0 && osverInfo.dwBuildNumber == 10586)
    {
        DbgPrint("WINDOWS 10 10586\r\n");
        WinVersion = WINDOWS_10_10586;
    }
    else if(osverInfo.dwMajorVersion == 10 && osverInfo.dwMinorVersion == 0 && osverInfo.dwBuildNumber == 14393)
    {
        DbgPrint("WINDOWS 10 14393\r\n");
        WinVersion = WINDOWS_10_14393;
    }
    else if(osverInfo.dwMajorVersion == 10 && osverInfo.dwMinorVersion == 0 && osverInfo.dwBuildNumber == 15063)
    {
        DbgPrint("WINDOWS 10 15063\r\n");
        WinVersion = WINDOWS_10_15063;
    }
    else if(osverInfo.dwMajorVersion == 10 && osverInfo.dwMinorVersion == 0 && osverInfo.dwBuildNumber == 16299)
    {
        DbgPrint("WINDOWS 10 16299\r\n");
        WinVersion = WINDOWS_10_16299;
    }
    else if(osverInfo.dwMajorVersion == 10 && osverInfo.dwMinorVersion == 0 && osverInfo.dwBuildNumber == 17134)
    {
        DbgPrint("WINDOWS 10 17134\r\n");
        WinVersion = WINDOWS_10_17134;
    }
    else
    {
        DbgPrint("This is a new os\r\n");
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