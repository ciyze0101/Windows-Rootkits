#include "HookKiFastCallEntry.h"
#include "Common.h"


ULONG dword_4C1B8 = 0;
extern WCHAR FirstModulePath[260];
extern ULONG_PTR FirstModuleBase;
extern ULONG_PTR FirstModuleSize;
extern PKSERVICE_TABLE_DESCRIPTOR KeServiceDescriptorTable;


extern WIN_VERSION WinVersion;

ULONG_PTR KiFastCallEntryRetAddress;
UNICODE_STRING StringTest;

ULONG_PTR FastCallEntryExitAddress = 0;
ULONG_PTR FirstModuleMappingBase;

ULONG g_nPid = 0xFFFFFFFF;
ULONG g_nVad = 0;

PKSERVICE_TABLE_DESCRIPTOR KeServiceDescriptorShadowTable;


PSYSCALL_FILTER_INFO_TABLE pSysCallFilterInfo = NULL; //过滤器信息结构


NTSTATUS _KisInstallKiFastCallEntryHook()
{
	PUCHAR RetCode;
	NTSTATUS Status;
	//获得KeServiceDescriptor的地址
#ifdef _WIN64
	KeServiceDescriptorShadowTable = (PKSERVICE_TABLE_DESCRIPTOR)GetKeShadowServiceDescriptorTable64();
#else
	KeServiceDescriptorShadowTable = (PKSERVICE_TABLE_DESCRIPTOR)GetKeShadowServiceDescriptorTable32();
#endif

	pSysCallFilterInfo=(PSYSCALL_FILTER_INFO_TABLE)ExAllocatePoolWithTag(
		NonPagedPool,
		sizeof(SYSCALL_FILTER_INFO_TABLE),
		'WJSD');


	if ( pSysCallFilterInfo )
	{
		RtlZeroMemory(pSysCallFilterInfo,sizeof(SYSCALL_FILTER_INFO_TABLE));
		pSysCallFilterInfo->ulSSDTAddr = (ULONG)KeServiceDescriptorTable->Base;
		pSysCallFilterInfo->ulSSDTNum =  KeServiceDescriptorTable->Limit;
		
		pSysCallFilterInfo->ulSHADOWSSDTAddr=(ULONG)KeServiceDescriptorShadowTable->Base;
		pSysCallFilterInfo->ulSHADOWSSDTNum = KeServiceDescriptorShadowTable->Limit;

		if ( KeServiceDescriptorTable->Limit <= SSDT_MAX_NUM )
		{
			//保存原来的地址
			RtlCopyMemory(pSysCallFilterInfo->SavedSSDTTable,KeServiceDescriptorTable->Base,KeServiceDescriptorTable->Limit * 4);
			SetFakeFunction();
		
			//pSysCallFilterInfo->ProxySSDTTable[pSysCallFilterInfo->SSDTServiceIndex[SSDT_NTSETSYSTEMINFORMATION_INDEX]]=(ULONG)FakeNtSetSystemInformation;
			pSysCallFilterInfo->ProxySSDTTable[pSysCallFilterInfo->SSDTServiceIndex[SSDT_NTOPENPROCESS_INDEX]]=(ULONG)FakeNtOpenProcess;
			pSysCallFilterInfo->ProxySSDTTable[pSysCallFilterInfo->SSDTServiceIndex[SSDT_NTCREATETHREAD_INDEX]]=(ULONG)FakeNtCreateThread;
			pSysCallFilterInfo->ProxySSDTTable[pSysCallFilterInfo->SSDTServiceIndex[SSDT_NTOPENTHREAD_INDEX]]=(ULONG)FakeNtOpenThread;
			pSysCallFilterInfo->ProxySSDTTable[pSysCallFilterInfo->SSDTServiceIndex[SSDT_NTWRITEVIRTUALMEMORY_INDEX]]=(ULONG)FakeNtWriteVirtualMemory;
			//pSysCallFilterInfo->ProxySSDTTable[pSysCallFilterInfo->SSDTServiceIndex[SSDT_NTDUPLICATEOBJECT_INDEX]]=(ULONG)FakeNtDuplicateObject;

			pSysCallFilterInfo->ProxySSDTTable[pSysCallFilterInfo->SSDTServiceIndex[SSDT_NTDEBUGACTIVEPROCESS_INDEX]]=(ULONG)FakeNtDebugActiveProcess;
			pSysCallFilterInfo->ProxySSDTTable[pSysCallFilterInfo->SSDTServiceIndex[SSDT_NTREMOVEPROCESSDEBUG_INDEX]]=(ULONG)FakeNtRemoveProcessDebug;

			//	pSysCallFilterInfo->SSDTSwitchTable[pSysCallFilterInfo->SSDTServiceIndex[SSDT_NTSETSYSTEMINFORMATION_INDEX]]=1;
			pSysCallFilterInfo->SSDTSwitchTable[pSysCallFilterInfo->SSDTServiceIndex[SSDT_NTOPENPROCESS_INDEX]]=1;
			pSysCallFilterInfo->SSDTSwitchTable[pSysCallFilterInfo->SSDTServiceIndex[SSDT_NTCREATETHREAD_INDEX]]=1;
			pSysCallFilterInfo->SSDTSwitchTable[pSysCallFilterInfo->SSDTServiceIndex[SSDT_NTOPENTHREAD_INDEX]]=1;
			pSysCallFilterInfo->SSDTSwitchTable[pSysCallFilterInfo->SSDTServiceIndex[SSDT_NTWRITEVIRTUALMEMORY_INDEX]]=1;
			//pSysCallFilterInfo->SSDTSwitchTable[pSysCallFilterInfo->SSDTServiceIndex[SSDT_NTDUPLICATEOBJECT_INDEX]]=1;
			pSysCallFilterInfo->SSDTSwitchTable[pSysCallFilterInfo->SSDTServiceIndex[SSDT_NTDEBUGACTIVEPROCESS_INDEX]]=1;

			//HookSysenter();

			//status = STATUS_SUCCESS;

		}


	}

	if( pSysCallFilterInfo->ulSHADOWSSDTNum < SDOW_MAX_NUM)
	{
		if (MmIsAddressValid(pSysCallFilterInfo->ulSHADOWSSDTAddr))
		{
			RtlCopyMemory(pSysCallFilterInfo->SavedShadowSSDTTable , pSysCallFilterInfo->ulSHADOWSSDTAddr,
				pSysCallFilterInfo->ulSHADOWSSDTNum * 4);
			pSysCallFilterInfo->ProxyShadowSSDTTable[pSysCallFilterInfo->ShadowSSDTServiceIndex[SHADOWSSDT_NTUSERFINDWINDOWSEX_INDEX]]=(ULONG)FakeNtUserFindWindowEx;
			pSysCallFilterInfo->ShadowSSDTSwitchTable[pSysCallFilterInfo->ShadowSSDTServiceIndex[SHADOWSSDT_NTUSERFINDWINDOWSEX_INDEX]]=1;
			// 
			pSysCallFilterInfo->ProxyShadowSSDTTable[pSysCallFilterInfo->ShadowSSDTServiceIndex[SHADOWSSDT_NTUSERQUERYWINDOW_INDEX]]=(ULONG)FakeNtUserQueryWindow;
			pSysCallFilterInfo->ShadowSSDTSwitchTable[pSysCallFilterInfo->ShadowSSDTServiceIndex[SHADOWSSDT_NTUSERQUERYWINDOW_INDEX]]=1;
			// 
			pSysCallFilterInfo->ProxyShadowSSDTTable[pSysCallFilterInfo->ShadowSSDTServiceIndex[SHADOWSSDT_NTUSERSETWINDOWSHOOKEX_INDEX]] = (ULONG)FakeNtUserSetWindowsHookEx;
			pSysCallFilterInfo->ShadowSSDTSwitchTable[pSysCallFilterInfo->ShadowSSDTServiceIndex[SHADOWSSDT_NTUSERSETWINDOWSHOOKEX_INDEX]] = 1;
			// 
			pSysCallFilterInfo->ProxyShadowSSDTTable[pSysCallFilterInfo->ShadowSSDTServiceIndex[SHADOWSSDT_NTUSERBUILDHWNDLIST_INDEX]] = (ULONG)FakeNtUserBuildHwndList;
			pSysCallFilterInfo->ShadowSSDTSwitchTable[pSysCallFilterInfo->ShadowSSDTServiceIndex[SHADOWSSDT_NTUSERBUILDHWNDLIST_INDEX]] = 1;
			// 
			pSysCallFilterInfo->ProxyShadowSSDTTable[pSysCallFilterInfo->ShadowSSDTServiceIndex[SHADOWSSDT_NTUSERWINDOWFROMPOINT_INDEX]] = (ULONG)FakeNtUserWindowFromPoint;
			pSysCallFilterInfo->ShadowSSDTSwitchTable[pSysCallFilterInfo->ShadowSSDTServiceIndex[SHADOWSSDT_NTUSERWINDOWFROMPOINT_INDEX]] = 1;
			// 
			pSysCallFilterInfo->ProxyShadowSSDTTable[pSysCallFilterInfo->ShadowSSDTServiceIndex[SHADOWSSDT_NTUSERPOSTMESSAGE_INDEX]] = (ULONG)FakeNtUserPostMessage;
			pSysCallFilterInfo->ShadowSSDTSwitchTable[pSysCallFilterInfo->ShadowSSDTServiceIndex[SHADOWSSDT_NTUSERPOSTMESSAGE_INDEX]] = 1;
			// 
			pSysCallFilterInfo->ProxyShadowSSDTTable[pSysCallFilterInfo->ShadowSSDTServiceIndex[SHADOWSSDT_NTUSERDESTROYWINDOW_INDEX]] = (ULONG)FakeNtUserDestroyWindow;
			pSysCallFilterInfo->ShadowSSDTSwitchTable[pSysCallFilterInfo->ShadowSSDTServiceIndex[SHADOWSSDT_NTUSERDESTROYWINDOW_INDEX]] = 1;
			// 
			pSysCallFilterInfo->ProxyShadowSSDTTable[pSysCallFilterInfo->ShadowSSDTServiceIndex[SHADOWSSDT_NTUSERSHOWWINDOW_INDEX]] = (ULONG)FakeNtUserShowWindow;
			pSysCallFilterInfo->ShadowSSDTSwitchTable[pSysCallFilterInfo->ShadowSSDTServiceIndex[SHADOWSSDT_NTUSERSHOWWINDOW_INDEX]] = 1;
			// 
			pSysCallFilterInfo->ProxyShadowSSDTTable[pSysCallFilterInfo->ShadowSSDTServiceIndex[SHADOWSSDT_NTUSERSETWINDOWLONG_INDEX]] = (ULONG)FakeNtUserSetWindowLong;
			pSysCallFilterInfo->ShadowSSDTSwitchTable[pSysCallFilterInfo->ShadowSSDTServiceIndex[SHADOWSSDT_NTUSERSETWINDOWLONG_INDEX]] = 1;
			// 
			pSysCallFilterInfo->ProxyShadowSSDTTable[pSysCallFilterInfo->ShadowSSDTServiceIndex[SHADOWSSDT_NTUSERGETCLASSNAME_INDEX]] = (ULONG)FakeNtUserGetClassName;
			pSysCallFilterInfo->ShadowSSDTSwitchTable[pSysCallFilterInfo->ShadowSSDTServiceIndex[SHADOWSSDT_NTUSERGETCLASSNAME_INDEX]] = 1;
			// 
			pSysCallFilterInfo->ProxyShadowSSDTTable[pSysCallFilterInfo->ShadowSSDTServiceIndex[SHADOWSSDT_NTUSERINTERNALGETWINDOWTEXT_INDEX]] = (ULONG)FakeNtUserInternalGetWindowText;
			pSysCallFilterInfo->ShadowSSDTSwitchTable[pSysCallFilterInfo->ShadowSSDTServiceIndex[SHADOWSSDT_NTUSERINTERNALGETWINDOWTEXT_INDEX]] = 1;

			pSysCallFilterInfo->ProxyShadowSSDTTable[pSysCallFilterInfo->ShadowSSDTServiceIndex[SHADOWSSDT_NTUSERGETFOREGROUNDWINDOW_INDEX] ] = (ULONG)FakeNtUserGetForegroundWindow;
			pSysCallFilterInfo->ShadowSSDTSwitchTable[pSysCallFilterInfo->ShadowSSDTServiceIndex[SHADOWSSDT_NTUSERGETFOREGROUNDWINDOW_INDEX] ] = 1;

		}
	}

	//通过栈回溯  获得call ebx的返回地址
	if(_HookZwDisplayString() >= 0)
	{
		//In KiFastCallEntry
		//8053e62e 0f83a8010000    jae     nt!KiSystemCallExit2+0x9f (8053e7dc)
		//8053e634 f3a5            rep movs dword ptr es:[edi],dword ptr [esi]
		//8053e636 ffd3            call    ebx
		if(
			(ULONG)KiFastCallEntryRetAddress >= (ULONG)FirstModuleBase
			&& (ULONG)KiFastCallEntryRetAddress <= (ULONG)FirstModuleBase + FirstModuleSize
			&& (RetCode = (PUCHAR)KiFastCallEntryRetAddress - 2, MmIsAddressValid((PUCHAR)KiFastCallEntryRetAddress - 2))
			&& *(PUSHORT)RetCode == 0xd3ff 
			)
		{
			Status = _KisHookKiFastCallEntry();

		}

	}
	return Status;
}

