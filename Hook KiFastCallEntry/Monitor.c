
#ifndef CXX_MONITOR_H
#	include "Monitor.h"
#include "HookKiFastCallEntry.h"
#endif



extern BOOLEAN         IsClear;//资源是否销毁 

extern PKSERVICE_TABLE_DESCRIPTOR KeServiceDescriptorTable;
WCHAR FirstModulePath[260] = {0};
ULONG_PTR FirstModuleBase = 0;
ULONG_PTR FirstModuleSize = 0;
NTSTATUS
DriverEntry(IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING pRegistryString)
{
	NTSTATUS		status = STATUS_SUCCESS;


	// Unload routine
	DriverObject->DriverUnload = DriverUnload;



	
	InitGlobalVariable();

	GetFirstModuleInformation(DriverObject);

	_KisInstallKiFastCallEntryHook();

	return STATUS_SUCCESS;
}


VOID GetFirstModuleInformation(PDRIVER_OBJECT DriverObject)
{
	  ULONG Base=0;//模块基地址
    LDR_DATA_TABLE_ENTRY* SectionBase=NULL;
    LIST_ENTRY* Entry=NULL;
    LIST_ENTRY InLoadOrderLinks;
    Entry=((LIST_ENTRY*)DriverObject->DriverSection)->Flink;
    do
    {
        SectionBase=CONTAINING_RECORD(Entry,LDR_DATA_TABLE_ENTRY,InLoadOrderLinks);//得到这个Entry所属的Section的地址，此方法经过验证可行
        if (SectionBase->EntryPoint&&SectionBase->BaseDllName.Buffer&&SectionBase->FullDllName.Buffer&&
            SectionBase->LoadCount
            )
        {
			if(wcscmp(SectionBase->BaseDllName.Buffer,L"ntkrnlpa.exe")!=0)
			{
				wcsncpy(FirstModulePath,SectionBase->FullDllName.Buffer,SectionBase->FullDllName.Length);
				FirstModuleBase= SectionBase->DllBase;
				FirstModuleSize = SectionBase->SizeOfImage;
				DbgPrint("第一模块名称:%wZ,地址:%x\r\n",&(SectionBase->FullDllName),SectionBase->DllBase);
				break;
			}
			DbgPrint("第一模块名称:%wZ,地址:%x\n",&(SectionBase->FullDllName),SectionBase->DllBase);
			
  
        }
        Entry=Entry->Flink;
    }while(Entry!=((LIST_ENTRY*)DriverObject->DriverSection)->Flink);//直到遍历回来
}




VOID
	DriverUnload(IN PDRIVER_OBJECT DriverObject)
{	

	return;
}
