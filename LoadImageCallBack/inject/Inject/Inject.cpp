// Inject.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "Inject.h"
#include <Windows.h>
#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 唯一的应用程序对象

CWinApp theApp;

using namespace std;

BOOL EnableDebugPrivilege();

VOID InjectDll(ULONG_PTR ProcessID);
BOOL InjectDllByRemoteThread(const TCHAR* wzDllFile, ULONG_PTR ProcessId);
int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
    int nRetCode = 0;
    ULONG_PTR ID = GetCurrentProcessId();
    cout<<"当前ID为:"<<ID<<endl;
    ULONG_PTR ProcessID = 0;

    EnableDebugPrivilege();

    printf("Input Inject ProcessID\r\n");

    cin>>ProcessID;

    InjectDll(ProcessID);

    getchar();
    getchar();

    return nRetCode;
}


VOID InjectDll(ULONG_PTR ProcessID)
{
    CString strPath;
#ifdef  _WIN64
    strPath =  L"Dll.dll";
#else
    strPath = L"Dll.dll";
#endif
    if (ProcessID == 0)
    {
        return;
    }
    if (PathFileExists(strPath))
    {
        WCHAR wzPath[MAX_PATH] = {0};
        GetCurrentDirectory(260,wzPath);
        wcsncat_s(wzPath, L"\\", 2);
        wcsncat_s(wzPath, strPath.GetBuffer(), strPath.GetLength());
        strPath.ReleaseBuffer();

        if (!InjectDllByRemoteThread(wzPath,ProcessID))    //远程线程进行Inject
        {
            printf("Inject Fail\r\n");
        }
        else
        {
            printf("Inject Success\r\n");
        }
    }    
}


BOOL InjectDllByRemoteThread(const TCHAR* wzDllFile, ULONG_PTR ProcessId)
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

    //获得目标进程句柄
    hProcess = OpenProcess(PROCESS_ALL_ACCESS,FALSE, ProcessId);

    if (NULL == hProcess)
    {
        printf("Open Process Fail LastError [%d]\r\n", GetLastError());
        return FALSE;
    }

    // 在目标进程中分配内存空间
    dwSize = (DWORD)::_tcslen(wzDllFile) + 1;
    VirtualAddress = (TCHAR*)::VirtualAllocEx(hProcess, NULL, dwSize * sizeof(TCHAR), 
        MEM_COMMIT,PAGE_READWRITE);
    if (NULL == VirtualAddress)
    {

        printf("Virtual Process Memory Fail LastError [%d]\r\n", GetLastError());
        CloseHandle(hProcess);
        return FALSE;
    }

    // 在目标进程的内存空间中写入所需参数(模块名)
    if (FALSE == ::WriteProcessMemory(hProcess, VirtualAddress, (LPVOID)wzDllFile, dwSize * sizeof(TCHAR), NULL))
    {
        printf("Write Data Fail lastError [%d]\r\n", GetLastError());
        VirtualFreeEx(hProcess, VirtualAddress, dwSize, MEM_DECOMMIT);
        CloseHandle(hProcess);
        return FALSE;
    }

#ifdef _UNICODE
    FuncAddress = (PTHREAD_START_ROUTINE)::GetProcAddress(::GetModuleHandle(_T("Kernel32")), "LoadLibraryW");
#else
    FuncAddress = (PTHREAD_START_ROUTINE)::GetProcAddress(::GetModuleHandle(_T("Kernel32")), "LoadLibraryA");
#endif

    //让目标进程开启一个线程  并 让他执行LoadLibrary(Address)   Address 一定是对方的内存地址  
    hThread = ::CreateRemoteThread(hProcess, NULL, 0, FuncAddress, VirtualAddress, 0, NULL);

    //Loadlirbrar(LPPARAMDATA);
    if (NULL == hThread)
    {
        printf("CreateRemoteThread Fail LastError [%d]\r\n", GetLastError());

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




BOOL EnableDebugPrivilege()
{
    HANDLE hToken;   
    TOKEN_PRIVILEGES TokenPrivilege;
    LUID uID;

    if (!OpenProcessToken(GetCurrentProcess(),TOKEN_ADJUST_PRIVILEGES|TOKEN_QUERY,&hToken))
    {
        printf("OpenProcessToken is Error\n");

        return FALSE;
    }

    if (!LookupPrivilegeValue(NULL,SE_DEBUG_NAME,&uID))
    {
        printf("LookupPrivilegeValue is Error\n");

        return FALSE;
    }

    TokenPrivilege.PrivilegeCount = 1;
    TokenPrivilege.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    TokenPrivilege.Privileges[0].Luid = uID;

    //在这里我们进行调整权限
    if (!AdjustTokenPrivileges(hToken,false,&TokenPrivilege,sizeof(TOKEN_PRIVILEGES),NULL,NULL))
    {
        printf("AdjuestTokenPrivileges is Error\n");
        return  FALSE;
    }
    return TRUE;
}