VOID SetFakeFunction()
{
	pSysCallFilterInfo->SSDTServiceIndex[SSDT_NTSETSYSTEMINFORMATION_INDEX] = 240;
	pSysCallFilterInfo->SSDTServiceIndex[SSDT_NTOPENPROCESS_INDEX] = 0x7a;
	pSysCallFilterInfo->SSDTServiceIndex[SSDT_NTCREATETHREAD_INDEX] = 0x35;
	pSysCallFilterInfo->SSDTServiceIndex[SSDT_NTOPENTHREAD_INDEX] = 0x80;
	pSysCallFilterInfo->SSDTServiceIndex[SSDT_NTWRITEVIRTUALMEMORY_INDEX] = 0x115;
	pSysCallFilterInfo->SSDTServiceIndex[SSDT_NTDUPLICATEOBJECT_INDEX] = 0x44;
	pSysCallFilterInfo->SSDTServiceIndex[SSDT_NTDEBUGACTIVEPROCESS_INDEX] = 0x39;
	pSysCallFilterInfo->SSDTServiceIndex[SSDT_NTREMOVEPROCESSDEBUG_INDEX] = 0xBF; 

	pSysCallFilterInfo->ShadowSSDTServiceIndex[SHADOWSSDT_NTUSERFINDWINDOWSEX_INDEX] = 0x17a;
	pSysCallFilterInfo->ShadowSSDTServiceIndex[SHADOWSSDT_NTUSERQUERYWINDOW_INDEX] = 0x1e3;
	pSysCallFilterInfo->ShadowSSDTServiceIndex[SHADOWSSDT_NTUSERSETWINDOWSHOOKEX_INDEX] = 0x225;

	pSysCallFilterInfo->ShadowSSDTServiceIndex[SHADOWSSDT_NTUSERBUILDHWNDLIST_INDEX] = 0x138;
	pSysCallFilterInfo->ShadowSSDTServiceIndex[SHADOWSSDT_NTUSERWINDOWFROMPOINT_INDEX] = 0x250;
	pSysCallFilterInfo->ShadowSSDTServiceIndex[SHADOWSSDT_NTUSERSETPARENT_INDEX] = 0x211;
	pSysCallFilterInfo->ShadowSSDTServiceIndex[SHADOWSSDT_NTUSERPOSTMESSAGE_INDEX] = 0x1DB;
	pSysCallFilterInfo->ShadowSSDTServiceIndex[SHADOWSSDT_NTUSERMESSAGECALL_INDEX] = 0x1CC;
	pSysCallFilterInfo->ShadowSSDTServiceIndex[SHADOWSSDT_NTUSERDESTROYWINDOW_INDEX] = 0x163;
	pSysCallFilterInfo->ShadowSSDTServiceIndex[SHADOWSSDT_NTUSERSHOWWINDOW_INDEX] = 0x22B;
	pSysCallFilterInfo->ShadowSSDTServiceIndex[SHADOWSSDT_NTUSERSETWINDOWLONG_INDEX] = 0x220;


	pSysCallFilterInfo->ShadowSSDTServiceIndex[SHADOWSSDT_NTUSERGETCLASSNAME_INDEX] = 0x184;
	pSysCallFilterInfo->ShadowSSDTServiceIndex[SHADOWSSDT_NTUSERSETWINDOWPOS_INDEX] = 0x222;
	pSysCallFilterInfo->ShadowSSDTServiceIndex[SHADOWSSDT_NTUSERINTERNALGETWINDOWTEXT_INDEX] = 0x1C1;
	pSysCallFilterInfo->ShadowSSDTServiceIndex[SHADOWSSDT_NTUSERGETWINDOWPLACEMENT_INDEX] = 0x1B8;


	pSysCallFilterInfo->ShadowSSDTServiceIndex[SHADOWSSDT_NTUSERGETFOREGROUNDWINDOW_INDEX] = 0x194;

}


