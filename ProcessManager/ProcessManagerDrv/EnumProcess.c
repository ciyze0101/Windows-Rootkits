#include "EnumProcess.h"
#include "Common.h"


extern WIN_VERSION WinVersion;
extern ULONG_PTR ProcessIdOfEprocess;
extern ULONG_PTR ProcessImageNameOfEprocess;



extern ULONG_PTR ulProcessParametersOfPeb;
extern ULONG_PTR ulImagePathNameOfProcessParameters;

extern ULONG_PTR Active_List;


extern ULONG_PTR PebOfEProcess;

extern ULONG_PTR PspCidTable;
extern ULONG_PTR FatherOfEprocess;


extern PEPROCESS g_EProcess;

ULONG_PTR SectionObjectOfEProcess = 0;
NTSTATUS EnumProcessByOpenProcess(PVOID OutputBuffer,ULONG_PTR uOutSize)
{
	NTSTATUS status;
	HANDLE ProcessHandle = NULL;
	CLIENT_ID Cid = {0};
	OBJECT_ATTRIBUTES oa = {0};
	PEPROCESS EProcess = NULL;
	ULONG_PTR i = 0;
	PROCESS_BASIC_INFORMATION pbi;
	PVOID Buffer = NULL;
	ULONG RetusnSize;
	KAPC_STATE ApcState;
	ULONG_PTR  Peb = 0;
	ULONG_PTR  ulProcessParamters = 0;
	ULONG_PTR ulProcessNameLen = 0;
	ULONG_PTR ulCnt = (uOutSize-sizeof(PROCESS_INFORMATION_OWN))/sizeof(PROCESS_INFORMATION_ENTRY);
	DbgPrint("2");
	for(i=0;i<0x1000;i+=4)
	{
		DbgPrint("3");
		Cid.UniqueProcess = (HANDLE)i;
		Cid.UniqueThread = 0;
		status = ZwOpenProcess(&ProcessHandle,GENERIC_ALL,&oa,&Cid);
		if(NT_SUCCESS(status))
		{
			ObReferenceObjectByHandle(ProcessHandle,
				GENERIC_ALL,
				NULL,
				KernelMode,
				&EProcess,
				NULL);
			if(!IsProcessDie(EProcess))
			{
				ULONG_PTR ulCurrentCnt = ((PPROCESS_INFORMATION_OWN)OutputBuffer)->NumberOfEntry;
				((PPROCESS_INFORMATION_OWN)OutputBuffer)->Entry[ulCurrentCnt].Eprocess = (ULONG_PTR)EProcess;
				((PPROCESS_INFORMATION_OWN)OutputBuffer)->Entry[ulCurrentCnt].Pid =  *(ULONG*)((ULONG_PTR)EProcess + ProcessIdOfEprocess);
				ulProcessNameLen = strlen((const char*)PsGetProcessImageFileName(EProcess));
				//通过EProcess获得进程名称
				memcpy(((PPROCESS_INFORMATION_OWN)OutputBuffer)->Entry[ulCurrentCnt].ProcessName,(const char*)PsGetProcessImageFileName(EProcess),ulProcessNameLen);  
			
				((PPROCESS_INFORMATION_OWN)OutputBuffer)->Entry[ulCurrentCnt].ParentId =  *(ULONG*)((ULONG_PTR)EProcess + FatherOfEprocess);
				DbgPrint("ZwOpenProcess ID:%d  %s\r\n",*((ULONG_PTR*)((ULONG_PTR)EProcess+ProcessIdOfEprocess)),(char*)((ULONG_PTR)EProcess+ProcessImageNameOfEprocess));
			
				GetProcessPathBySectionObject(EProcess,((PPROCESS_INFORMATION_OWN)OutputBuffer)->Entry[ulCurrentCnt].ProcessPath);
				((PPROCESS_INFORMATION_OWN)OutputBuffer)->NumberOfEntry++;
			}
			ObDereferenceObject(EProcess);   //解引用
			ZwClose(ProcessHandle);
		}
		memset(&oa,0,sizeof(OBJECT_ATTRIBUTES));
	}
	return STATUS_SUCCESS;
}


