#include "GetService.h"

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
				/*

				: kd> u fffff800`03ed7640 l 100
				nt!KiSystemCall64:
				fffff800`03ed7640 0f01f8          swapgs
				fffff800`03ed7643 654889242510000000 mov   qword ptr gs:[10h],rsp
			
	
				nt!KiSystemServiceRepeat:
				fffff800`03ed7772 4c8d15c7202300  lea     r10,[nt!KeServiceDescriptorTable (fffff800`04109840)]
				fffff800`03ed7779 4c8d1d00212300  lea     r11,[nt!KeServiceDescriptorTableShadow (fffff800`04109880)
				*/
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
	/*
	kd> u KeAddSystemServiceTable l 50
	nt!KeAddSystemServiceTable:
	805b3ff1 8bff            mov     edi,edi
	805b3ff3 55              push    ebp
	805b3ff4 8bec            mov     ebp,esp
	805b3ff6 837d1803        cmp     dword ptr [ebp+18h],3
	805b3ffa 774e            ja      nt!KeAddSystemServiceTable+0x6b (805b404a)
	805b3ffc 8b4518          mov     eax,dword ptr [ebp+18h]
	805b3fff c1e004          shl     eax,4
	805b4002 83b82035568000  cmp     dword ptr nt!KeServiceDescriptorTable (80563520)[eax],0
	805b4009 753f            jne     nt!KeAddSystemServiceTable+0x6b (805b404a)
	805b400b 8d88e0345680    lea     ecx,nt!KeServiceDescriptorTableShadow (805634e0)[eax]
	*/
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





PVOID GetKeServiceDescriptorTable64()
{
	PUCHAR StartSearchAddress = (PUCHAR)__readmsr(0xC0000082);
	PUCHAR EndSearchAddress = StartSearchAddress + 0x500;
	PUCHAR i = NULL;
	UCHAR b1=0,b2=0,b3=0;
	ULONG_PTR ulv1 = 0;
	PVOID FunctionAddress = 0;
	for(i=StartSearchAddress;i<EndSearchAddress;i++)
	{
		if( MmIsAddressValid(i) && MmIsAddressValid(i+1) && MmIsAddressValid(i+2) )
		{
			b1=*i;
			b2=*(i+1);
			b3=*(i+2);
			if( b1==0x4c && b2==0x8d && b3==0x15 ) 
			{
				memcpy(&ulv1,i+3,4);
				FunctionAddress = (PVOID)((ULONG_PTR)ulv1 + (ULONG_PTR)i + 7);
				return FunctionAddress;
			}
		}
	}
	return 0;
}