NTSTATUS _KisHookKiFastCallEntry()
{
	NTSTATUS Status = STATUS_SUCCESS;
	void *HookAddress;
	unsigned int v2; 
	NTSTATUS result; 
	ULONG MoveEbxAddress = 0; 
	ULONG KiFastCallEntryMoveEbxAddress;
	/*
	8053e619 8a0c18          mov     cl,byte ptr [eax+ebx]
	8053e61c 8b3f            mov     edi,dword ptr [edi]
	8053e61e 8b1c87          mov     ebx,dword ptr [edi+eax*4]          =====>v4
	8053e621 2be1            sub     esp,ecx
	8053e623 c1e902          shr     ecx,2
	8053e626 e95034a431      jmp     b1f81a7b
	8053e62b 90              nop
	8053e62c 90              nop
	8053e62d 90              nop
	8053e62e 0f83a8010000    jae     nt!KiSystemCallExit2+0x9f (8053e7dc)
	8053e634 f3a5            rep movs dword ptr es:[edi],dword ptr [esi]
	8053e636 ffd3            call    ebx
	8053e638 8be5            mov     esp,ebp
	*/
	//dword_4C1B8 = 1
	dword_4C1B8 = _KisGetKiFastCallEntryMoveEbxAddress((ULONG)KiFastCallEntryRetAddress - 100, 100, (ULONG*)&MoveEbxAddress);//v4 = 8053e61e
	if (dword_4C1B8)
	{
		KiFastCallEntryMoveEbxAddress = MoveEbxAddress; //0x8053e61e 8b1c87  mov     ebx,dword ptr [edi+eax*4]  
		Status = STATUS_SUCCESS;
	}
	//nt!KiFastCallEntry+0xe6:
	//8053e626 8bfc            mov     edi,esp
	//8053e628 3b35d4995580    cmp     esi,dword ptr [nt!MmUserProbeAddress (805599d4)]
	//8053e62e 0f83a8010000    jae     nt!KiSystemCallExit2+0x9f (8053e7dc)
	//8053e634 f3a5            rep movs dword ptr es:[edi],dword ptr [esi]
	//8053e636 ffd3            call    ebx
	//8053e638 8be5            mov     esp,ebp
	//8053e626 8bfc            mov     edi,esp
	//v1 = 8053e626    hook之后v1 = 0
	HookAddress = (void*)_KisGetKiFastCallEntryHookAddress((ULONG)KiFastCallEntryRetAddress - 100, 100, (ULONG*)MmUserProbeAddress);
	if ( HookAddress && MmIsAddressValid(HookAddress))
	{
		result = _KisHookKiFastCallEntryPoint((ULONG)HookAddress);
	}
	else
	{
		DbgPrint("已经被hook过了");
		result = _KisHookKiFastCallEntryPoint((ULONG)((PUCHAR)KiFastCallEntryRetAddress-18));
		result = STATUS_UNSUCCESSFUL;
	}
	return result;
}

                           //Ret Address - 100  , 100 ,  
