// InjectDllBySetThreadContextx64.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

#include <iostream>
using namespace std;
#include <windows.h>
#include "tlhelp32.h"
BYTE ShellCode[64]=
{
	0x60,
	0x9c,
	0x68,               //push
	0xaa,0xbb,0xcc,0xdd,//dll path  +3  dll最目标进程中的地址
	0xff,0x15,          //call     这里感觉有点乱，我在64下直接call 相对地址 
	0xdd,0xcc,0xbb,0xaa,//+9 LoadLibrary Addr  Addr
	0x9d,
	0x61,
	0xff,0x25,          //jmp
	0xaa,0xbb,0xcc,0xdd,// +17  jmp  eip
	0xaa,0xaa,0xaa,0xaa,// loadlibrary addr
	0xaa,0xaa,0xaa,0xaa//  jmpaddr  +25

	//  +29
}; 

/*
{
00973689 >    60                PUSHAD
0097368A      9C                PUSHFD
0097368B      68 50369700       PUSH notepad.00973650
00973690      FF15 70369700     CALL DWORD PTR DS:[973670]
00973696      9D                POPFD
00973697      61                POPAD
00973698    - FF25 30369700     JMP DWORD PTR DS:[973630]
}
*/

BYTE ShellCode64[64]=
{
	0x48,0x83,0xEC,0x28,  // sub rsp ,28h

	0x48,0x8D,0x0d,       // [+4] lea rcx,
	0xaa,0xbb,0xcc,0xdd,  // [+7] dll path offset =  TargetAddress- Current(0x48)[+4] -7 


	0xe8,                 // [+11]
	0xdd,0xcc,0xbb,0xaa,  // [+12] call LoadLibrary offset = TargetAddress - Current(0xe8)[+11] -5

	0x48,0x83,0xc4,0x28,  // [+16] add rsp,28h
	//0xcc, 调试时断下来的int 3 正常运行的时候非常傻逼的没有请掉...难怪一直死
	0xff,0x25,            // [+20]
	0xaa,0xbb,0xcc,0xdd,  // [+22] jmp rip offset  = TargetAddress - Current(0xff)[+20] - 6

	0xaa,0xbb,0xcc,0xdd,  //+26
	0xaa,0xbb,0xcc,0xdd   
	//+34
};
BOOL EnableDebugPriv() ;
BOOL StartHook(HANDLE hProcess,HANDLE hThread);

int _tmain(int argc, _TCHAR* argv[])
{

	EnableDebugPriv() ;
	int ProcessId = 0;
	cin>>ProcessId;

	HANDLE Process = OpenProcess(PROCESS_ALL_ACCESS,NULL,ProcessId);

	// 定义线程信息结构  
	THREADENTRY32 te32 = {sizeof(THREADENTRY32)} ;  
	//创建系统线程快照  ss
	HANDLE hThreadSnap = CreateToolhelp32Snapshot ( TH32CS_SNAPTHREAD, 0 ) ;  
	if ( hThreadSnap == INVALID_HANDLE_VALUE )  
		return FALSE ;  

	// 循环枚举线程信息  
	if ( Thread32First ( hThreadSnap, &te32 ) )  
	{  
		do{  

			if(te32.th32OwnerProcessID == ProcessId)
			{
				HANDLE Thread = OpenThread(THREAD_ALL_ACCESS,NULL,te32.th32ThreadID);

				SuspendThread(Thread);
				if (!StartHook(Process,Thread))
				{
					TerminateProcess(Process,0);
					printf("失败\n");
					getchar();
					return 0;
				}
				CloseHandle(Process);
				CloseHandle(Thread);


			}
			
		}while ( Thread32Next ( hThreadSnap, &te32 ) ) ;  
	}  

	CloseHandle ( hThreadSnap ) ;  
}