NTSTATUS EnumProcessByActiveList(PPROCESS_INFORMATION_OWN OutputBuffer,ULONG_PTR OutSize)
{

	NTSTATUS Status = STATUS_UNSUCCESSFUL;

	PLIST_ENTRY ListHead = NULL;
	PLIST_ENTRY ListTemp = NULL;
	PETHREAD EThread = NULL;
	CHAR PreMode;
	ULONG_PTR ulCnt = (OutSize - sizeof(PROCESS_INFORMATION_OWN)) / sizeof(PROCESS_INFORMATION_ENTRY);
	PEPROCESS EProcess = g_EProcess;
	KAPC_STATE ApcState;
	PPEB  Peb = NULL;
	ULONG_PTR  ulProcessParamters = 0;

	if(EProcess!=NULL)
	{
		ListHead = ListTemp = (PLIST_ENTRY)((ULONG_PTR)EProcess + Active_List);  //system.exe
		ListHead = ListHead->Blink;

		while (ListTemp!=ListHead)
		{
			ULONG_PTR  ulCurrentCnt = OutputBuffer->NumberOfEntry;
			EProcess = (PEPROCESS)((ULONG_PTR)ListTemp-Active_List);
			((PPROCESS_INFORMATION_OWN)OutputBuffer)->Entry[ulCurrentCnt].Eprocess = (ULONG_PTR)EProcess;
			DbgPrint("ActiveList ID:%d  %s\r\n",*((ULONG_PTR*)((ULONG_PTR)EProcess+ProcessIdOfEprocess)),(char*)((ULONG_PTR)EProcess+ProcessImageNameOfEprocess));
			OutputBuffer->Entry[ulCurrentCnt].Pid = *((ULONG*)((ULONG_PTR)EProcess+ProcessIdOfEprocess));
			strcpy(OutputBuffer->Entry[ulCurrentCnt].ProcessName,(char*)((ULONG_PTR)EProcess + ProcessImageNameOfEprocess));

			((PPROCESS_INFORMATION_OWN)OutputBuffer)->Entry[ulCurrentCnt].ParentId =  *(ULONG*)((ULONG_PTR)EProcess + FatherOfEprocess);
			GetProcessPathBySectionObject(EProcess,((PPROCESS_INFORMATION_OWN)OutputBuffer)->Entry[ulCurrentCnt].ProcessPath);
			/*
			Peb = PsGetProcessPeb(EProcess);     //获得PEB
			if (Peb==NULL) 
			{
				return FALSE;
			}
			KeStackAttachProcess(EProcess, &ApcState);

			ulProcessParamters = *(ULONG_PTR*)((ULONG_PTR)Peb+ulProcessParametersOfPeb);//0x20

			memcpy(((PPROCESS_INFORMATION_OWN)OutputBuffer)->Entry[ulCurCnt].ProcessPath,((PUNICODE_STRING)(ulProcessParamters+ulImagePathNameOfProcessParameters))->Buffer,
				((PUNICODE_STRING)(ulProcessParamters+ulImagePathNameOfProcessParameters))->Length);//0x60
			KeUnstackDetachProcess(&ApcState);*/

			ListTemp = ListTemp->Flink;
			OutputBuffer->NumberOfEntry++;
		}
	}
	return STATUS_SUCCESS;
}




NTSTATUS EnumProcessByPspCidTable(PPROCESS_INFORMATION_OWN OutputBuffer,ULONG_PTR OutSize)
{
	NTSTATUS Status;
	
	ULONG_PTR ulCnt = (OutSize - sizeof(PROCESS_INFORMATION_OWN)) / sizeof(PROCESS_INFORMATION_ENTRY);
	
	ScanHandleTableToEnumProcess(OutputBuffer, ulCnt);
	if (ulCnt >= OutputBuffer->NumberOfEntry)
	{
		Status = STATUS_SUCCESS;
	}
	else
	{
		Status = STATUS_BUFFER_TOO_SMALL;
	}
	return Status;
}