NTSTATUS __stdcall _KisGetKiFastCallEntryMoveEbxAddress(ULONG StartAddress, ULONG Length, ULONG *MoveEbxAddress)
{
	ULONG Number = 0; 
	NTSTATUS Status; 
	PUCHAR Code; 

	if ( !Length )
		return 0;
	Code = (PUCHAR)(StartAddress + 2);  //0x8053e5d6   开始搜索8b 1c 87
	do        
	{ 
		/*
		8053e619 8a0c18          mov     cl,byte ptr [eax+ebx]
		8053e61c 8b3f            mov     edi,dword ptr [edi]
		8053e61e 8b1c87          mov     ebx,dword ptr [edi+eax*4]   //给ebx赋值语句  搜索硬编码8b1c87
		8053e621 2be1            sub     esp,ecx
		8053e623 c1e902          shr     ecx,2
		8053e626 e950349f31      jmp     kisknl+0x2ba7b (b1f31a7b)
		8053e62b 90              nop
		8053e62c 90              nop
		8053e62d 90              nop
		8053e62e 0f83a8010000    jae     nt!KiSystemCallExit2+0x9f (8053e7dc)
		8053e634 f3a5            rep movs dword ptr es:[edi],dword ptr [esi]
		8053e636 ffd3            call    ebx
		*/
		if (*(Code - 2)==0x8b && *(Code-1) == 0x1C && *Code == 0x87 )//28  -121
		{
			Status = STATUS_SUCCESS;
			*MoveEbxAddress = (ULONG)(Code - 2);//*a3 = 0x8053e61e
			++Number;
		}
		++Code;
		--Length;
	}
	while ( Length );
	if ( Number != 1 )
		return STATUS_UNSUCCESSFUL;
	return Status;
}
ULONG __stdcall _KisGetKiFastCallEntryHookAddress(ULONG StartAddress, ULONG Length, ULONG *MmUserProbeAddress1)
{
	ULONG Number = 0; 
	ULONG* v6;

	if ( !Length )
		return 0;
	/*
	Hook前
    kd> u 8053e626
	nt!KiFastCallEntry+0xe6:
	8053e626 8bfc            mov     edi,esp
	8053e628 3b35d4995580    cmp     esi,dword ptr [nt!MmUserProbeAddress (805599d4)]
	8053e62e 0f83a8010000    jae     nt!KiSystemCallExit2+0x9f (8053e7dc)
	8053e634 f3a5            rep movs dword ptr es:[edi],dword ptr [esi]
	8053e636 ffd3            call    ebx
	8053e638 8be5            mov     esp,ebp
	
	Hook后
	nt!KiFastCallEntry+0xe6:
	8053e626 e950e4c431      jmp     b218ca7b
	8053e62b 90              nop
	8053e62c 90              nop
	8053e62d 90              nop
	8053e62e 0f83a8010000    jae     nt!KiSystemCallExit2+0x9f (8053e7dc)
	8053e634 f3a5            rep movs dword ptr es:[edi],dword ptr [esi]
	8053e636 ffd3            call    ebx
	8053e638 8be5            mov     esp,ebp

	*/
	do
	{
		if ( *(PUCHAR)StartAddress == 0x8B && *(PUCHAR)((PUCHAR)StartAddress + 1) == 0xFC )
		{                   
			if ( *((PUCHAR)(StartAddress + 2)) != 0x3B || *((PUCHAR)(StartAddress + 3)) != 0x35&&(*(ULONG*)(StartAddress + 4)) != *(ULONG*)MmUserProbeAddress1)
			{
				if ( *(ULONG*)(StartAddress + 2) != 0x27245F6 )//41043446
				{

				}
				//dword_4C1CC = 2;
			}
			else
			{
				//dword_4C1CC = 1;
			}
			Number++;
			v6 = MmUserProbeAddress1;
		}
		++StartAddress;
		--Length;
	}
	while ( Length );
	if ( Number == 1 )
		return (ULONG)v6;
	return 0;
}
NTSTATUS __stdcall _KisHookKiFastCallEntryPoint(ULONG HookAddress)
{
	CHAR Code[] = {0x90,0x90,0x90};
	int (*v2)();
	signed int v4; 
	ULONG64 JmpCode; 
	KIRQL Irql;
	LOBYTE(JmpCode) = 0xE9;//-23
	//HIDWORD(JmpCode) = 0x90909090;  //-1869574000
	
	//8053e626 8bfc            mov     edi,esp
	v2 = (PUCHAR)_KifakeFastCallEntry;
	v4 = 8;


	*(ULONG*)(((PUCHAR)&JmpCode + 1)) = (PUCHAR)v2 - (PUCHAR)HookAddress - 5;
	FastCallEntryExitAddress = HookAddress + v4;//FastCallEntryExit的调用地址
	
	Irql = SecCloseProtect();
	HookKiFastCallEntry((signed __int64*)HookAddress, JmpCode, HIDWORD(JmpCode));
	memcpy(HookAddress+5,Code,3);
	SecOpenProtect(Irql);
	
	return 0;
}
ULONG __stdcall HookKiFastCallEntry(signed __int64 *a1, signed __int64 a2)
{
	return _InterlockedCompareExchange64(a1, a2, *a1);
}
_declspec(naked) VOID _KifakeFastCallEntry()
{

	__asm
	{
		pushfd;
		pushad;
		push edi;
		push ecx;
		push eax;
		push edx;
		push ebx;
		call _KisSyscallFilter;
		mov dword ptr [esp+10h],eax

		popad;
		popfd;
		//	这2条指令就是原先hook点的指令
		mov edi, esp;
		cmp esi, ds:MmUserProbeAddress;
		push FastCallEntryExitAddress;	//	注意这里，压地址，然后返回先前保存的下一条指令开始执行
		retn;
	}
}


ULONG __stdcall _KisSyscallFilter(ULONG FuncAddress,ULONG edx  ,ULONG Index ,ULONG Argc, PVOID SSDTBase)
{

	if ( SSDTBase ==  KeServiceDescriptorTable->Base && Index <= KeServiceDescriptorTable->Limit )
		{
			if (pSysCallFilterInfo->SSDTSwitchTable[Index])// && HookOrNot(SysCallNum,FALSE)KeServiceDescriptorTable->Base[Index]
			{
				DbgPrint("sysenter was ssdt hooked! Get service ID:%d  , Address :%p\n",Index,FuncAddress); //

				return pSysCallFilterInfo->ProxySSDTTable[Index];//返回我们代理函数的地址
			}
		}

		//判断是否是ShadowSSDT中的调用
		if (SSDTBase == KeServiceDescriptorShadowTable->Base && Index <= KeServiceDescriptorShadowTable->Limit)
		{
			if ( pSysCallFilterInfo->ShadowSSDTSwitchTable[Index] )// && HookOrNot(SysCallNum,TRUE)
			{
				DbgPrint("sysenter was shadow hooked! Get service ID:%d, Address :%p\n ",Index,FuncAddress); //
				return pSysCallFilterInfo->ProxyShadowSSDTTable[Index];//返回我们代理函数的地址
			}
		}

	return FuncAddress; // 不明调用,就直接返回原始例程
	
}

