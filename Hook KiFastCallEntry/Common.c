#include "Common.h"


PFUNCDATA FuncData = NULL;
PKEVENT  EventFuncMonitor = NULL; 

MSG         Msg = {0}; 
PKEVENT     EventArray[3] = {0}; 

ERESOURCE  Resource;
KMUTEX  Mutex;

BOOLEAN         IsClear = FALSE;//资源是否销毁

WIN_VERSION WinVersion = WINDOWS_UNKNOW;

ULONG_PTR SectionObjectOfEProcess = 0;

WIN_VERSION GetWindowsVersion()
{
	RTL_OSVERSIONINFOEXW osverInfo = {sizeof(osverInfo)}; 
	pfnRtlGetVersion RtlGetVersion = NULL;
	WIN_VERSION WinVersion;
	WCHAR wzRtlGetVersion[] = L"RtlGetVersion";
	RtlGetVersion = GetFunctionAddressByName(wzRtlGetVersion);    //Ntoskrnl.exe  导出表
	if (RtlGetVersion)
	{
		RtlGetVersion((PRTL_OSVERSIONINFOW)&osverInfo); 
	} 
	else 
	{
		PsGetVersion(&osverInfo.dwMajorVersion, &osverInfo.dwMinorVersion, &osverInfo.dwBuildNumber, NULL);   //Documet
	}
	DbgPrint("Build Number: %d\r\n", osverInfo.dwBuildNumber);
	if (osverInfo.dwMajorVersion == 5 && osverInfo.dwMinorVersion == 1) 
	{
		DbgPrint("WINDOWS_XP\r\n");
		WinVersion = WINDOWS_XP;
	}
	else if (osverInfo.dwMajorVersion == 6 && osverInfo.dwMinorVersion == 1)
	{
		DbgPrint("WINDOWS 7\r\n");
		WinVersion = WINDOWS_7;
	}
	else if (osverInfo.dwMajorVersion == 6 && 
		osverInfo.dwMinorVersion == 2 &&
		osverInfo.dwBuildNumber == 9200)
	{
		DbgPrint("WINDOWS 8\r\n");
		WinVersion = WINDOWS_8;
	}
	else if (osverInfo.dwMajorVersion == 6 && 
		osverInfo.dwMinorVersion == 3 && 
		osverInfo.dwBuildNumber == 9600)
	{
		DbgPrint("WINDOWS 8.1\r\n");
		WinVersion = WINDOWS_8_1;
	}
	else
	{
		DbgPrint("WINDOWS_UNKNOW\r\n");
		WinVersion = WINDOWS_UNKNOW;
	}
	return WinVersion;
}



VOID InitGlobalVariable()
{
	WinVersion = GetWindowsVersion();
	switch(WinVersion)
	{
	case WINDOWS_XP:
		{

			break;
		}
	case WINDOWS_7:
		{
			
			break;

		}
	}
}

PVOID 
	GetFunctionAddressByName(WCHAR *wzFunction)
{
	UNICODE_STRING uniFunction;  
	PVOID AddrBase = NULL;
	if (wzFunction && wcslen(wzFunction) > 0)
	{
		RtlInitUnicodeString(&uniFunction, wzFunction);      //常量指针
		AddrBase = MmGetSystemRoutineAddress(&uniFunction);  //在System 进程  第一个模块  Ntosknrl.exe  ExportTable
	}
	return AddrBase;
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
								}
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
								}
							}
						}


					}

				}

			}
			break;
		}
	}

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
					ULONG_PTR ulRet= 0;
					PVOID     Buffer = ExAllocatePool(PagedPool,0x1000);
					if (Buffer)
					{
						// ObQueryNameString :C:\Program Files\VMware\VMware Tools\VMwareTray.exe
						memset(Buffer, 0, 0x1000);
						Status = ObQueryNameString(FileObject, (POBJECT_NAME_INFORMATION)Buffer, 0x1000, &ulRet);
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






KIRQL 
	SecCloseProtect()
{
	KIRQL Irql;

	Irql = KeRaiseIrqlToDpcLevel();
#ifdef _WIN64
#else
	_asm
	{
		CLI;					//	禁止中断
		MOV EAX,CR0;  
		AND EAX,NOT 10000H;		// CR0 16 BIT = 0
		MOV CR0, EAX;        
	}	
#endif
	return Irql;
}


VOID 
	SecOpenProtect(KIRQL Irql)
{
#ifdef _WIN64
#else
	_asm 
	{
		MOV EAX,CR0;     
		OR EAX,10000H;            
		MOV CR0,EAX;           
		STI;					//	开启中断
	}
#endif

	KeLowerIrql(Irql);
}



PVOID __stdcall _CreateFileMap(PCWSTR SourceString, int a2, int a3)
{
	SECTION_IMAGE_INFORMATION SectionInformation; // [sp+8h] [bp-68h]@3
	OBJECT_ATTRIBUTES ObjectAttributes; // [sp+38h] [bp-38h]@1
	UNICODE_STRING DestinationString; // [sp+50h] [bp-20h]@1
	struct _IO_STATUS_BLOCK IoStatusBlock; // [sp+58h] [bp-18h]@1
	PVOID BaseAddress; // [sp+60h] [bp-10h]@1
	ULONG_PTR ViewSize; // [sp+64h] [bp-Ch]@1
	HANDLE FileHandle; // [sp+68h] [bp-8h]@1
	HANDLE SectionHandle; // [sp+6Ch] [bp-4h]@1

	IoStatusBlock.Status = 0;
	IoStatusBlock.Information = 0;
	BaseAddress = 0;
	FileHandle = 0;
	SectionHandle = 0;
	ViewSize = 0;
	RtlInitUnicodeString(&DestinationString, SourceString);
	ObjectAttributes.ObjectName = &DestinationString;
	ObjectAttributes.Length = 24;
	ObjectAttributes.RootDirectory = 0;
	ObjectAttributes.Attributes = 576;
	ObjectAttributes.SecurityDescriptor = 0;
	ObjectAttributes.SecurityQualityOfService = 0;
	if ( ZwOpenFile(&FileHandle, FILE_SYNCHRONOUS_IO_NONALERT|FILE_RESERVE_OPFILTER|MEM_4MB_PAGES, &ObjectAttributes, 
		&IoStatusBlock, FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE, 
		FILE_ATTRIBUTE_DEVICE|FILE_ATTRIBUTE_ARCHIVE) >= 0 )
	{//0x80100020    0x7  0x60
		ObjectAttributes.ObjectName = 0;
		//0xD 2 IMAGE_SECMENT = 0x100 0000
		if ( ZwCreateSection(&SectionHandle, SECTION_MAP_EXECUTE|SECTION_MAP_READ|SECTION_MAP_WRITE, &ObjectAttributes,
			0, PAGE_READONLY, 0x1000000u, FileHandle) >= 0
			&& ZwQuerySection(SectionHandle, SectionImageInformation, &SectionInformation, 0x30u, 0) >= 0 )
		{
			if ( a2 )
				*(int*)a2 = SectionInformation.ImageFileSize;
			ZwMapViewOfSection(SectionHandle, (HANDLE)0xFFFFFFFF, &BaseAddress, 0, 0, 0, &ViewSize, ViewShare, 0, 0x20u);
			if ( a3 )
				*(int*)a3 = ViewSize;
		}
	}
	if ( FileHandle )
		ZwClose(FileHandle);
	if ( SectionHandle )
		ZwClose(SectionHandle);
	return BaseAddress;
}