ULONG_PTR GetPspCidTableAddress()  
{  
	PVOID PsLookupProcessByProcessIdAddress = NULL;  
	ULONG_PTR ulPspCidTableValue = 0;  
	UNICODE_STRING uniFuncName; 
	ULONG  uIndex = 0;
	int    Offset = 0;

	// 获取PsLookupProcessByProcessId的函数地址   
	RtlInitUnicodeString(&uniFuncName, L"PsLookupProcessByProcessId");    //从Ntos导出表中获得函数PsLookupProcessByProcessId地址
	PsLookupProcessByProcessIdAddress = MmGetSystemRoutineAddress(&uniFuncName);  
	if (PsLookupProcessByProcessIdAddress== NULL )  
	{  
		return ulPspCidTableValue;  
	}  
	DbgPrint("PsLookupProcessByProcessId->%08X",PsLookupProcessByProcessIdAddress);  

	switch(WinVersion)
	{
	case WINDOWS_7:
		{
			
			/*
			kd> u PsLookupProcessByProcessId l 20
			nt!PsLookupProcessByProcessId:
			fffff800`041a61fc 48895c2408      mov     qword ptr [rsp+8],rbx
			fffff800`041a6201 48896c2410      mov     qword ptr [rsp+10h],rbp
			fffff800`041a6206 4889742418      mov     qword ptr [rsp+18h],rsi
			fffff800`041a620b 57              push    rdi
			fffff800`041a620c 4154            push    r12
			fffff800`041a620e 4155            push    r13
			fffff800`041a6210 4883ec20        sub     rsp,20h
			fffff800`041a6214 65488b3c2588010000 mov   rdi,qword ptr gs:[188h]
			fffff800`041a621d 4533e4          xor     r12d,r12d
			fffff800`041a6220 488bea          mov     rbp,rdx
			fffff800`041a6223 66ff8fc4010000  dec     word ptr [rdi+1C4h]
			fffff800`041a622a 498bdc          mov     rbx,r12
			fffff800`041a622d 488bd1          mov     rdx,rcx
			fffff800`041a6230 488b0d9149edff  mov     rcx,qword ptr [nt!PspCidTable (fffff800`0407abc8)]
			fffff800`041a6237 e834480200      call    nt!ExMapHandleToPointer (fffff800`041caa70)
			*/
			for (uIndex=0;uIndex<0x1000;uIndex++ )  
			{  
				if (*((PUCHAR)((ULONG_PTR)PsLookupProcessByProcessIdAddress+ uIndex)) == 0x48 &&  
					*((PUCHAR)((ULONG_PTR)PsLookupProcessByProcessIdAddress+ uIndex + 1) ) == 0x8B &&  
					*((PUCHAR)((ULONG_PTR)PsLookupProcessByProcessIdAddress+ uIndex + 7) ) == 0xE8 )  
				{  
			
					memcpy(&Offset,(PUCHAR)((ULONG_PTR)PsLookupProcessByProcessIdAddress+ uIndex + 3),4);
					ulPspCidTableValue = (ULONG_PTR)PsLookupProcessByProcessIdAddress+uIndex+Offset+7; 

					DbgPrint("Found OK!!\r\n");
					break;  
				}  
			}  
			break;
		}

	case WINDOWS_XP:
		{
			/*
			kd> u PsLookupProcessByProcessId l 20
			nt!PsLookupProcessByProcessId:
			80582687 8bff            mov     edi,edi
			80582689 55              push    ebp
			8058268a 8bec            mov     ebp,esp
			8058268c 53              push    ebx
			8058268d 56              push    esi
			8058268e 64a124010000    mov     eax,dword ptr fs:[00000124h]
			80582694 ff7508          push    dword ptr [ebp+8]
			80582697 8bf0            mov     esi,eax
			80582699 ff8ed4000000    dec     dword ptr [esi+0D4h]
			8058269f ff3560a75680    push    dword ptr [nt!PspCidTable (8056a760)]

			*/
	    	for (uIndex = 0; uIndex < 0x1000; uIndex++ )  
			{  
				if ( *( (PUCHAR)((ULONG_PTR)PsLookupProcessByProcessIdAddress+ uIndex) ) == 0xFF &&  
					*( (PUCHAR)((ULONG_PTR)PsLookupProcessByProcessIdAddress+ uIndex + 1) ) == 0x35 &&  
					*( (PUCHAR)((ULONG_PTR)PsLookupProcessByProcessIdAddress+ uIndex + 6) ) == 0xE8 )  
				{  
					DbgPrint("Found OK!!\r\n");  
					ulPspCidTableValue = *((PULONG)((ULONG)PsLookupProcessByProcessIdAddress+ uIndex + 2) );  
					break;  
				}  
			}  
			break;
		}
	}
	

	return ulPspCidTableValue;  
}  