NTSTATUS _HookZwDisplayString()
{
	NTSTATUS Status = STATUS_UNSUCCESSFUL;
	PVOID ZwDisplayStringAddress; 
	ULONG_PTR Index; 
	ULONG_PTR OffsetOfSSDTService; 
	UNICODE_STRING SystemRoutineName;
	
	KIRQL Irql;
	SystemRoutineName.Length = 30;
	SystemRoutineName.MaximumLength = 32;
	SystemRoutineName.Buffer = L"ZwDisplayString";

	RtlInitUnicodeString(&StringTest,L"HelloWorld");
	ZwDisplayStringAddress = MmGetSystemRoutineAddress(&SystemRoutineName);   //0x804ff2d4
	if ( !ZwDisplayStringAddress )
		return STATUS_NOT_FOUND;

	if ( *(PUCHAR)ZwDisplayStringAddress == 0xB8 )//b8  43000000  比较函数第一个字节
	{
		Index = *(PULONG)((PUCHAR)ZwDisplayStringAddress + 1); //0x43  获得SSDT中索引号
	}
	else
	{
		//if ( !NtdllFunc_92820[55].FuncAddress )  //NtDisplayString的地址  dword_929D8
			return STATUS_NOT_FOUND;
		//通过函数地址   获得索引
		//Index = _GetIndexByFuncAddress(NtdllFunc_92820[55].FuncAddress);//dword_929D8
	}
	if ( Index & 0xFFFFC000 )//判断是否合法
		return STATUS_NOT_FOUND;

	Irql = SecCloseProtect();

	OffsetOfSSDTService = 4 * (Index & 0x3FFF); //0x10c   //32位索引号实际只使用12位                     //NtDisplayString  8060a0ce
	//Ori                                                                               hook
	OriZwDisplayStringAddress = InterlockedExchange(((PUCHAR)(KeServiceDescriptorTable->Base) + OffsetOfSSDTService), (LONG)_FakeZwDisplayString); //换了ZwDisplayString
	SecOpenProtect(Irql);
	//KiFastCallEntryRetAddress 是call ebx 的下一条指令
	if ( ZwDisplayString(&StringTest) == 0xEEEEE007 && KiFastCallEntryRetAddress ) //调用ZwDisplayString  初始化KiFastCallEntryRetAddress
	{
		if ( MmIsAddressValid((PVOID)KiFastCallEntryRetAddress) )
			Status = STATUS_SUCCESS;
	}

	Irql = SecCloseProtect(); 
	InterlockedExchange(((PUCHAR)(KeServiceDescriptorTable->Base) + OffsetOfSSDTService), (LONG)OriZwDisplayStringAddress);
	SecOpenProtect(Irql);
	return Status;
}

//Hook函数
ULONG __stdcall _FakeZwDisplayString(PCUNICODE_STRING String)  //hook
{
	ULONG EbpSub4; // eax@6
	PULONG CurrentEbp;

	if ( !ExGetPreviousMode() && String && RtlEqualUnicodeString(String, &StringTest, 0) )
	{

		/************************************************************************/
		/*
					lower   
				    ...             esp
			&CurrentEbp		 <--	varible
			&EbpSub4		 <--    varible
			ebp		         <--	ebp
			ret			     -->	call ebx 的下一条指令	 	FiFastCallEntry中		
			DisplayString	f8af1bc0	
			
					High
		*/
		//In KiFastCallEntry
		//8053e62e 0f83a8010000    jae     nt!KiSystemCallExit2+0x9f (8053e7dc)
		//8053e634 f3a5            rep movs dword ptr es:[edi],dword ptr [esi]
		//8053e636 ffd3            call    ebx
		/************************************************************************/

		CurrentEbp = (PULONG)((PUCHAR)&EbpSub4  +4 + 4);
		if ( _GetKiFastCallEntryRetuanAddress(CurrentEbp) //将全局变量KiFastCallEntryRetAddress赋值为call ebx(NtDisplayString)  的返回地址
			)  
			EbpSub4 = 0xEEEEE007;
		else
			EbpSub4 = STATUS_NOT_FOUND;
	}
	else
	{
		EbpSub4 = OriZwDisplayStringAddress((int)String); //调用原始的ZwDisplayString
	}
	return EbpSub4;
}

/*
通过ZwDisplayString的fake函数中的ebx 上面的 ret 地址，
将地址 上下的硬编码比较确认出ret地址
*/
ULONG __stdcall _GetKiFastCallEntryRetuanAddress(PVOID Ebp)
{
	PULONG pSaveRet = NULL;  
	PUCHAR pRetAddress = NULL;
	PUCHAR pCallAddress = NULL;
	signed int Count = 0;

	/*
	8053e636 ffd3            call    ebx
	8053e638 8be5            mov     esp,ebp
	8053e63a 8b0d24f1dfff    mov     ecx,dword ptr ds:[0FFDFF124h]
	8053e640 8b553c          mov     edx,dword ptr [ebp+3Ch]
	8053e643 899134010000    mov     dword ptr [ecx+134h],edx
	*/

	while ( Count < 2 )
	{
		if ( !MmIsAddressValid(Ebp) )
			break;
		if ( (ULONG)Ebp < MmUserProbeAddress )
			break;
		//Result Addr  Zw函数的Ebp + 4 = ret地址 就是call ebx的返回地址
		pSaveRet = (PULONG)((PUCHAR)Ebp + 4); 
		if ( !MmIsAddressValid(pSaveRet) )
			break;
		if ( (ULONG)pSaveRet < MmUserProbeAddress )
			break;
		//Ret Address
		pRetAddress = (PUCHAR)(*pSaveRet);
		if ( !MmIsAddressValid(pRetAddress) || (ULONG)pRetAddress < MmUserProbeAddress )
			break;
		//Call ebx Address
		pCallAddress = (PUCHAR)(pRetAddress - 2);
		if ( MmIsAddressValid(pCallAddress)
			&& *(PUSHORT)pCallAddress == 0xd3ff  //FF FF D3 FF
		/*	&& pCallAddress > (PUCHAR)FirstModuleImageBase
			&& pCallAddress < (PUCHAR)FirstModuleImageBase + FirstModuleSize*/ )//判断是否在第一模块内
		{
			KiFastCallEntryRetAddress = (ULONG_PTR)pRetAddress;  //Ret Address  
			DbgPrint("RetAddress = %x",KiFastCallEntryRetAddress);
			return 1;
		}
		Ebp = (PVOID)(*(ULONG*)Ebp);//如果已经被hook，得到上一层函数帧，一般只做两次
		++Count;

		
	}
	return 0;
}


