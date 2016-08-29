/*
x86和x64的注入因为x64的系统增加了较多的权限的校验，需要进行提权处理。
x64提权主要就是用到了ntdll.dll中的未导出函数，RtlAdjustPrivilege().
*/
#include "stdafx.h"
#include "CreateRemoteThread.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 唯一的应用程序对象

CWinApp theApp;

using namespace std;

typedef enum  _WIN_VERSION
{
	WindowsNT,
	Windows2000,
	WindowsXP,
	Windows2003,
	WindowsVista,
	Windows7,
	Windows8,
	WinUnknown
}WIN_VERSION;

VOID InjectDll(ULONG_PTR ProcessID);
WIN_VERSION  GetWindowsVersion();
BOOL InjectDllByRemoteThread32(const TCHAR* wzDllFile, ULONG_PTR ProcessId);
WIN_VERSION  WinVersion = WinUnknown;

BOOL InjectDllByRemoteThread64(const TCHAR* wzDllFile, ULONG_PTR ProcessId);
typedef long (__fastcall *pfnRtlAdjustPrivilege64)(ULONG,ULONG,ULONG,PVOID);
pfnRtlAdjustPrivilege64 RtlAdjustPrivilege;





int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
	cout<<"查看要注入进程的ID"<<endl;   
	ULONG_PTR ProcessID = 0;
	WinVersion = GetWindowsVersion();
	printf("Input ProcessID\r\n");
	cin>>ProcessID;
	InjectDll(ProcessID);
	return 0;
}

VOID InjectDll(ULONG_PTR ProcessID)
{
    CString strPath32 = L"Dllx86.dll";   //32位dll注入32位系统
    CString strPath64 = L"Dllx64.dll";
    if (ProcessID == 0)
    {
        return;
    }
    if (PathFileExists(strPath32)&&PathFileExists(strPath64))
    {
        switch(WinVersion)
        {
        case Windows7:   //这里用的是Win7 x64 sp1
            {

                WCHAR wzPath[MAX_PATH] = {0};
                GetCurrentDirectory(260,wzPath);
                wcsncat_s(wzPath, L"\\", 2);
                wcsncat_s(wzPath, strPath64.GetBuffer(), strPath64.GetLength());//dll完整路径
                strPath32.ReleaseBuffer();
                if (!InjectDllByRemoteThread64(wzPath,ProcessID))
                    printf("Inject Fail\r\n");
                else printf ("Inject Success\r\n");
                break;
            }

        case WindowsXP:  //WinXp x86 sp3
            {
                WCHAR wzPath[MAX_PATH] = {0};
                GetCurrentDirectory(260,wzPath);
                wcsncat_s(wzPath, L"\\", 2);
                wcsncat_s(wzPath, strPath32.GetBuffer(), strPath32.GetLength());

                strPath32.ReleaseBuffer();
                if (!InjectDllByRemoteThread32(wzPath,ProcessID))
                    printf("Inject Fail\r\n");            
                else printf("Inject Success\r\n");
                break;
            }
        }
    
    }    
}



BOOL InjectDllByRemoteThread64(const TCHAR* wzDllFile, ULONG_PTR ProcessId)
{
    if (NULL == wzDllFile || 0 == ::_tcslen(wzDllFile) || ProcessId == 0 || -1 == _taccess(wzDllFile, 0))
    {
        return FALSE;
    }
    HANDLE                 hProcess = NULL;
    HANDLE                 hThread  = NULL;
    DWORD                  dwRetVal    = 0;
    LPTHREAD_START_ROUTINE FuncAddress = NULL;
    DWORD  dwSize = 0;
    TCHAR* VirtualAddress = NULL;
    //预编译，支持Unicode
#ifdef _UNICODE
    FuncAddress = (PTHREAD_START_ROUTINE)::GetProcAddress(::GetModuleHandle(_T("Kernel32")), "LoadLibraryW");
#else
    FuncAddress = (PTHREAD_START_ROUTINE)::GetProcAddress(::GetModuleHandle(_T("Kernel32")), "LoadLibraryA");
#endif

    if (FuncAddress==NULL)
    {
        return FALSE;
    }

    RtlAdjustPrivilege=(pfnRtlAdjustPrivilege64)GetProcAddress((HMODULE)(FuncAddress(L"ntdll.dll")),"RtlAdjustPrivilege");

    if (RtlAdjustPrivilege==NULL)
    {
        return FALSE;
    }
        /*
        .常量 SE_BACKUP_PRIVILEGE, "17", 公开
        .常量 SE_RESTORE_PRIVILEGE, "18", 公开
        .常量 SE_SHUTDOWN_PRIVILEGE, "19", 公开
        .常量 SE_DEBUG_PRIVILEGE, "20", 公开
        */
    RtlAdjustPrivilege(20,1,0,&dwRetVal);  //19

    hProcess = OpenProcess(PROCESS_ALL_ACCESS,FALSE, ProcessId);

    if (NULL == hProcess)
    {
        printf("Open Process Fail\r\n");
        return FALSE;
    }

    // 在目标进程中分配内存空间
    dwSize = (DWORD)::_tcslen(wzDllFile) + 1;
    VirtualAddress = (TCHAR*)::VirtualAllocEx(hProcess, NULL, dwSize * sizeof(TCHAR), MEM_COMMIT, PAGE_READWRITE);  
    if (NULL == VirtualAddress)
    {
        printf("Virtual Process Memory Fail\r\n");
        CloseHandle(hProcess);
        return FALSE;
    }

    // 在目标进程的内存空间中写入所需参数(模块名)
    if (FALSE == ::WriteProcessMemory(hProcess, VirtualAddress, (LPVOID)wzDllFile, dwSize * sizeof(TCHAR), NULL))
    {
        printf("Write Data Fail\r\n");
        VirtualFreeEx(hProcess, VirtualAddress, dwSize, MEM_DECOMMIT);
        CloseHandle(hProcess);
        return FALSE;
    }

    hThread = ::CreateRemoteThread(hProcess, NULL, 0, FuncAddress, VirtualAddress, 0, NULL);
    if (NULL == hThread)
    {
        printf("CreateRemoteThread Fail\r\n");
        VirtualFreeEx(hProcess, VirtualAddress, dwSize, MEM_DECOMMIT);
        CloseHandle(hProcess);
        return FALSE;
    }
    // 等待远程线程结束
    WaitForSingleObject(hThread, INFINITE);
    // 清理资源
    VirtualFreeEx(hProcess, VirtualAddress, dwSize, MEM_DECOMMIT);
    CloseHandle(hThread);
    CloseHandle(hProcess);
    return TRUE;

}