VOID ScanHandleTableToEnumProcess(PPROCESS_INFORMATION_OWN OutputBuffer, ULONG_PTR ulCnt)
{
	PHANDLE_TABLE   HandleTable = NULL;    // 指向句柄表的指针   
	ULONG_PTR uTableCode = 0;  
	ULONG uFlag = 0;


	HandleTable = (PHANDLE_TABLE)(*(ULONG_PTR*)PspCidTable);  

	if (HandleTable && MmIsAddressValid((PVOID)HandleTable))
	{
		uTableCode = (ULONG_PTR)(HandleTable->TableCode) & 0xFFFFFFFFFFFFFFFC;  ;
		if (uTableCode && MmIsAddressValid((PVOID)uTableCode))
		{
			uFlag = (ULONG)(HandleTable->TableCode) & 0x03;    //00  01  10  

			switch (uFlag)
			{
			case 0:
				{
					EnumTable1(uTableCode,OutputBuffer, ulCnt);
					break;
				}
			case 1:
				{
					EnumTable2(uTableCode,OutputBuffer, ulCnt);
					break;
				}


			case 2:
				{
					EnumTable3(uTableCode,OutputBuffer, ulCnt);
					break; 
				}


			default:
				KdPrint(("TableCode error\n"));
			} 			
		}
	}
}




//uTableCode  已经清了最后两位
NTSTATUS EnumTable1(ULONG_PTR uTableCode,PPROCESS_INFORMATION_OWN OutputBuffer, ULONG_PTR ulCnt)
{


	PVOID  Object = NULL;
	PHANDLE_TABLE_ENTRY HandleTableEntry = NULL;  
	ULONG uIndex = 0;
	ULONG_PTR ulOffset = 0;
	switch(WinVersion)
	{
	case WINDOWS_XP:
		{
			ulOffset = 0x8;
			break;
		}
	case WINDOWS_7:
		{
			ulOffset = 0x10;
			break;
		}
	}
	HandleTableEntry = (PHANDLE_TABLE_ENTRY)((ULONG_PTR)(*(ULONG_PTR*)uTableCode) + ulOffset); //xp offset 0x08  
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

						Object = (PVOID)(((ULONG_PTR)HandleTableEntry->Object)  & 0xFFFFFFFFFFFFFFF8);  
						InsertProcess((PEPROCESS)Object,OutputBuffer, ulCnt);

					}

				}
			}

		}

		HandleTableEntry++;  

	}  

	return STATUS_SUCCESS;
}



NTSTATUS EnumTable2(ULONG_PTR uTableCode,PPROCESS_INFORMATION_OWN OutputBuffer, ULONG_PTR ulCnt)
{
	do   
	{  
		DbgPrint("Two");
		EnumTable1(uTableCode,OutputBuffer,ulCnt);  
		uTableCode += sizeof(ULONG_PTR);  
	} while (*(PULONG_PTR)uTableCode != 0&&MmIsAddressValid((PVOID)*(PULONG_PTR)uTableCode));  

	return STATUS_SUCCESS;
}



NTSTATUS EnumTable3(ULONG_PTR uTableCode, PPROCESS_INFORMATION_OWN OutputBuffer, ULONG_PTR ulCnt)
{
	do   
	{  
		EnumTable2(uTableCode,OutputBuffer,ulCnt);  
		uTableCode += sizeof(ULONG_PTR);  
	} while (*(PULONG_PTR)uTableCode != 0);  

	return STATUS_SUCCESS;  
}