PVOID GetKeShadowServiceDescriptorTable64()
{
	PUCHAR StartAddress = (PUCHAR)__readmsr(0xC0000082);
	PUCHAR i = NULL;
	UCHAR v1=0,v2=0,v3=0;
	int       iOffset = 0;
	ULONG_PTR SSSDTDescriptor = 0;
	for(i=StartAddress;i<StartAddress+PAGE_SIZE;i++)
	{
		if( MmIsAddressValid(i) && MmIsAddressValid(i+1) && MmIsAddressValid(i+2) )
		{
			v1 = *i;
			v2 = *(i+1);
			v3 = *(i+2);
			if(v1==0x4c && v2==0x8d && v3==0x1d) 
			{
				memcpy(&iOffset,i+3,4);
				SSSDTDescriptor = iOffset + (ULONG_PTR)i + 7;
				SSSDTDescriptor+=32;
				return (PVOID)SSSDTDescriptor;
			}
		}
	}
	return 0;
}
PVOID GetKeShadowServiceDescriptorTable32()
{
	WCHAR wzKeAddSystemServiceTable[] = L"KeAddSystemServiceTable";
	ULONG_PTR SSSDTDescriptor = 0;
	PUCHAR i = 0;
	PUCHAR StartAddress;
	UCHAR v1=0,v2=0;
	StartAddress = (PUCHAR)GetFunctionAddressByNameFromNtosExport(wzKeAddSystemServiceTable);
	if (StartAddress==NULL)
	{
		return 0;
	}
	for(i=StartAddress;i<StartAddress+PAGE_SIZE;i++)
	{
		if( MmIsAddressValid(i) && MmIsAddressValid(i+1))
		{
			v1 = *i;
			v2 = *(i+1);
			if(v1==0x8d && v2==0x88) 
			{
				SSSDTDescriptor = *(ULONG_PTR*)(i+2);
				SSSDTDescriptor = SSSDTDescriptor + 16;
				return (PVOID)SSSDTDescriptor;
			}
		}
	}
	return 0;
}

PVOID 
	GetFunctionAddressByNameFromNtosExport(WCHAR *wzFunctionName)
{
	UNICODE_STRING uniFunctionName;  
	PVOID FunctionAddress = NULL;
	if (wzFunctionName && wcslen(wzFunctionName) > 0)
	{
		RtlInitUnicodeString(&uniFunctionName, wzFunctionName);      
		FunctionAddress = MmGetSystemRoutineAddress(&uniFunctionName);  
	}
	return FunctionAddress;
}


NTSTATUS FuncMonitorDeviceControl(ULONG_PTR  uIoControlCode,PVOID InputBuffer,ULONG_PTR uInSize,PVOID OutputBuffer,ULONG_PTR uOutSize)
{	
	NTSTATUS Status = STATUS_SUCCESS;
	ULONG DataSize = sizeof(FUNCDATA);
	if(uIoControlCode == CTL_SETFUNCEVENT)
	{
	
		__try{
			DbgPrint("SetMonitorProcessRoutine");
			if (InputBuffer!=NULL&&uInSize==sizeof(ULONG_PTR)*3)
			{		
				DbgPrint("us");
			//	Status = FuncEventToKernelEvent((PULONG_PTR)InputBuffer,uInSize/sizeof(ULONG_PTR));

				if(!NT_SUCCESS(Status))
				{
					return Status;
				}
				if(WinVersion==WINDOWS_XP)
				{
					Status =_KisInstallKiFastCallEntryHook();
					if (!NT_SUCCESS(Status))
					{
						DbgPrint("gg");
						Status = STATUS_UNSUCCESSFUL;
					}
				}
				else if(WinVersion == WINDOWS_7)
				{

				}
			}
			Status = STATUS_SUCCESS;
		}
		__except(EXCEPTION_EXECUTE_HANDLER)
		{
			DbgPrint("No Set");
			Status = STATUS_SUCCESS;
		}
	}
	else if(uIoControlCode==CTL_GETFUNCBUFFER)
	{
		__try{

			DbgPrint("GetMonitorProcessInfor");
			Status = STATUS_SUCCESS;
		}
		__except(EXCEPTION_EXECUTE_HANDLER)
		{
			DbgPrint("No Get");
			Status = STATUS_UNSUCCESSFUL;
		}
	}
	return Status;
}


NTSTATUS FakeNtSetSystemInformation(
	__in SYSTEM_INFORMATION_CLASS SystemInformationClass,
	__inout PVOID SystemInformation,
	__in ULONG SystemInformationLength)
{	

	DbgPrint("Enter NtSetSystemInformation\r\n");
	return ((NTSETSYSTEMINFORMATION)pSysCallFilterInfo->SavedSSDTTable[pSysCallFilterInfo->SSDTServiceIndex[SSDT_NTSETSYSTEMINFORMATION_INDEX]])(
		SystemInformationClass,
		SystemInformation,
		SystemInformationLength);
}

NTSTATUS FakeNtOpenProcess (
	__out PHANDLE ProcessHandle,
	__in ACCESS_MASK DesiredAccess,
	__in POBJECT_ATTRIBUTES ObjectAttributes,
	__in_opt PCLIENT_ID ClientId
	)
{


	DbgPrint("Enter NtOpenProcess\r\n");

	return ((NTOPENPROCESS)pSysCallFilterInfo->SavedSSDTTable[pSysCallFilterInfo->SSDTServiceIndex[SSDT_NTOPENPROCESS_INDEX]])(
		ProcessHandle,
		DesiredAccess,
		ObjectAttributes,
		ClientId);
	;
}

#define PROCESS_CREATE_THREAD   0x0002


NTSTATUS FakeNtCreateThread(
	__out PHANDLE ThreadHandle,
	__in ACCESS_MASK DesiredAccess,
	__in_opt POBJECT_ATTRIBUTES ObjectAttributes,
	__in HANDLE ProcessHandle,
	__out PCLIENT_ID ClientId,
	__in PCONTEXT ThreadContext,
	__in PINITIAL_TEB InitialTeb,
	__in BOOLEAN CreateSuspended
	)
{
	DbgPrint("Enter NtCreateThread\r\n");
	return ((NTCREATETHREAD)pSysCallFilterInfo->SavedSSDTTable[pSysCallFilterInfo->SSDTServiceIndex[SSDT_NTCREATETHREAD_INDEX]])(
		ThreadHandle,
		DesiredAccess,
		ObjectAttributes,
		ProcessHandle,
		ClientId,
		ThreadContext,
		InitialTeb,
		CreateSuspended);
}

