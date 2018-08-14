
#ifndef CXX_MODULECALLBACK_H
#    include "ModuleCallBack.h"
#endif
#include "struct.h"

IMAGE_DOS_HEADER* pDosHeader;
IMAGE_OPTIONAL_HEADER * pOptHeader;
IMAGE_IMPORT_DESCRIPTOR * pImportDesc;
HANDLE  hanSection = NULL;
HANDLE  hFile = NULL;
PVOID MapFileBaseAddress = NULL;


NTSTATUS DriverEntry(IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING RegisterPath)
{
    DriverObject->DriverUnload = UnloadDriver;
    PsSetLoadImageNotifyRoutine((PLOAD_IMAGE_NOTIFY_ROUTINE)LoadImageNotifyRoutine);

    return STATUS_SUCCESS;
}

VOID LoadImageNotifyRoutine(PUNICODE_STRING FullImageName,HANDLE ProcessId,PIMAGE_INFO ImageInfor)
{
    PEPROCESS Process = NULL;
    CHAR  szProcessPath[512] = {0};
    NTSTATUS  Status = STATUS_SUCCESS;
    ULONG ulProcessNameLen = 0;
    HANDLE Thread = NULL;
    WCHAR wzPath[260] = {0};
    PFILE_OBJECT FileObject;

    if(ProcessId == 0 || ProcessId == (HANDLE)4)
        return ;

    if( FullImageName != NULL && MmIsAddressValid(FullImageName))
    {
        
        Status = PsLookupProcessByProcessId((HANDLE)ProcessId,&Process);
        if(!NT_SUCCESS(Status))
            return;

        //通过EProcess获得进程名称
        ulProcessNameLen = strlen((const char*)PsGetProcessImageFileName(Process));
        memcpy(szProcessPath,(const char*)PsGetProcessImageFileName(Process),ulProcessNameLen);     
        DbgPrint("%s\r\n",szProcessPath);

        //strcmp(szProcessPath,"Inject.exe")==0&&
        if(wcsstr(FullImageName->Buffer,L"Dll.dll") != NULL)
        {
            FileObject = CONTAINING_RECORD(FullImageName,FILE_OBJECT,FileName);

            if(!GetPathByFileObject(FileObject, wzPath))
                goto __END;

            Status = PsCreateSystemThread (&Thread,
                THREAD_ALL_ACCESS,
                NULL,
                NULL,
                NULL,
                MyProcessThread,
                wzPath);

            //进程信息拷贝到结构体中
            //在导入表中查找到记录函数则给于r3提示
        }
    }

__END:

    if(Process)
    {
        ObDereferenceObject(Process);
        Process = NULL;
    }
    return;
}



