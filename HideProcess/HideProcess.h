

#ifndef CXX_HIDEPROCESS_H
#define CXX_HIDEPROCESS_H


#include <ntifs.h>
typedef enum WIN_VERSION {
	WINDOWS_UNKNOW,
	WINDOWS_XP,
	WINDOWS_7,
	WINDOWS_8,
	WINDOWS_8_1
} WIN_VERSION;

VOID UnloadDriver(PDRIVER_OBJECT  DriverObject);
VOID HideProcess(char* ProcessName);




//»ñµÃPC °æ±¾
WIN_VERSION GetWindowsVersion();
PVOID 
GetFunctionAddressByName(WCHAR *wzFunction);
typedef 
NTSTATUS 
(*pfnRtlGetVersion)(OUT PRTL_OSVERSIONINFOW lpVersionInformation);
#endif
