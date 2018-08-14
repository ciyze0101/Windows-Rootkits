#include "SSSDT.h"

extern ULONG_PTR  SSSDTDescriptor;

extern PDRIVER_OBJECT   CurrentDriverObject;
extern PVOID            SysModuleBsse;
extern ULONG_PTR        ulSysModuleSize;

//获得SSSDT地址，索引*4+SSSDT 存放着一个数，右移4位即获得函数偏移。加上SSSDT即得到 对应函数地址
PVOID GetSSSDTFunctionAddress64(ULONG ulIndex)
{
	LONG v1 = 0;
	ULONG_PTR v2 = 0;
	ULONG_PTR ServiceTableBase= 0 ;
	PSYSTEM_SERVICE_TABLE64 SSSDT = (PSYSTEM_SERVICE_TABLE64)SSSDTDescriptor;
	ServiceTableBase=(ULONG_PTR)(SSSDT ->ServiceTableBase);
	v2 = ServiceTableBase + 4 * ulIndex;
	v1 = *(PLONG)v2;
	v1 = v1>>4;
	return (PVOID)(ServiceTableBase + (ULONG_PTR)v1);
}

//SSSDT基地址+4*Index里面存放着SSSDT对应的函数地址
PVOID GetSSSDTFunctionAddress32(ULONG ulIndex)
{
	ULONG_PTR ServiceTableBase= 0 ;
	PSYSTEM_SERVICE_TABLE32 SSSDT = (PSYSTEM_SERVICE_TABLE32)SSSDTDescriptor;
	ServiceTableBase = (ULONG_PTR)(SSSDT->ServiceTableBase);
	return (PVOID)(*(PULONG_PTR)((ULONG_PTR)ServiceTableBase + 4 * ulIndex));
}

//Ring3传入模块名称与DriverObject->DriverSection结构的链表中  驱动名称相比较， 一致则返回驱动地址
BOOLEAN GetSysModuleByLdrDataTable(WCHAR* wzModuleName)
{
	BOOLEAN bRet = FALSE;
	if (CurrentDriverObject)
	{
		PKLDR_DATA_TABLE_ENTRY ListHead = NULL, ListNext = NULL;

		ListHead = ListNext = (PKLDR_DATA_TABLE_ENTRY)CurrentDriverObject->DriverSection;  //dt _DriverObject
		while((PKLDR_DATA_TABLE_ENTRY)ListNext->InLoadOrderLinks.Flink != ListHead)
		{
			//DbgPrint("%wZ\r\n",&ListNext->BaseDllName);
			if (ListNext->BaseDllName.Buffer&& 														
				wcsstr((WCHAR*)(ListNext->BaseDllName.Buffer),wzModuleName)!=NULL)
			{
				SysModuleBsse = (PVOID)(ListNext->DllBase);
				ulSysModuleSize = ListNext->SizeOfImage;

				//DbgPrint("%x    %x\r\n",ListNext->DllBase,ListNext->EntryPoint);
				//	DbgPrint("ModuleNameSecondGet:%wZ\r\n",&(ListNext->FullDllName));

				bRet = TRUE;
				break;
			}
			ListNext = (PKLDR_DATA_TABLE_ENTRY)ListNext->InLoadOrderLinks.Flink;
		}
	}
	return bRet;
}

//遍历DriverObject->DriverSection结构，在驱动链表中查找函数所在模块
BOOLEAN GetSysModuleByLdrDataTable1(PVOID Address,WCHAR* wzModuleName)
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
			if((ULONG_PTR)Address > ulBase && (ULONG_PTR)Address < ulSize+ulBase)
			{
				memcpy(wzModuleName,(WCHAR*)(((ListNext)->FullDllName).Buffer),sizeof(WCHAR)*60);
				bRet = TRUE;
				break;
			} 
			ListNext = (PKLDR_DATA_TABLE_ENTRY)ListNext->InLoadOrderLinks.Flink;
		}
	}
	return bRet;
}

VOID  UnHookSSSDTWin7(ULONG ulIndex, ULONG_PTR OriginalFunctionAddress)
{
	ULONG_PTR v2 = 0;
	ULONG_PTR ServiceTableBase = 0 ;
	ULONG CurrentFunctionOffsetOfSSSDT = 0;
	PSYSTEM_SERVICE_TABLE64 SSSDT = (PSYSTEM_SERVICE_TABLE64)SSSDTDescriptor;
	ServiceTableBase=(ULONG_PTR)(SSSDT ->ServiceTableBase);
	CurrentFunctionOffsetOfSSSDT = (ULONG)((ULONG_PTR)OriginalFunctionAddress - (ULONG_PTR)(SSSDT->ServiceTableBase));
	CurrentFunctionOffsetOfSSSDT = CurrentFunctionOffsetOfSSSDT<<4;

	v2 = ServiceTableBase + 4 * ulIndex;
	WPOFF();
	*(PLONG)v2 = CurrentFunctionOffsetOfSSSDT;
	WPON();
}

VOID UnHookSSSDTWinXP(ULONG ulIndex, ULONG_PTR OriginalFunctionAddress)
{
	ULONG_PTR ServiceTableBase = 0 ;
	ULONG_PTR v2 = 0;
	PSYSTEM_SERVICE_TABLE32 SSSDT = (PSYSTEM_SERVICE_TABLE32)SSSDTDescriptor;
	ServiceTableBase=(ULONG_PTR)(SSSDT->ServiceTableBase);

	v2 = ServiceTableBase + 4 * ulIndex;
	WPOFF();
	*(PLONG)v2 = (ULONG)OriginalFunctionAddress;
	WPON();
}

BOOLEAN ResumeSSSDTInlineHook(ULONG ulIndex,UCHAR* szOriginalFunctionCode)
{
	PVOID  CurrentFunctionAddress = NULL;
#ifdef _WIN64
	CurrentFunctionAddress = GetSSSDTFunctionAddress64(ulIndex);
#else
	CurrentFunctionAddress = GetSSSDTFunctionAddress32(ulIndex);
#endif

	WPOFF();
	SafeCopyMemory(CurrentFunctionAddress,szOriginalFunctionCode,CODE_LENGTH);  
	//memcpy(CurrentFunctionAddress,szOriginalFunctionCode,CODE_LENGTH);
	WPON();

	return TRUE;
}