BOOLEAN GetPathByFileObject(PFILE_OBJECT FileObject, WCHAR* wzPath)
{
    BOOLEAN bGetPath = FALSE;
    POBJECT_NAME_INFORMATION ObjectNameInformation1 = NULL;
    __try
    {
        if (FileObject && MmIsAddressValid(FileObject) && wzPath)
        {
            if (NT_SUCCESS(IoQueryFileDosDeviceName(FileObject,&ObjectNameInformation1)))   //注意该函数调用后要释放内存
            {
                wcsncpy(wzPath,ObjectNameInformation1->Name.Buffer,ObjectNameInformation1->Name.Length);
                bGetPath = TRUE;
                ExFreePool(ObjectNameInformation1);
            }
            if (!bGetPath)
            {
                if (IoVolumeDeviceToDosName||RtlVolumeDeviceToDosName)
                {
                    NTSTATUS  Status = STATUS_UNSUCCESSFUL;
                    ULONG ulRet= 0;
                    POBJECT_NAME_INFORMATION Buffer = (POBJECT_NAME_INFORMATION)ExAllocatePool(PagedPool,0x1000);
                    if (Buffer)
                    {
                        // ObQueryNameString :C:\Program Files\VMware\VMware Tools\VMwareTray.exe
                        memset(Buffer, 0, 0x1000);
                        Status = ObQueryNameString(FileObject, Buffer, 0x1000, &ulRet);
                        if (NT_SUCCESS(Status))
                        {
                            POBJECT_NAME_INFORMATION Temp = Buffer;
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


void MyProcessThread(PVOID pContext)
{
    NTSTATUS Status;
    WCHAR* wzPath = (WCHAR*)pContext;
    //L"\\??\\C:\\Documents and Settings\\Administrator\\桌面\\Dll.dll"
    UNICODE_STRING unicode;
    DbgPrint("First:%d",PsGetCurrentProcessId());
    RtlInitUnicodeString(&unicode,wzPath);
    EnumIATByFileMapping(&unicode);
    PsTerminateSystemThread(STATUS_SUCCESS);
}

//ReadFile的方式，需要转为文件偏移
BOOLEAN  EnumImportTable(PUNICODE_STRING uniDestFileFullPath)
{
    PIMAGE_DOS_HEADER  DosHead  = NULL;
    PIMAGE_NT_HEADERS  NtHead   = NULL;
    PIMAGE_FILE_HEADER FileHead = NULL;
    PIMAGE_OPTIONAL_HEADER  OptionHead = NULL;
    IMAGE_DATA_DIRECTORY      ImportDirectory = {0};   //选项头中的结构体数组中的数据类型
    PIMAGE_IMPORT_DESCRIPTOR  ImportTable  = NULL;     
    PIMAGE_THUNK_DATA Thunk;
    char *szDllNam;
    HANDLE hFile = NULL;
    PVOID Buffer = NULL;
    ULONG ulLength = 0;
    LARGE_INTEGER Offset = {0};
    IO_STATUS_BLOCK  Iosb;
    OBJECT_ATTRIBUTES  oa;
    NTSTATUS Status = STATUS_SUCCESS;
    FILE_STANDARD_INFORMATION   FileStandardInfor = {0};

    InitializeObjectAttributes(&oa,uniDestFileFullPath,OBJ_CASE_INSENSITIVE,NULL,NULL);

//     //打开文件,返回文件句柄给hFile;
//     Status = ZwCreateFile(&hFile,GENERIC_READ,&oa,&Iosb,NULL,
//             FILE_ATTRIBUTE_NORMAL,
//             FILE_SHARE_READ|FILE_SHARE_WRITE,
//             FILE_OPEN,
//             FILE_SYNCHRONOUS_IO_NONALERT,
//             NULL,
//             0);
// 
//     if(!NT_SUCCESS(Status))
//     {
//         return FALSE;
//     }
// 
//     Status = NtOpenFile(&hFile,GENERIC_READ|GENERIC_WRITE,&oa,&Iosb,
//         FILE_SHARE_READ|FILE_SHARE_WRITE,FILE_NON_DIRECTORY_FILE);
// 
//     if(!NT_SUCCESS(Status))
//     {
//         return FALSE;
//     }

    Status = IoCreateFile(&hFile,
        GENERIC_READ,
        &oa,
        &Iosb,
        NULL,
        0,
        FILE_SHARE_READ,
        FILE_OPEN_IF,
        FILE_SYNCHRONOUS_IO_NONALERT,
        NULL,
        0,
        CreateFileTypeNone,
        NULL,
        IO_NO_PARAMETER_CHECKING
        );
        
    if (!NT_SUCCESS(Status))
    {
        return FALSE;
    }


    //获得文件信息;
    //参数列表（文件句柄，io栈锁，【out】文件信息,文件大小，文件信息类？）;
    Status = ZwQueryInformationFile(hFile, &Iosb, &FileStandardInfor, sizeof(FILE_STANDARD_INFORMATION),
        FileStandardInformation);

    //获取文件信息失败就直接返回;
    if (!NT_SUCCESS(Status))
    {
        ZwClose(hFile);
        return FALSE;
    }

    //文件信息有误，清除句柄,直接返回;
    if (FileStandardInfor.AllocationSize.u.LowPart == 0)
    {
        ZwClose(hFile);
        return FALSE;
    }

    //根据文件大小，申请空间;
    Buffer = (char*)ExAllocatePool(PagedPool,FileStandardInfor.AllocationSize.u.LowPart);

    //申请失败，重复失败步骤;
    if (Buffer==NULL)
    {
        ZwClose(hFile);
        return FALSE;
    }

    //读取文件信息;
    Status = ZwReadFile(hFile,
        NULL,
        NULL,
        NULL,
        &Iosb,
        Buffer ,
        FileStandardInfor.AllocationSize.u.LowPart,
        &Offset,
        NULL);

    //读取失败,返回;
    if (!NT_SUCCESS(Status))
    {
        ExFreePool(Buffer);
        ZwClose(hFile);
        return FALSE;
    }
    
    DosHead = (PIMAGE_DOS_HEADER)Buffer;
    if (DosHead->e_magic != 0x5A4D)   //MZ
    {
        ZwClose(hFile);
        ExFreePool(Buffer);
        return FALSE;
    }

    //得到NT 头
    NtHead = (PIMAGE_NT_HEADERS)((ULONG_PTR)Buffer + DosHead->e_lfanew);
    if (NtHead->Signature != 0x4550)
    {
        ZwClose(hFile);
        ExFreePool(Buffer);
        return FALSE;
    }

    //获得文件头
    FileHead = &(NtHead->FileHeader);
    if (FileHead->Machine == IMAGE_FILE_MACHINE_I386)
    {
        DbgPrint("32Bits File");
    }
    else if(FileHead->Machine==IMAGE_FILE_MACHINE_AMD64 || FileHead->Machine==IMAGE_FILE_MACHINE_IA64)
    {
        DbgPrint("64Bits File");
    }
    else
    {
        ZwClose(hFile);
        ExFreePool(Buffer);
        return FALSE;
    }

    //获得选项头
    OptionHead = &(NtHead->OptionalHeader);

    //获得导入表
    if(OptionHead->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress == 0)   
    {
        ZwClose(hFile);
        ExFreePool(Buffer);
        return FALSE;
    }

    ImportDirectory = OptionHead->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
    ImportTable =  (PIMAGE_IMPORT_DESCRIPTOR)((ULONG_PTR)Buffer+ RVAToOffset(NtHead,ImportDirectory.VirtualAddress));

    while(ImportTable->OriginalFirstThunk)
    {
        //得到DLL文件名
        szDllNam=(char*)((BYTE*)Buffer+RVAToOffset(NtHead,ImportTable->Name));

        //通过OriginalFirstThunk定位到PIMAGE_THUNK_DATA结构数组
        Thunk =(PIMAGE_THUNK_DATA)(PVOID)(((BYTE*)Buffer + RVAToOffset(NtHead,ImportTable->OriginalFirstThunk)));
        while(Thunk->u1.Function)
        {
            //判断函数是用函数名导入的还是序号导入的
            if(Thunk->u1.Ordinal& IMAGE_ORDINAL_FLAG32)//高位为1
            {
                //输出序号
                DbgPrint("从此DLL模块导入的函数的序号：%x\n",Thunk->u1.Ordinal & 0xFFFF);
            }
            else//高位为0
            {
                //得到IMAGE_IMPORT_BY_NAME结构中的函数名
                IMAGE_IMPORT_BY_NAME* ImportName=(IMAGE_IMPORT_BY_NAME*)((BYTE*)Buffer+RVAToOffset(NtHead,Thunk->u1.AddressOfData));
                DbgPrint("从此DLL模块导入的函数的函数名：%s\n",ImportName->Name);
                if(strcmp((char*)ImportName->Name, "CreateThread" ) == 0)
                {
                    DbgPrint("Success");

                }    
            }

            Thunk++;
        }
        ImportTable++;
    }
    ZwClose(hFile);
    ExFreePool(Buffer);

    return TRUE;
}
ULONG_PTR RVAToOffset(PIMAGE_NT_HEADERS NTHeader, ULONG_PTR ulRVA)   
{
    int i = 0;
    PIMAGE_SECTION_HEADER SectionHeader =
        (PIMAGE_SECTION_HEADER)((ULONG_PTR)NTHeader + sizeof(IMAGE_NT_HEADERS));  //获得节表  
    
    for( i = 0; i < NTHeader->FileHeader.NumberOfSections; i++)
    {
        //查询导出表是属于哪个节的 
        /***********************************************************************
            SectionHeader[i].VirtualAddress    节起始的RVA  0x1000
            SectionHeader[i].SizeOfRawData     节在文件上的大小
            SectionHeader[i].PointerToRawData  这是节基于文件的偏移量，PE 装载器通过本域值找到节数据在文件中的位置

            假如导出表在.txt节中
            SectionHeader[i].PointerToRawData == 0x200       SectionHeader[i].VirtualAddress == 0x1000    
                                                             ulRVA = 0x1030
                                                             
            那么导出表在文件中的偏移就是0x230  返回
        ***********************************************************************/
        if(ulRVA >= SectionHeader[i].VirtualAddress && ulRVA < 
            (SectionHeader[i].VirtualAddress 
            + SectionHeader[i].SizeOfRawData))
        {
            //文件偏移
            return SectionHeader[i].PointerToRawData + 
                (ulRVA - SectionHeader[i].VirtualAddress);
        }
    }
    
    return 0;
}

//CreateSection文件映射，不需要转
BOOLEAN EnumIATByFileMapping(PUNICODE_STRING uniDestFileFullPath)
{
    NTSTATUS _statu = STATUS_SUCCESS;
    IMAGE_THUNK_DATA * pFirstThunk ;
    IMAGE_THUNK_DATA * pOriginalFirstThunk;
    IMAGE_IMPORT_BY_NAME * pImpName;
    char* pimFuncName;
    ULONG * pThuk;
    ULONG * funcRva;
    ULONG dwCount  = 0;
    UNICODE_STRING ptrDriverName;
    OBJECT_ATTRIBUTES oa;
    ANSI_STRING anStrPath;

    DbgPrint("Second:%d",PsGetCurrentProcessId());
    _statu = MAPFILE_AND_GETBASE(uniDestFileFullPath);

    if(!NT_SUCCESS(_statu))
    {
        KdPrint((" NTSTATUS = %X \n",_statu));
        return FALSE;
    }

    //DbgPrint((" PNTKRNLPABASE->%0X ;",PNTKRNLPABASE));
    DbgPrint(" pImportDesc->%0X \n",pImportDesc);
    DbgPrint(" MapFileBaseAddress->%0X \n",MapFileBaseAddress);
  
    dwCount = 1;
    while(pImportDesc->OriginalFirstThunk != 0)
    {
        pThuk = (ULONG*)((BYTE*)MapFileBaseAddress + pImportDesc->OriginalFirstThunk);
        funcRva = (ULONG*)((BYTE*)MapFileBaseAddress +pImportDesc->FirstThunk);
  
        // KdPrint((" OriginalFirstThunk->%0X ; FirstThunk->%0X \n",pThuk,funcRva));
        while(*pThuk != 0)
        {
            //这里要小心，内存映射中的pThuk是偏移不是地址
            pImpName = (IMAGE_IMPORT_BY_NAME *)((BYTE*)MapFileBaseAddress + *pThuk);
            DbgPrint(" No. %d : Hint->%d; Name->%s; Address->%0X\n", dwCount, pImpName->Hint, (char*)pImpName->Name, *funcRva);
      
            if(strcmp((char*)pImpName->Name,"CreateThread")==0)
            {
                DbgPrint("Thrid:%d",PsGetCurrentProcessId());
            }
            pThuk++;
            dwCount++;
            funcRva++;
        }
        pImportDesc++;
    }
    ZwUnmapViewOfSection(ZwCurrentProcess(),MapFileBaseAddress);
    ZwClose(hanSection);
    ZwClose(hFile);
    KdPrint((" step->5->CloseFileHandle \n"));
    return TRUE;
}

NTSTATUS MAPFILE_AND_GETBASE(PUNICODE_STRING pDriverName)
{
    SIZE_T size=0;
    IO_STATUS_BLOCK stataus;
    OBJECT_ATTRIBUTES oa ;
    NTSTATUS _stu;
    IMAGE_DOS_HEADER * ptrDosHeader;
    IMAGE_OPTIONAL_HEADER * ptrOptHeader;

    InitializeObjectAttributes(&oa,pDriverName,OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,NULL,NULL);
    DbgPrint((" step->1 \n"));

    _stu = IoCreateFile(&hFile,
        GENERIC_READ,
        &oa,
        &stataus,
        NULL,
        0,
        FILE_SHARE_READ|FILE_SHARE_DELETE,
        FILE_OPEN_IF,
        FILE_SYNCHRONOUS_IO_NONALERT,
        NULL,
        0,
        CreateFileTypeNone,
        NULL,
        IO_NO_PARAMETER_CHECKING
        );
    DbgPrint(" step->2->hFile->%0X  \n",hFile);
    oa.ObjectName = 0;
    _stu = ZwCreateSection(&hanSection,SECTION_ALL_ACCESS,&oa,0,
    PAGE_EXECUTE_READWRITE, SEC_IMAGE, hFile);
    if (!NT_SUCCESS(_stu))
    {
        DbgPrint("<==============ZwCreateSection STATUS->ERROR : %0X \n",_stu);
        ZwClose(hFile);
        return _stu;
    }
    KdPrint((" step->3->hanSection->%0X \n",hanSection));
  
    _stu = ZwMapViewOfSection(hanSection,ZwCurrentProcess(),
      &MapFileBaseAddress, 0, 1024,0, &size,
      ViewShare,MEM_TOP_DOWN, PAGE_READWRITE); 
    if (!NT_SUCCESS(_stu))
    {
        KdPrint(("<==============ZwMapViewOfSection STATUS->ERROR : %0X \n",_stu));
        ZwClose(hFile);
        ZwClose(hanSection);
        return _stu;
    }
    __try
    {
        if ((ULONG)MapFileBaseAddress < 0x80000000L)
        {
            ProbeForRead(MapFileBaseAddress,size,1L);
        }
    }__except(EXCEPTION_EXECUTE_HANDLER)
    {
        KdPrint((" MapFileBaseAddress can not read \n"));
        ZwUnmapViewOfSection(ZwCurrentProcess(),MapFileBaseAddress);
        ZwClose(hFile);
        ZwClose(hanSection);
        return STATUS_ACCESS_DENIED;
    }
    KdPrint((" step->4->mapFileBase->%0X \n",MapFileBaseAddress));
  
    /*基址就是PE的IMAGE_DOS_HEADER*/
    ptrDosHeader = (IMAGE_DOS_HEADER *)MapFileBaseAddress;
    KdPrint((" ptrDosHeader.e_magic-> %8X,ptrDosHeader.e_lfanew-> %8X \n",
    ptrDosHeader->e_magic,ptrDosHeader->e_lfanew));
    //定位到PE HEADER
    //基址hMod加上IMAGE_DOS_HEADER结构的e_lfanew成员到达IMAGE_NT_HEADERS
    //NT文件头的前4字节是文件签名("PE00" 字符串),然后是20字节的IMAGE_FILE_HEADER结构
    //即到达IMAGE_OPTIONAL_HEADER结构的地址,获取了一个指向IMAGE_OPTIONAL_HEADER结构体的指针
    ptrOptHeader = (IMAGE_OPTIONAL_HEADER *)((BYTE*)MapFileBaseAddress + ptrDosHeader->e_lfanew + 24);
    //定位到导入表
    //通过IMAGE_OPTIONAL_HEADER结构中的DataDirectory结构数组中的第二个成员中的
    //VirturalAddress字段定位到IMAGE_IMPORT_DESCRIPTOR结构的起始地址
    //即获得导入表中第一个IMAGE_IMPORT_DESCRIPTOR结构的指针(导入表首地址)
    //DataDirectory[0]是导出表
    pImportDesc = (IMAGE_IMPORT_DESCRIPTOR*)
      ((BYTE*)MapFileBaseAddress + ptrOptHeader->DataDirectory[1].VirtualAddress);
    DbgPrint("fourth:%d",PsGetCurrentProcessId());
    return _stu;
}

VOID UnicodeToChar(PUNICODE_STRING uniSource ,CHAR *szDest)
{
    ANSI_STRING ansiTemp;
    RtlUnicodeStringToAnsiString(&ansiTemp,uniSource,TRUE);

    strcpy(szDest,ansiTemp.Buffer);
    RtlFreeAnsiString(&ansiTemp);
}
NTSTATUS  ForceDeleteFileHandle(UNICODE_STRING  uniFileFullPath)
{
    NTSTATUS                         Status;
    PSYSTEM_HANDLE_INFOR             SystemHandleInfor = NULL;
    PSYSTEM_HANDLE_TABLE_ENTRY_INFOR SystemHandleTableEntryInfor = NULL;
    ULONG ulLength = 0;
    ULONG   i = 0;
    OBJECT_ATTRIBUTES  oa;
    CLIENT_ID          Cid;
    KIRQL               OldIrql;
    HANDLE             hCurrentProcess = NULL;
    HANDLE             hTargetProcess = NULL;
    HANDLE             hCurrentHandle  = NULL;
    char               szBuffer[260] = {0};
    POBJECT_NAME_INFORMATION  NameInfor = NULL;

    NameInfor = (POBJECT_NAME_INFORMATION)szBuffer;

    Cid.UniqueProcess = PsGetCurrentProcessId();  //获得SystemID
    Cid.UniqueThread = 0;

    InitializeObjectAttributes(&oa,NULL,OBJ_KERNEL_HANDLE,NULL,NULL);

    Status = NtOpenProcess(&hCurrentProcess,PROCESS_ALL_ACCESS,&oa,&Cid);  //获得System句柄
    if (!NT_SUCCESS(Status))
    {
        return Status;
    }

    Status = ZwQuerySystemInformation(SystemHandleInformation,NULL,
        ulLength,&ulLength);

    if (Status == STATUS_INFO_LENGTH_MISMATCH)
    {
        SystemHandleInfor = (PSYSTEM_HANDLE_INFOR)ExAllocatePool(PagedPool,ulLength);
        if (SystemHandleInfor==NULL)
        {
            ZwClose(hCurrentProcess);
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        Status = ZwQuerySystemInformation(SystemHandleInformation,SystemHandleInfor,
            ulLength,&ulLength);
    }

    if (!NT_SUCCESS(Status))
    {
        ZwClose(hCurrentProcess);
        ExFreePool(SystemHandleInfor);
        return Status;
    }

    for (i=0; i<SystemHandleInfor->NumberOfHandles; i++)
    {
        SystemHandleTableEntryInfor = &SystemHandleInfor->Handles[i];

        if (SystemHandleTableEntryInfor->ObjectTypeIndex == OB_TYPE_FILE)  //文件对象类型
        {
            InitializeObjectAttributes(&oa,NULL,OBJ_KERNEL_HANDLE,NULL,NULL);

            Cid.UniqueProcess = (HANDLE)SystemHandleTableEntryInfor->UniqueProcessId;
            Cid.UniqueThread = 0;
            Status = NtOpenProcess(&hTargetProcess,PROCESS_DUP_HANDLE,&oa,&Cid);
            if (NT_SUCCESS(Status))
            {
                if (NT_SUCCESS(ZwDuplicateObject(hTargetProcess,(HANDLE)SystemHandleTableEntryInfor->HandleValue,hCurrentProcess,&hCurrentHandle,0,0,DUPLICATE_SAME_ACCESS)))
                {
                    if (NT_SUCCESS(ZwQueryObject(hCurrentHandle,ObjectNameInformation,NameInfor,260,NULL)))
                    {
                        DbgPrint("%wZ\r\n",&NameInfor->Name);
                        if (RtlCompareUnicodeString(&NameInfor->Name,&uniFileFullPath,FALSE) == 0)
                        {
                            ZwClose(hCurrentHandle);
                            OldIrql = KeRaiseIrqlToDpcLevel();  
                            if (NT_SUCCESS(ZwDuplicateObject(hTargetProcess,(HANDLE)SystemHandleTableEntryInfor->HandleValue,hCurrentProcess,&hCurrentHandle,0,
                                0,DUPLICATE_CLOSE_SOURCE)))
                            {
                                ZwClose(hCurrentHandle);
                            }
                            KeLowerIrql(OldIrql);
                            ZwClose(hTargetProcess);
                            break;
                        }
                    }
                    ZwClose(hCurrentHandle);
                }
                ZwClose(hTargetProcess);
            }
        }
    }

    ZwClose(hCurrentProcess);
    ExFreePool(SystemHandleInfor);
    return Status;
}

VOID UnloadDriver(PDRIVER_OBJECT  DriverObject)
{
    PsRemoveLoadImageNotifyRoutine((PLOAD_IMAGE_NOTIFY_ROUTINE)LoadImageNotifyRoutine);
    DbgPrint("UnloadDriver\r\n");
}
