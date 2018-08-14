#include "SSDT.h"

extern ULONG_PTR  SSDTDescriptor;
extern PDRIVER_OBJECT   CurrentDriverObject;
extern PVOID            SysSSDTModuleBase;
extern ULONG_PTR        ulSSDTSysModuleSize;
PVOID GetSSDTFunctionAddress64(ULONG_PTR ulIndex,ULONG_PTR SSDTDescriptor)
{
	LONG      ulv1 = 0;
	ULONG_PTR ulv2 = 0;
	ULONG_PTR ServiceTableBase= 0 ;
	PSYSTEM_SERVICE_TABLE64 SSDT = (PSYSTEM_SERVICE_TABLE64)SSDTDescriptor;
	ServiceTableBase=(ULONG_PTR)(SSDT ->ServiceTableBase);
	ulv2 = ServiceTableBase + 4 * ulIndex;
	ulv1 = *(PLONG)ulv2;
	ulv1 = ulv1>>4;
	return (PVOID)(ServiceTableBase + (ULONG_PTR)ulv1);
}

PVOID GetSSDTFunctionAddress32(ULONG_PTR ulIndex,ULONG_PTR SSDTDescriptor)
{
	ULONG_PTR ServiceTableBase= 0 ;
	PSYSTEM_SERVICE_TABLE32 SSDT = (PSYSTEM_SERVICE_TABLE32)SSDTDescriptor;

	ServiceTableBase=(ULONG)(SSDT ->ServiceTableBase);

	return (PVOID)(*(PULONG_PTR)(ServiceTableBase + 4 * ulIndex));
}

/**/
BOOLEAN GetSysModuleByLdrDataTableSSDT(WCHAR* wzModuleName)
{
	BOOLEAN bRet = FALSE;
	if (CurrentDriverObject)
	{
		PLDR_DATA_TABLE_ENTRY ListHead = NULL, ListNext = NULL;

		ListHead = ListNext = (PLDR_DATA_TABLE_ENTRY)CurrentDriverObject->DriverSection;  //dt _DriverObject
		while((PLDR_DATA_TABLE_ENTRY)ListNext->InLoadOrderLinks.Flink != ListHead)
		{
			//DbgPrint("%wZ\r\n",&ListNext->BaseDllName);
			if (ListNext->BaseDllName.Buffer&& 														
				wcsstr((WCHAR*)(ListNext->BaseDllName.Buffer),wzModuleName)!=NULL)
			{
				SysSSDTModuleBase = (PVOID)(ListNext->DllBase);
				ulSSDTSysModuleSize = ListNext->SizeOfImage;

				//DbgPrint("%x    %x\r\n",ListNext->DllBase,ListNext->EntryPoint);
				//	DbgPrint("ModuleNameSecondGet:%wZ\r\n",&(ListNext->FullDllName));

				bRet = TRUE;
				break;
			}
			ListNext = (PLDR_DATA_TABLE_ENTRY)ListNext->InLoadOrderLinks.Flink;
		}
	}
	return bRet;
}