BOOL InjectDllByRemoteThread32(const TCHAR* wzDllFile, ULONG_PTR ProcessId)
{
    // 参数无效
    if (NULL == wzDllFile || 0 == ::_tcslen(wzDllFile) || ProcessId == 0 || -1 == _taccess(wzDllFile, 0))
    {    
        return FALSE;
    }
    HANDLE hProcess = NULL;
    HANDLE hThread  = NULL;
    DWORD dwSize = 0;
    TCHAR* VirtualAddress = NULL;
    LPTHREAD_START_ROUTINE FuncAddress = NULL;
    // 获取目标进程句柄
    hProcess = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_VM_OPERATION | PROCESS_VM_WRITE, FALSE, ProcessId);
    if (NULL == hProcess)
    {
        printf("Open Process Fail\r\n");
        return FALSE;
    }
    // 在目标进程中分配内存空间
    dwSize = (DWORD)::_tcslen(wzDllFile) + 1;
    VirtualAddress = (TCHAR*)::VirtualAllocEx(hProcess, NULL, dwSize * sizeof(TCHAR), MEM_COMMIT, PAGE_READWRITE);
    if (NULL == VirtualAddress)
    {
        printf("Virtual Process Memory Fail\r\n");
        CloseHandle(hProcess);
        return FALSE;
    }
    // 在目标进程的内存空间中写入所需参数(模块名)
    if (FALSE == ::WriteProcessMemory(hProcess, VirtualAddress, (LPVOID)wzDllFile, dwSize * sizeof(TCHAR), NULL))
    {
        printf("Write Data Fail\r\n");
        VirtualFreeEx(hProcess, VirtualAddress, dwSize, MEM_DECOMMIT);
        CloseHandle(hProcess);
        return FALSE;
    }
    // 从 Kernel32.dll 中获取 LoadLibrary 函数地址
#ifdef _UNICODE
    FuncAddress = (PTHREAD_START_ROUTINE)::GetProcAddress(::GetModuleHandle(_T("Kernel32")), "LoadLibraryW");
#else
    FuncAddress = (PTHREAD_START_ROUTINE)::GetProcAddress(::GetModuleHandle(_T("Kernel32")), "LoadLibraryA");
#endif

    if (NULL == FuncAddress)
    {
        printf("Get LoadLibrary Fail\r\n");
        VirtualFreeEx(hProcess, VirtualAddress, dwSize, MEM_DECOMMIT);
        CloseHandle(hProcess);
        return false;
    }

    // 创建远程线程调用 LoadLibrary
    hThread = ::CreateRemoteThread(hProcess, NULL, 0, FuncAddress, VirtualAddress, 0, NULL);
    if (NULL == hThread)
    {
        printf("CreateRemoteThread Fail\r\n");
        VirtualFreeEx(hProcess, VirtualAddress, dwSize, MEM_DECOMMIT);
        CloseHandle(hProcess);
        return FALSE;
    }

    // 等待远程线程结束
    WaitForSingleObject(hThread, INFINITE);
    // 清理
    VirtualFreeEx(hProcess, VirtualAddress, dwSize, MEM_DECOMMIT);
    CloseHandle(hThread);
    CloseHandle(hProcess);

    return TRUE;
}

WIN_VERSION  GetWindowsVersion()
{
    OSVERSIONINFOEX    OsVerInfoEx;
    OsVerInfoEx.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    GetVersionEx((OSVERSIONINFO *)&OsVerInfoEx); // 注意转换类型
    switch (OsVerInfoEx.dwPlatformId)
    {
    case VER_PLATFORM_WIN32_NT:
        {
            if (OsVerInfoEx.dwMajorVersion <= 4 )
            {
                return WindowsNT;
            }
            if (OsVerInfoEx.dwMajorVersion == 5 && OsVerInfoEx.dwMinorVersion == 0)
            {
                return Windows2000;
            }

            if (OsVerInfoEx.dwMajorVersion == 5 && OsVerInfoEx.dwMinorVersion == 1)
            {
                return WindowsXP;
            }
            if (OsVerInfoEx.dwMajorVersion == 5 && OsVerInfoEx.dwMinorVersion == 2)
            {
                return Windows2003;
            }
            if (OsVerInfoEx.dwMajorVersion == 6 && OsVerInfoEx.dwMinorVersion == 0)
            {
                return WindowsVista;
            }

            if (OsVerInfoEx.dwMajorVersion == 6 && OsVerInfoEx.dwMinorVersion == 1)
            {
                return Windows7;
            }
            if (OsVerInfoEx.dwMajorVersion == 6 && OsVerInfoEx.dwMinorVersion == 2 )
            {
                return Windows8;
            }
            break;
        }

    default:
        {
            return WinUnknown;
        }
    }

}