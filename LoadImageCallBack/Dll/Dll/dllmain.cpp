// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "stdafx.h"



DWORD WINAPI ThreadProc(LPVOID lpParameter);
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		{

				MessageBox(NULL,L"注入成功",L"a",1);
				HANDLE hIhread = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)ThreadProc,NULL,0,NULL);
		}
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}



DWORD WINAPI ThreadProc(LPVOID lpParameter)
{
	return 0;
}             