/*从DriverObject->DriverSection 驱动列表中查找当前函数地址所属的模块名称*/
NTSTATUS GetSysModuleByLdrDataTable2(PVOID Address,WCHAR* wzModuleName)
{
	BOOLEAN bRet = FALSE;
	ULONG_PTR ulBase;
	ULONG ulSize;

	if (CurrentDriverObject)
	{
		PKLDR_DATA_TABLE_ENTRY ListHead = NULL, ListNext = NULL;

		ListHead = ListNext = (PKLDR_DATA_TABLE_ENTRY)CurrentDriverObject->DriverSection;  //dt _DriverObject
		while((PKLDR_DATA_TABLE_ENTRY)ListNext->InLoadOrderLinks.Flink != ListHead)
		{
			ulBase = (ListNext)->DllBase;
			ulSize = (ListNext)->SizeOfImage;
			if(ulBase!=0)
			{
				if((ULONG_PTR)Address>ulBase && (ULONG_PTR)Address < ulSize + ulBase)
				{
					__try
					{
						DbgPrint("%wZ\r\n",&ListNext->BaseDllName);
						DbgPrint("%wZ\r\n",&(ListNext->FullDllName));

						memcpy(wzModuleName,(WCHAR*)(((ListNext)->FullDllName).Buffer),sizeof(WCHAR)*60);
					}
					__except(EXCEPTION_EXECUTE_HANDLER)
					{
						DbgPrint("EXCEEPTION:%d",GetExceptionCode());
					}

					bRet = TRUE;
					break;
				} 
			}
			ListNext = (PKLDR_DATA_TABLE_ENTRY)ListNext->InLoadOrderLinks.Flink;
		}
	}
	return bRet;
	/*
	int       i = 0;
	NTSTATUS  Status = STATUS_SUCCESS;
	PVOID     Buffer = NULL;
	ULONG     ulNeeds = 0;

	Status = ZwQuerySystemInformation(SystemModuleInformation,NULL,0,&ulNeeds);

	if (Status!=STATUS_INFO_LENGTH_MISMATCH)
	{
		return FALSE;
	}
	Buffer = ExAllocatePool(PagedPool,ulNeeds); 

	if (Buffer==NULL)
	{
		return FALSE;
	}
	Status = ZwQuerySystemInformation(SystemModuleInformation,Buffer,ulNeeds,&ulNeeds);

	if (!NT_SUCCESS(Status))
	{
		ExFreePool(Buffer);
		return FALSE;
	}

	for (i=0;i<((PRTL_PROCESS_MODULES)Buffer)->NumberOfModules;i++)
	{

		if (Address>((PRTL_PROCESS_MODULES)Buffer)->Modules[i].ImageBase&&Address<(PVOID)((ULONG_PTR)((PRTL_PROCESS_MODULES)Buffer)->Modules[i].ImageBase
			+(ULONG_PTR)((PRTL_PROCESS_MODULES)Buffer)->Modules[i].ImageSize))
		{
			//BaseAddress  =  ((PRTL_PROCESS_MODULES)Buffer)->Modules[i].ImageBase;
			//ulModuleSize = ((PRTL_PROCESS_MODULES)Buffer)->Modules[i].ImageSize;

			__try
			{
			//	wcscpy(wzModuleName,((PRTL_PROCESS_MODULES)Buffer)->Modules[i].FullPathName);
			//	memcpy(wzModuleName,((PRTL_PROCESS_MODULES)Buffer)->Modules[i].FullPathName,sizeof(WCHAR)*60);
			}
			__except(EXCEPTION_EXECUTE_HANDLER)
			{
				DbgPrint("EXCEEPTION:%d",GetExceptionCode());
			}

			return TRUE;

		}

	}


	ExFreePool(Buffer);

	return FALSE;*/
}

VOID  UnHookSSDT(ULONG ulIndex, ULONG_PTR OriginalFunctionAddress)
{
#ifdef _WIN64
	ULONG_PTR v2 = 0;
	ULONG_PTR ServiceTableBase = 0 ;
	ULONG CurrentFunctionOffsetOfSSDT = 0;
	PSYSTEM_SERVICE_TABLE64 SSDT = (PSYSTEM_SERVICE_TABLE64)SSDTDescriptor;
	ServiceTableBase=(ULONG_PTR)(SSDT ->ServiceTableBase);
	CurrentFunctionOffsetOfSSDT = (ULONG)((ULONG_PTR)OriginalFunctionAddress - (ULONG_PTR)(SSDT->ServiceTableBase));
	CurrentFunctionOffsetOfSSDT = CurrentFunctionOffsetOfSSDT<<4;

	v2 = ServiceTableBase + 4 * ulIndex;
	WPOFF();
	*(PLONG)v2 = CurrentFunctionOffsetOfSSDT;
	WPON();
#else
	ULONG_PTR ServiceTableBase = 0 ;
	ULONG_PTR v2 = 0;
	PSYSTEM_SERVICE_TABLE32 SSDT = (PSYSTEM_SERVICE_TABLE32)SSDTDescriptor;
	ServiceTableBase=(ULONG_PTR)(SSDT->ServiceTableBase);

	v2 = ServiceTableBase + 4 * ulIndex;
	WPOFF();
	*(PLONG)v2 = (ULONG)OriginalFunctionAddress;
	WPON();
#endif

}






BOOLEAN ResumeSSDTInlineHook(ULONG ulIndex,UCHAR* szOriginalFunctionCode)
{
	PVOID  CurrentFunctionAddress = NULL;
#ifdef _WIN64
	CurrentFunctionAddress = GetSSDTFunctionAddress64(ulIndex,SSDTDescriptor);
#else
	CurrentFunctionAddress = GetSSDTFunctionAddress32(ulIndex,SSDTDescriptor);
#endif

	WPOFF();
	SafeCopyMemory(CurrentFunctionAddress,szOriginalFunctionCode,CODE_LENGTH);  
	WPON();

	return TRUE;
}