VOID InsertProcess(PEPROCESS EProcess, PPROCESS_INFORMATION_OWN OutputBuffer, ULONG_PTR ulCnt)
{
	KAPC_STATE ApcState;
	PPEB  Peb = NULL;
	ULONG_PTR  ulProcessParamters = 0;

	if (EProcess && MmIsAddressValid((PVOID)EProcess) && KeGetObjectType((PVOID)EProcess) == (ULONG_PTR)*PsProcessType)
	{ 

		if (!IsProcessDie(EProcess)&&
			NT_SUCCESS(ObReferenceObjectByPointer(EProcess, 0, NULL, KernelMode)))  //因为要操作该对象所以增加引用计数
		{
			ULONG_PTR ulCurrentCnt = OutputBuffer->NumberOfEntry;
			if (ulCnt > ulCurrentCnt)
			{
				((PPROCESS_INFORMATION_OWN)OutputBuffer)->Entry[ulCurrentCnt].Eprocess = (ULONG_PTR)EProcess;
				OutputBuffer->Entry[ulCurrentCnt].Pid = *(ULONG*)((ULONG_PTR)EProcess+ProcessIdOfEprocess);
				strcpy(OutputBuffer->Entry[ulCurrentCnt].ProcessName,(char*)((ULONG_PTR)EProcess + ProcessImageNameOfEprocess));
					((PPROCESS_INFORMATION_OWN)OutputBuffer)->Entry[ulCurrentCnt].ParentId =  *(ULONG*)((ULONG_PTR)EProcess + FatherOfEprocess);
			DbgPrint("PspCidTable ID:%d  %s\r\n",*((ULONG*)((ULONG_PTR)EProcess+ProcessIdOfEprocess)),(char*)((ULONG_PTR)EProcess+ProcessImageNameOfEprocess));
				GetProcessPathBySectionObject(EProcess,((PPROCESS_INFORMATION_OWN)OutputBuffer)->Entry[ulCurrentCnt].ProcessPath);	
			/*	Peb = PsGetProcessPeb(EProcess);     //获得PEB
				if (Peb==NULL) 
				{
					return FALSE;
				}
				KeStackAttachProcess(EProcess, &ApcState);

				ulProcessParamters = *(ULONG_PTR*)((ULONG_PTR)Peb+ulProcessParametersOfPeb);//0x20

				memcpy(((PPROCESS_INFORMATION_OWN)OutputBuffer)->Entry[ulCurCnt].ProcessPath,((PUNICODE_STRING)(ulProcessParamters+ulImagePathNameOfProcessParameters))->Buffer,
					((PUNICODE_STRING)(ulProcessParamters+ulImagePathNameOfProcessParameters))->Length);//0x60
				KeUnstackDetachProcess(&ApcState);
				*/
				
			}

			OutputBuffer->NumberOfEntry++;
			ObfDereferenceObject(EProcess);
		}
	} 
}