#pragma LOCKEDCODE

NTSTATUS FakeNtDebugActiveProcess  (IN HANDLE  ProcessHandle,  
	IN HANDLE  DebugObjectHandle   
	)
{
	DbgPrint("Enter NtDebugActiveProcess\r\n");
	return ((NTDEBUGACTIVEPROCESS)pSysCallFilterInfo->SavedSSDTTable[pSysCallFilterInfo->SSDTServiceIndex[SSDT_NTDEBUGACTIVEPROCESS_INDEX]])(
		ProcessHandle,
		DebugObjectHandle
		);

}

#pragma LOCKEDCODE

NTSTATUS FakeNtRemoveProcessDebug  ( IN HANDLE  ProcessHandle,  
	IN HANDLE  DebugObjectHandle   
	)
{
	DbgPrint("Enter NtRemoveProcessDebug\r\n");
	return ((NTREMOVEPROCESSDEBUG)pSysCallFilterInfo->SavedSSDTTable[pSysCallFilterInfo->SSDTServiceIndex[SSDT_NTREMOVEPROCESSDEBUG_INDEX]])(
		ProcessHandle,
		DebugObjectHandle
		);
}


NTSTATUS FakeNtWriteVirtualMemory(
	__in HANDLE ProcessHandle,
	__in_opt PVOID BaseAddress,
	__in_bcount(BufferSize) CONST VOID *Buffer,
	__in SIZE_T BufferSize,
	__out_opt PSIZE_T NumberOfBytesWritten
	)

{
	DbgPrint("Enter NtWriteVirtualMemory\r\n");
	return ((NTWRITEVIRTUALMEMORY)pSysCallFilterInfo->SavedSSDTTable[pSysCallFilterInfo->SSDTServiceIndex[SSDT_NTWRITEVIRTUALMEMORY_INDEX]])(
		ProcessHandle,
		BaseAddress,
		Buffer,
		BufferSize,
		NumberOfBytesWritten);
}

NTSTATUS FakeNtOpenThread (
	__out PHANDLE ThreadHandle,
	__in ACCESS_MASK DesiredAccess,
	__in POBJECT_ATTRIBUTES ObjectAttributes,
	__in_opt PCLIENT_ID ClientId
	)
{
	DbgPrint("Enter NtOpenThread\r\n");
	return ((NTOPENTHREAD)pSysCallFilterInfo->SavedSSDTTable[pSysCallFilterInfo->SSDTServiceIndex[SSDT_NTOPENTHREAD_INDEX]])(
		ThreadHandle,
		DesiredAccess,
		ObjectAttributes,
		ClientId);
}



NTSTATUS FakeNtDuplicateObject (
	__in HANDLE SourceProcessHandle,
	__in HANDLE SourceHandle,
	__in_opt HANDLE TargetProcessHandle,
	__out_opt PHANDLE TargetHandle,
	__in ACCESS_MASK DesiredAccess,
	__in ULONG HandleAttributes,
	__in ULONG Options)
{
	DbgPrint("Enter NtDuplicateObject\r\n");
	return ((NTDUPLICATEOBJECT)pSysCallFilterInfo->SavedSSDTTable[pSysCallFilterInfo->SSDTServiceIndex[SSDT_NTDUPLICATEOBJECT_INDEX]])(
		SourceProcessHandle,
		SourceHandle,
		TargetProcessHandle,
		TargetHandle,
		DesiredAccess,
		HandleAttributes,
		Options);
}


NTSTATUS
	NTAPI FakeNtQueueApcThread(
	IN HANDLE ThreadHandle,
	IN PKNORMAL_ROUTINE ApcRoutine,
	IN PVOID ApcContext OPTIONAL,
	IN PVOID Argument1 OPTIONAL,
	IN PVOID Argument2 OPTIONAL
	)
{
	DbgPrint("Enter NtQueueApcThread\r\n");
	return ((NTQUEUEAPCTHREAD)pSysCallFilterInfo->SavedSSDTTable[pSysCallFilterInfo->SSDTServiceIndex[SSDT_NTQUEUEAPCTHREAD_INDEX]])(
		ThreadHandle,
		ApcRoutine,
		ApcContext,
		Argument1,
		Argument2);
}


HWND 
	FakeNtUserFindWindowEx(
	__in HWND hwndParent, 
	__in HWND hwndChild, 
	__in PUNICODE_STRING pstrClassName, 
	__in PUNICODE_STRING pstrWindowName, 
	__in DWORD dwType)
{
	DbgPrint("Enter NtUserFindWindowEx\r\n");
	return (( NTUSERFINDWINDOWEX )pSysCallFilterInfo->SavedShadowSSDTTable[pSysCallFilterInfo->ShadowSSDTServiceIndex[SHADOWSSDT_NTUSERFINDWINDOWSEX_INDEX]])(
		hwndParent, 
		hwndChild, 
		pstrClassName, 
		pstrWindowName, 
		dwType);
}


ULONG FakeNtUserGetForegroundWindow(VOID)
{
	DbgPrint("Enter NtUserGetForegroundWindow\r\n");
	return ((NTUSERGETFOREGROUNDWINDOW)pSysCallFilterInfo->SavedShadowSSDTTable[pSysCallFilterInfo->ShadowSSDTServiceIndex[SHADOWSSDT_NTUSERGETFOREGROUNDWINDOW_INDEX]])(
		);
}

UINT_PTR FakeNtUserQueryWindow(
	IN ULONG WindowHandle,
	IN ULONG TypeInformation)
{
	DbgPrint("Enter NtUserQueryWindow\r\n");
	return ((NTUSERQUERYWINDOW)pSysCallFilterInfo->SavedShadowSSDTTable[pSysCallFilterInfo->ShadowSSDTServiceIndex[SHADOWSSDT_NTUSERQUERYWINDOW_INDEX]])(
		WindowHandle,
		TypeInformation);;
}


HANDLE  FakeNtUserSetWindowsHookEx(HANDLE Mod,
	PUNICODE_STRING UnsafeModuleName,
	DWORD ThreadId,
	INT HookId,
	HOOKPROC HookProc,
	BOOL Ansi)

{
	DbgPrint("Enter NtUserSetWindowsHookEx\r\n");
	return ((NTUSERSETWINDOWSHOOKEX)pSysCallFilterInfo->SavedShadowSSDTTable[pSysCallFilterInfo->ShadowSSDTServiceIndex[SHADOWSSDT_NTUSERSETWINDOWSHOOKEX_INDEX]])(
		Mod,
		UnsafeModuleName,
		ThreadId,
		HookId,
		HookProc,
		Ansi);
}