BYTE *DllPath;
BOOL StartHook(HANDLE hProcess,HANDLE hThread)
{

#ifdef _WIN64 

	CONTEXT ctx;
	ctx.ContextFlags=CONTEXT_ALL;
	if (!GetThreadContext(hThread,&ctx))
	{
		printf("GetThreadContext Error\n");
		return FALSE;
	}
	LPVOID LpAddr=VirtualAllocEx(hProcess,NULL,64,MEM_COMMIT,PAGE_EXECUTE_READWRITE);
	if (LpAddr==NULL)
	{
		printf("VirtualAlloc Error\n");
		return FALSE;
	}
	DWORD64 LoadDllAAddr=(DWORD64)GetProcAddress(GetModuleHandle(L"kernel32.dll"),"LoadLibraryA");
	if (LoadDllAAddr==NULL)
	{
		printf("LoadDllAddr error\n");
		return FALSE;
	}
	/*

	0x48,0x83,0xEC,0x28,  //sub rsp ,28h

	0x48,0x8D,0x0d,       // [+4] lea rcx,
	0xaa,0xbb,0xcc,0xdd,  // [+7] dll path offset =  TargetAddress- Current(0x48)[+4] -7 


	0xe8,                 // [+11]
	0xdd,0xcc,0xbb,0xaa,  // [+12] call LoadLibrary offset = TargetAddress - Current(0xe8)[+11] -5

	0x48,0x83,0xc4,0x28,  // [+16] add rsp,28h

	0xff,0x25,            // [+20]
	0xaa,0xbb,0xcc,0xdd,  // [+22] jmp rip offset  = TargetAddress - Current(0xff)[+20] - 6

	0xaa,0xbb,0xcc,0xdd,  //+26
	0xaa,0xbb,0xcc,0xdd   
	//+34
	*/
	DllPath=ShellCode64+34;
	strcpy((char*)DllPath,"D:\\Dll.dll");//这里是要注入的DLL名字
	DWORD DllNameOffset = 23;// ((BYTE*)LpAddr+34) -((BYTE*)LpAddr+4) -7 这个指令7个字节
	*(DWORD*)(ShellCode64+7)=(DWORD)DllNameOffset;
	////////////////
	DWORD LoadDllAddroffset = (BYTE*)LoadDllAAddr - ((BYTE*)LpAddr + 11) -5;  //这个指令5个字节e8 + 4addroffset
	*(DWORD*)(ShellCode64+12)=LoadDllAddroffset;
	//////////////////////////////////
	
	
	*(DWORD64*)(ShellCode64+26)=ctx.Rip; //64下为rip
	*(DWORD*)(ShellCode64+22)= (DWORD)0; //我将地址放在+26的地方，相对offset为0
	
//  这里因为这样写跳转不到目标地址，故x64 应该要中转一次  相对寻址
// 	DWORD Ds = (DWORD)ctx.SegDs;
// 	DWORD RipOffset = (BYTE*)ctx.Rip - ((BYTE*)LpAddr+20) -6;
// 	*(DWORD*)(ShellCode64+22)=(DWORD)ctx.Rip;

	////////////////////////////////////
	if (!WriteProcessMemory(hProcess,LpAddr,ShellCode64,64,NULL))
	{
		printf("write Process Error\n");
		return FALSE;
	}
	ctx.Rip=(DWORD64)LpAddr;
	if (!SetThreadContext(hThread,&ctx))
	{
		printf("set thread context error\n");
		return FALSE;
	}
	ResumeThread(hThread);
	return TRUE;
	
#else
	CONTEXT ctx = {0};
	ctx.ContextFlags=CONTEXT_ALL;
	if (!GetThreadContext(hThread,&ctx))
	{
		int a = GetLastError();
		printf("GetThreadContext Error\n");
		return FALSE;
	}
	LPVOID LpAddr=VirtualAllocEx(hProcess,NULL,64,MEM_COMMIT,PAGE_EXECUTE_READWRITE);
	if (LpAddr==NULL)
	{
		printf("VirtualAlloc Error\n");
		return FALSE;
	}
	DWORD LoadDllAAddr=(DWORD)GetProcAddress(GetModuleHandle(L"kernel32.dll"),"LoadLibraryA");
	if (LoadDllAAddr==NULL)
	{
		printf("LoadDllAddr error\n");
		return FALSE;
	}

	/////////////
	/*
	0x60,              PUSHAD
	0x9c,              PUSHFD
	0x68,              PUSH 
	0xaa,0xbb,0xcc,0xdd,//dll path  address  
	0xff,0x15,            CALL
	0xdd,0xcc,0xbb,0xaa,  offset  
	0x9d,                  POPFD
	0x61,                  POPAD
	0xff,0x25,             JMP 
	0xaa,0xbb,0xcc,0xdd,//  [xxxxx]
	0xaa,0xaa,0xaa,0xaa,// LoadLibrary Address
	0xaa,0xaa,0xaa,0xaa//  恢复的EIP  Address  
	                     // +29  Dll名字
	*/
	_asm mov esp,esp
	DllPath=ShellCode+29;
	strcpy((char*)DllPath,"D:\\Dllx86.dll");//这里是要注入的DLL名字
	*(DWORD*)(ShellCode+3)=(DWORD)LpAddr+29;
	////////////////
	*(DWORD*)(ShellCode+21)=LoadDllAAddr;   //loadlibrary地址放入shellcode中
	*(DWORD*)(ShellCode+9)=(DWORD)LpAddr+21;//修改call 之后的地址 为目标空间存放 loaddlladdr的地址
	//////////////////////////////////
	*(DWORD*)(ShellCode+25)=ctx.Eip;
	*(DWORD*)(ShellCode+17)=(DWORD)LpAddr+25;//修改jmp 之后为原来eip的地址
	////////////////////////////////////
	if (!WriteProcessMemory(hProcess,LpAddr,ShellCode,64,NULL))
	{
		printf("write Process Error\n");
		return FALSE;
	}
	ctx.Eip=(DWORD)LpAddr;
	if (!SetThreadContext(hThread,&ctx))
	{
		printf("set thread context error\n");
		return FALSE;
	}
	ResumeThread(hThread);
	return TRUE;
#endif
	
};




BOOL EnableDebugPriv() 
{
	HANDLE   hToken; 
	LUID   sedebugnameValue; 
	TOKEN_PRIVILEGES   tkp;
	if(!OpenProcessToken(GetCurrentProcess(),TOKEN_ADJUST_PRIVILEGES|TOKEN_QUERY,&hToken)) 
	{ 
		return   FALSE; 
	} 

	if(!LookupPrivilegeValue(NULL,SE_DEBUG_NAME,&sedebugnameValue)) 
	{ 
		CloseHandle(hToken); 
		return   FALSE; 
	} 
	tkp.PrivilegeCount   =   1; 
	tkp.Privileges[0].Luid   =   sedebugnameValue; 
	tkp.Privileges[0].Attributes   =   SE_PRIVILEGE_ENABLED; 

	if(!AdjustTokenPrivileges(hToken,FALSE,&tkp,sizeof(tkp),NULL,NULL)) 
	{ 
		return   FALSE; 
	}   
	CloseHandle(hToken); 
	return TRUE;

} 