BOOLEAN  GetProcessPathBySectionObject(PEPROCESS EProcess,WCHAR* wzProcessPath)
{
	PSECTION_OBJECT32   SectionObject32   = NULL;
	PSECTION_OBJECT64   SectionObject64 = NULL;
	PSEGMENT_OBJECT32   Segment32   = NULL;
	PSEGMENT_OBJECT64   Segment64   = NULL;
	PCONTROL_AREA32     ControlArea32 = NULL;
	PCONTROL_AREA64     ControlArea64 = NULL;
	PFILE_OBJECT        FileObject  = NULL;
	BOOLEAN             bGetPath = FALSE;
	switch(WinVersion)
	{
	case WINDOWS_XP:
		{
			SectionObjectOfEProcess = 0x138;
			if (SectionObjectOfEProcess!=0&&MmIsAddressValid((PVOID)((ULONG_PTR)EProcess + SectionObjectOfEProcess)))
			{
				SectionObject32 = *(PSECTION_OBJECT32*)((ULONG_PTR)EProcess + SectionObjectOfEProcess);
				if (SectionObject32 && MmIsAddressValid(SectionObject32))
				{
					Segment32 = (PSEGMENT_OBJECT32)(SectionObject32->Segment);
					if (Segment32 && MmIsAddressValid(Segment32))
					{
						ControlArea32 = (PCONTROL_AREA32)(Segment32->ControlArea);
						if (ControlArea32 && MmIsAddressValid(ControlArea32))
						{
							FileObject = ControlArea32->FilePointer;
							if (FileObject&&MmIsAddressValid(FileObject))
							{
								bGetPath = GetPathByFileObject(FileObject, wzProcessPath);
								if (!bGetPath)
								{
									DbgPrint("Error\r\n");
									return FALSE;
								}
								else
									return TRUE;
							}
						}
					}
				}
			}
			break;
		}
	case WINDOWS_7:
		{
			SectionObjectOfEProcess = 0x268;
			if (SectionObjectOfEProcess!=0&&MmIsAddressValid((PVOID)((ULONG_PTR)EProcess + SectionObjectOfEProcess)))
			{
				SectionObject64 = *(PSECTION_OBJECT64*)((ULONG_PTR)EProcess + SectionObjectOfEProcess);

				if (SectionObject64 && MmIsAddressValid(SectionObject64))
				{
					Segment64 = (PSEGMENT_OBJECT64)(SectionObject64->Segment);
					if (Segment64 && MmIsAddressValid(Segment64))
					{
						ControlArea64 = (PCONTROL_AREA64)Segment64->ControlArea;
						if (ControlArea64 && MmIsAddressValid(ControlArea64))
						{
							FileObject = (PFILE_OBJECT)ControlArea64->FilePointer;
							if (FileObject&&MmIsAddressValid(FileObject))
							{
								FileObject = (PFILE_OBJECT)((ULONG_PTR)FileObject & 0xFFFFFFFFFFFFFFF0);
								bGetPath = GetPathByFileObject(FileObject, wzProcessPath);
								if (!bGetPath)
								{
									DbgPrint("Error\r\n");
									return FALSE;
								}
								else
									return TRUE;
							}
						}


					}

				}

			}
			break;
		}
	}
	return TRUE;
}
BOOLEAN GetPathByFileObject(PFILE_OBJECT FileObject, WCHAR* wzPath)
{
	BOOLEAN bGetPath = FALSE;
	POBJECT_NAME_INFORMATION ObjectNameInformation = NULL;
	__try
	{
		if (FileObject && MmIsAddressValid(FileObject) && wzPath)
		{
			if (NT_SUCCESS(IoQueryFileDosDeviceName(FileObject,&ObjectNameInformation)))   //注意该函数调用后要释放内存
			{
				wcsncpy(wzPath,ObjectNameInformation->Name.Buffer,ObjectNameInformation->Name.Length);
				bGetPath = TRUE;
				ExFreePool(ObjectNameInformation);
			}
			if (!bGetPath)
			{
				if (IoVolumeDeviceToDosName||RtlVolumeDeviceToDosName)
				{
					NTSTATUS  Status = STATUS_UNSUCCESSFUL;
					ULONG ulRet= 0;
					PVOID     Buffer = ExAllocatePool(PagedPool,0x1000);
					if (Buffer)
					{
						// ObQueryNameString :C:\Program Files\VMware\VMware Tools\VMwareTray.exe
						memset(Buffer, 0, 0x1000);
						Status = ObQueryNameString((PVOID)FileObject, (POBJECT_NAME_INFORMATION)Buffer, 0x1000, &ulRet);
						if (NT_SUCCESS(Status))
						{
							POBJECT_NAME_INFORMATION Temp = (POBJECT_NAME_INFORMATION)Buffer;
							WCHAR szHarddiskVolume[100] = L"\\Device\\HarddiskVolume";
							if (Temp->Name.Buffer!=NULL)
							{
								if (Temp->Name.Length / sizeof(WCHAR) > wcslen(szHarddiskVolume) &&
									!_wcsnicmp(Temp->Name.Buffer, szHarddiskVolume, wcslen(szHarddiskVolume)))
								{
									// 如果是以 "\\Device\\HarddiskVolume" 这样的形式存在的，那么再查询其卷名。
									UNICODE_STRING uniDosName;
									if (NT_SUCCESS(IoVolumeDeviceToDosName(FileObject->DeviceObject, &uniDosName)))
									{
										if (uniDosName.Buffer!=NULL)
										{
											wcsncpy(wzPath, uniDosName.Buffer, uniDosName.Length);
											wcsncat(wzPath, Temp->Name.Buffer + wcslen(szHarddiskVolume) + 1, Temp->Name.Length - (wcslen(szHarddiskVolume) + 1));
											bGetPath = TRUE;
										}    
										ExFreePool(uniDosName.Buffer);
									}
									else if (NT_SUCCESS(RtlVolumeDeviceToDosName(FileObject->DeviceObject, &uniDosName)))
									{
										if (uniDosName.Buffer!=NULL)
										{
											wcsncpy(wzPath, uniDosName.Buffer, uniDosName.Length);
											wcsncat(wzPath, Temp->Name.Buffer + wcslen(szHarddiskVolume) + 1, Temp->Name.Length - (wcslen(szHarddiskVolume) + 1));
											bGetPath = TRUE;
										}    
										ExFreePool(uniDosName.Buffer);
									}
								}
								else
								{
									// 如果不是以 "\\Device\\HarddiskVolume" 这样的形式开头的，那么直接复制名称。
									wcsncpy(wzPath, Temp->Name.Buffer, Temp->Name.Length);
									bGetPath = TRUE;
								}
							}
						}
						ExFreePool(Buffer);
					}
				}
			}
		}
	}
	__except(1)
	{
		DbgPrint("GetPathByFileObject Catch __Except\r\n");
		bGetPath = FALSE;
	}
	return bGetPath;
}