NTSTATUS FakeNtUserBuildHwndList(
	IN HDESK hdesk, 
	IN HWND hwndNext, 
	IN ULONG fEnumChildren, 
	IN DWORD idThread, 
	IN UINT cHwndMax, 
	OUT HWND *phwndFirst, 
	OUT ULONG* pcHwndNeeded)
{
	DbgPrint("Enter NtUserBuildHwndList\r\n");
	return ((NTUSERBUILDHWNDLIST)pSysCallFilterInfo->SavedShadowSSDTTable[pSysCallFilterInfo->ShadowSSDTServiceIndex[SHADOWSSDT_NTUSERBUILDHWNDLIST_INDEX]])(
		hdesk, 
		hwndNext, 
		fEnumChildren, 
		idThread, 
		cHwndMax, 
		phwndFirst, 
		pcHwndNeeded);;
}




ULONG FakeNtUserWindowFromPoint(LONG x, LONG y)
{
	DbgPrint("Enter NtUserWindowFromPoint\r\n");
	return ((NTUSERWINDOWFROMPOINT)pSysCallFilterInfo->SavedShadowSSDTTable[pSysCallFilterInfo->ShadowSSDTServiceIndex[SHADOWSSDT_NTUSERWINDOWFROMPOINT_INDEX]])
		(
		x,
		y);
}




HWND FakeNtUserSetParent( HWND hWndChild , HWND hWndNewParent )
{
	DbgPrint("Enter NtUserSetParent\r\n");
	return ((NTUSERSETPARENT)pSysCallFilterInfo->SavedShadowSSDTTable[pSysCallFilterInfo->ShadowSSDTServiceIndex[SHADOWSSDT_NTUSERSETPARENT_INDEX]])(
		hWndChild,
		hWndNewParent
		);


}

BOOL FakeNtUserPostMessage(HWND hWnd,UINT Msg,WPARAM wParam,LPARAM lParam)
{
	DbgPrint("Enter NtUserPostMessage\r\n");
	return ((NTUSERPOSTMESSAGE)pSysCallFilterInfo->SavedShadowSSDTTable[pSysCallFilterInfo->ShadowSSDTServiceIndex[SHADOWSSDT_NTUSERPOSTMESSAGE_INDEX]] )(
		hWnd,
		Msg,
		wParam,
		lParam);

}


ULONG FakeNtUserMessageCall(
	HWND hwnd, 
	UINT msg,
	WPARAM wParam,
	LPARAM lParam,
	PULONG xParam,
	ULONG xpfnProc,
	BOOL bAnsi
	)
{

	DbgPrint("Enter NtUserMessageCall\r\n");
	return ((NTUSERMESSAGECALL)pSysCallFilterInfo->SavedShadowSSDTTable[pSysCallFilterInfo->ShadowSSDTServiceIndex[SHADOWSSDT_NTUSERMESSAGECALL_INDEX]] )(
		hwnd, 
		msg,
		wParam,
		lParam,
		xParam,
		xpfnProc,
		bAnsi);

}


BOOL FakeNtUserDestroyWindow( HWND hWnd )
{
	DbgPrint("Enter NtUserDestroyWindow\r\n");
return ((NTUSERDESTROYWINDOW)pSysCallFilterInfo->SavedShadowSSDTTable[pSysCallFilterInfo->ShadowSSDTServiceIndex[SHADOWSSDT_NTUSERDESTROYWINDOW_INDEX]] )(
		hWnd);
}


BOOL FakeNtUserShowWindow(HWND hWnd,long nCmdShow)
{
	DbgPrint("Enter NtUserShowWindow\r\n");
	return ((NTUSERSHOWWINDOW)pSysCallFilterInfo->SavedShadowSSDTTable[pSysCallFilterInfo->ShadowSSDTServiceIndex[SHADOWSSDT_NTUSERSHOWWINDOW_INDEX]] )
		(
		hWnd,
		nCmdShow
		);

}


long FakeNtUserSetWindowLong(HWND hWnd, ULONG Index, long NewValue, BOOL Ansi)
{
	DbgPrint("Enter NtUserSetWindowLong\r\n");
	return ((NTUSERSETWINDOWLONG)pSysCallFilterInfo->SavedShadowSSDTTable[pSysCallFilterInfo->ShadowSSDTServiceIndex[SHADOWSSDT_NTUSERSETWINDOWLONG_INDEX]] )(
		hWnd, 
		Index, 
		NewValue, 
		Ansi);

}


INT CALLBACK
	FakeNtUserInternalGetWindowText(HWND hWnd, LPWSTR lpString, INT nMaxCount)
{
	DbgPrint("Enter NtUserInternalGetWindowText\r\n");
	return ((NTUSERINTERNALGETWINDOWTEXT)pSysCallFilterInfo->SavedShadowSSDTTable[pSysCallFilterInfo->ShadowSSDTServiceIndex[SHADOWSSDT_NTUSERINTERNALGETWINDOWTEXT_INDEX]] )(
		hWnd, 
		lpString, 
		nMaxCount);
}



BOOL CALLBACK
	FakeNtUserSetWindowPos(
	HWND hWnd,
	HWND hWndInsertAfter,
	int X,
	int Y,
	int cx,
	int cy,
	UINT uFlags)
{

	DbgPrint("Enter NtUserSetWindowPos\r\n");
	return ((NTUSERSETWINDOWPOS)pSysCallFilterInfo->SavedShadowSSDTTable[pSysCallFilterInfo->ShadowSSDTServiceIndex[SHADOWSSDT_NTUSERSETWINDOWPOS_INDEX]] )(
		hWnd,
		hWndInsertAfter,
		X,
		Y,
		cx,
		cy,
		uFlags);
}


INT CALLBACK
	FakeNtUserGetClassName (IN HWND hWnd,
	OUT PUNICODE_STRING ClassName,
	IN BOOL Ansi)
{
	DbgPrint("Enter NtUserGetClassName\r\n");
	return ((NTUSERGETCLASSNAME)pSysCallFilterInfo->SavedShadowSSDTTable[pSysCallFilterInfo->ShadowSSDTServiceIndex[SHADOWSSDT_NTUSERGETCLASSNAME_INDEX]] )(
		hWnd,
		ClassName,
		Ansi);
}


BOOL CALLBACK
	FakeNtUserGetWindowPlacement(HWND hWnd,
	void* lpwndpl)
{
	DbgPrint("Enter NtUserGetWindowPlacement\r\n");
	return ((NTUSERGETWINDOWPLACEMENT)pSysCallFilterInfo->SavedShadowSSDTTable[pSysCallFilterInfo->ShadowSSDTServiceIndex[SHADOWSSDT_NTUSERGETWINDOWPLACEMENT_INDEX]] )(
		hWnd,
		lpwndpl);
}
