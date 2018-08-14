
#ifndef CXX_FILEPROTECTX86_H
#    include "FileProtectX86.h"
#endif

ULONG gC2pKeyCount = 0;
PDRIVER_OBJECT gDriverObject = NULL;

BOOLEAN bOk = FALSE;

ULONG_PTR  IndexOffsetOfFunction = 0;
ULONG_PTR  SSDTDescriptor = 0;
KIRQL Irql;
ULONG_PTR   ulIndex = 0;
ULONG_PTR   ulIndex1 = 0;
ULONG_PTR   ulIndex2 = 0;
pfnNtSetInformationFile Old_NtSetInformationFileWinXP = NULL;
pfnNtDeleteFile Old_NtDeleteFileWinXP = NULL;
//pfnNtCreateFile Old_NtCreateFileWinXP = NULL;
pfnNtWriteFile Old_NtWriteFileWinXP = NULL;
NTSTATUS DriverEntry(IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING RegisterPath)
{
    ULONG i; 
    NTSTATUS status;

    // 填写所有的分发函数的指针
    for (i = 0; i < IRP_MJ_MAXIMUM_FUNCTION; i++) 
    { 
        DriverObject->MajorFunction[i] = c2pDispatchGeneral; 
    } 

    // 单独的填写一个Read分发函数。因为要的过滤就是读取来的按键信息
    // 其他的都不重要。这个分发函数单独写。
    DriverObject->MajorFunction[IRP_MJ_READ] = c2pDispatchRead; 

    // 单独的填写一个IRP_MJ_POWER函数。这是因为这类请求中间要调用
    // 一个PoCallDriver和一个PoStartNextPowerIrp，比较特殊。
    DriverObject->MajorFunction [IRP_MJ_POWER] = c2pPower; 

    // 我们想知道什么时候一个我们绑定过的设备被卸载了（比如从机器上
    // 被拔掉了？）所以专门写一个PNP（即插即用）分发函数
    DriverObject->MajorFunction [IRP_MJ_PNP] = c2pPnP; 

    // 卸载函数。
    DriverObject->DriverUnload = c2pUnload; 
    gDriverObject = DriverObject;
    // 绑定所有键盘设备
    status =c2pAttachDevices(DriverObject, RegisterPath);

    SSDTDescriptor = (ULONG_PTR)GetFunctionAddressByNameFromNtosExport(L"KeServiceDescriptorTable");
    IndexOffsetOfFunction = 1;

    ulIndex = GetSSDTApiFunctionIndexFromNtdll("NtSetInformationFile");
    ulIndex1 = GetSSDTApiFunctionIndexFromNtdll("NtWriteFile");
    ulIndex2 = GetSSDTApiFunctionIndexFromNtdll("NtDeleteFile");

    HookSSDT(ulIndex);
    HookWrite(ulIndex1);
    HookDelete(ulIndex2);

    return STATUS_SUCCESS;
}



NTSTATUS 
    c2pDevExtInit( 
    IN PC2P_DEV_EXT devExt, 
    IN PDEVICE_OBJECT pFilterDeviceObject, 
    IN PDEVICE_OBJECT pTargetDeviceObject, 
    IN PDEVICE_OBJECT pLowerDeviceObject ) 
{ 
    memset(devExt, 0, sizeof(C2P_DEV_EXT)); 
    devExt->NodeSize = sizeof(C2P_DEV_EXT); 
    devExt->pFilterDeviceObject = pFilterDeviceObject; 
    KeInitializeSpinLock(&(devExt->IoRequestsSpinLock)); 
    KeInitializeEvent(&(devExt->IoInProgressEvent), NotificationEvent, FALSE); 
    devExt->TargetDeviceObject = pTargetDeviceObject; 
    devExt->LowerDeviceObject = pLowerDeviceObject; 
    return( STATUS_SUCCESS ); 
}


// 这个函数经过改造。能打开驱动对象Kbdclass，然后绑定
// 它下面的所有的设备：
NTSTATUS 
    c2pAttachDevices( 
    IN PDRIVER_OBJECT DriverObject, 
    IN PUNICODE_STRING RegistryPath 
    ) 
{ 
    NTSTATUS status = 0; 
    UNICODE_STRING uniNtNameString; 
    PC2P_DEV_EXT devExt; 
    PDEVICE_OBJECT pFilterDeviceObject = NULL; 
    PDEVICE_OBJECT pTargetDeviceObject = NULL; 
    PDEVICE_OBJECT pLowerDeviceObject = NULL; 

    PDRIVER_OBJECT KbdDriverObject = NULL; 

    KdPrint(("MyAttach\n")); 

    // 初始化一个字符串，就是Kdbclass驱动的名字。
    RtlInitUnicodeString(&uniNtNameString, KBD_DRIVER_NAME); 
    // 请参照前面打开设备对象的例子。只是这里打开的是驱动对象。
    status = ObReferenceObjectByName ( 
        &uniNtNameString, 
        OBJ_CASE_INSENSITIVE, 
        NULL, 
        0, 
        IoDriverObjectType, 
        KernelMode, 
        NULL, 
        &KbdDriverObject 
        ); 
    // 如果失败了就直接返回
    if(!NT_SUCCESS(status)) 
    { 
        KdPrint(("MyAttach: Couldn't get the MyTest Device Object\n")); 
        return( status ); 
    }
    else
    {
        // 这个打开需要解应用。早点解除了免得之后忘记。
        ObDereferenceObject(DriverObject);
    }

    // 这是设备链中的第一个设备    
    pTargetDeviceObject = KbdDriverObject->DeviceObject;
    // 现在开始遍历这个设备链
    while (pTargetDeviceObject) 
    {
        // 生成一个过滤设备，这是前面读者学习过的。这里的IN宏和OUT宏都是
        // 空宏，只有标志性意义，表明这个参数是一个输入或者输出参数。
        status = IoCreateDevice( 
            IN DriverObject, 
            IN sizeof(C2P_DEV_EXT), 
            IN NULL, 
            IN pTargetDeviceObject->DeviceType, 
            IN pTargetDeviceObject->Characteristics, 
            IN FALSE, 
            OUT &pFilterDeviceObject 
            ); 

        // 如果失败了就直接退出。
        if (!NT_SUCCESS(status)) 
        { 
            KdPrint(("MyAttach: Couldn't create the MyFilter Filter Device Object\n")); 
            return (status); 
        } 

        // 绑定。pLowerDeviceObject是绑定之后得到的下一个设备。也就是
        // 前面常常说的所谓真实设备。
        pLowerDeviceObject = 
            IoAttachDeviceToDeviceStack(pFilterDeviceObject, pTargetDeviceObject); 
        // 如果绑定失败了，放弃之前的操作，退出。
        if(!pLowerDeviceObject) 
        { 
            KdPrint(("MyAttach: Couldn't attach to MyTest Device Object\n")); 
            IoDeleteDevice(pFilterDeviceObject); 
            pFilterDeviceObject = NULL; 
            return( status ); 
        } 

        // 设备扩展！下面要详细讲述设备扩展的应用。
        devExt = (PC2P_DEV_EXT)(pFilterDeviceObject->DeviceExtension); 
        c2pDevExtInit( 
            devExt, 
            pFilterDeviceObject, 
            pTargetDeviceObject, 
            pLowerDeviceObject ); 

        // 下面的操作和前面过滤串口的操作基本一致。这里不再解释了。
        pFilterDeviceObject->DeviceType=pLowerDeviceObject->DeviceType; 
        pFilterDeviceObject->Characteristics=pLowerDeviceObject->Characteristics; 
        pFilterDeviceObject->StackSize=pLowerDeviceObject->StackSize+1; 
        pFilterDeviceObject->Flags |= pLowerDeviceObject->Flags & (DO_BUFFERED_IO | DO_DIRECT_IO | DO_POWER_PAGABLE) ; 
        //next device 
        pTargetDeviceObject = pTargetDeviceObject->NextDevice;
    }
    return status; 
} 


VOID 
    c2pDetach(IN PDEVICE_OBJECT pDeviceObject) 
{ 
    PC2P_DEV_EXT devExt; 
    BOOLEAN NoRequestsOutstanding = FALSE; 
    devExt = (PC2P_DEV_EXT)pDeviceObject->DeviceExtension; 
    __try 
    { 
        __try 
        { 
            IoDetachDevice(devExt->TargetDeviceObject);
            devExt->TargetDeviceObject = NULL; 
            IoDeleteDevice(pDeviceObject); 
            devExt->pFilterDeviceObject = NULL; 
            DbgPrint(("Detach Finished\n")); 
        } 
        __except (EXCEPTION_EXECUTE_HANDLER){} 
    } 
    __finally{} 
    return; 
}



VOID 
    c2pUnload(IN PDRIVER_OBJECT DriverObject) 
{ 
    PDEVICE_OBJECT DeviceObject; 
    PDEVICE_OBJECT OldDeviceObject; 
    PC2P_DEV_EXT devExt; 

    LARGE_INTEGER    lDelay;
    PRKTHREAD CurrentThread;
    //delay some time 
    lDelay = RtlConvertLongToLargeInteger(100 * DELAY_ONE_MILLISECOND);
    CurrentThread = KeGetCurrentThread();
    // 把当前线程设置为低实时模式，以便让它的运行尽量少影响其他程序。
    KeSetPriorityThread(CurrentThread, LOW_REALTIME_PRIORITY);

    UNREFERENCED_PARAMETER(DriverObject); 
    KdPrint(("DriverEntry unLoading...\n")); 

    // 遍历所有设备并一律解除绑定
    DeviceObject = DriverObject->DeviceObject;
    while (DeviceObject)
    {
        // 解除绑定并删除所有的设备
        c2pDetach(DeviceObject);
        DeviceObject = DeviceObject->NextDevice;
    } 
    ASSERT(NULL == DriverObject->DeviceObject);

    while (gC2pKeyCount)
    {
        KeDelayExecutionThread(KernelMode, FALSE, &lDelay);
    }

    UnHookSSDT(ulIndex);
    UnHookSSDTWrite(ulIndex1);
    UnHookSSDTDelete(ulIndex2);
    KdPrint(("DriverEntry unLoad OK!\n")); 
    //return; 
} 


//处理我们不关心的所有IRP
NTSTATUS c2pDispatchGeneral( 
    IN PDEVICE_OBJECT DeviceObject, 
    IN PIRP Irp 
    ) 
{ 
    // 其他的分发函数，直接skip然后用IoCallDriver把IRP发送到真实设备
    // 的设备对象。 
    KdPrint(("Other Diapatch!")); 
    IoSkipCurrentIrpStackLocation(Irp); 
    return IoCallDriver(((PC2P_DEV_EXT)
        DeviceObject->DeviceExtension)->LowerDeviceObject, Irp); 
} 
//只处理主功能号为IRP_MJ_POWER的IRP
NTSTATUS c2pPower( 
    IN PDEVICE_OBJECT DeviceObject, 
    IN PIRP Irp 
    ) 
{ 
    PC2P_DEV_EXT devExt;
    devExt =
        (PC2P_DEV_EXT)DeviceObject->DeviceExtension; 

    PoStartNextPowerIrp( Irp ); 
    IoSkipCurrentIrpStackLocation( Irp ); 
    return PoCallDriver(devExt->LowerDeviceObject, Irp ); 
} 
//设备被拔出时，需解除绑定，并删除过滤设备
NTSTATUS c2pPnP( 
    IN PDEVICE_OBJECT DeviceObject, 
    IN PIRP Irp 
    ) 
{ 
    PC2P_DEV_EXT devExt; 
    PIO_STACK_LOCATION irpStack; 
    NTSTATUS status = STATUS_SUCCESS; 
    KIRQL oldIrql; 
    KEVENT event; 

    // 获得真实设备。
    devExt = (PC2P_DEV_EXT)(DeviceObject->DeviceExtension); 
    irpStack = IoGetCurrentIrpStackLocation(Irp); 

    switch (irpStack->MinorFunction) 
    { 
    case IRP_MN_REMOVE_DEVICE: 
        KdPrint(("IRP_MN_REMOVE_DEVICE\n")); 

        // 首先把请求发下去
        IoSkipCurrentIrpStackLocation(Irp); 
        IoCallDriver(devExt->LowerDeviceObject, Irp); 
        // 然后解除绑定。
        IoDetachDevice(devExt->LowerDeviceObject); 
        // 删除我们自己生成的虚拟设备。
        IoDeleteDevice(DeviceObject); 
        status = STATUS_SUCCESS; 
        break; 

    default: 
        // 对于其他类型的IRP，全部都直接下发即可。 
        IoSkipCurrentIrpStackLocation(Irp); 
        status = IoCallDriver(devExt->LowerDeviceObject, Irp); 
    } 
    return status; 
}

// 这是一个IRP完成回调函数的原型
NTSTATUS c2pReadComplete( 
    IN PDEVICE_OBJECT DeviceObject, 
    IN PIRP Irp, 
    IN PVOID Context 
    ) 
{
    POBJECT_NAME_INFORMATION ObjetNameInfor;  
    ULONG* ulProcessNameLen;
    PIO_STACK_LOCATION IrpSp;
    ULONG buf_len = 0;
    PUCHAR buf = NULL;
    size_t i;
    ULONG numKeys = 0;
    IrpSp = IoGetCurrentIrpStackLocation( Irp );

    //  如果这个请求是成功的。很显然，如果请求失败了，这么获取
    //   进一步的信息是没意义的。
    if( NT_SUCCESS( Irp->IoStatus.Status ) ) 
    { 
        PKEYBOARD_INPUT_DATA pKeyData;
        // 获得读请求完成后输出的缓冲区
        buf = Irp->AssociatedIrp.SystemBuffer;
        pKeyData = Irp->AssociatedIrp.SystemBuffer;

        // 获得这个缓冲区的长度。一般的说返回值有多长都保存在
        // Information中。

        buf_len = Irp->IoStatus.Information;
        numKeys = Irp->IoStatus.Information / sizeof(KEYBOARD_INPUT_DATA);

        __try
        {
            if (NT_SUCCESS(IoQueryFileDosDeviceName((PFILE_OBJECT)IrpSp->FileObject, &ObjetNameInfor)))  
            {  
                if(wcsstr(ObjetNameInfor->Name.Buffer,L"Shine.txt")!=0)
                {
                    DbgPrint("aaaaaaa");
                }
            }  
        }
        __except(1)
        {
            DbgPrint("Exception:%x",GetExceptionCode());
        }


        //通过Process获得进程名称
        for(i = 0; i < numKeys; i++) 
        {
            //    DbgPrint("%02X %d\n",pKeyData[i].MakeCode,pKeyData[i].Flags);

            if(pKeyData[i].MakeCode == 0x1d && pKeyData[i].Flags == KEY_MAKE)
            {
                //左Ctrl
                bOk = TRUE;
            }

            if(pKeyData[i].MakeCode == 0x2e && pKeyData[i].Flags == KEY_MAKE && bOk == TRUE ) //按下
            {
                pKeyData[i].MakeCode = 0x20;
                bOk = FALSE;
                DbgPrint("aaaaaaaaaaaaaa");
            }
        }
        //… 这里可以做进一步的处理。我这里很简单的打印出所有的扫
        // 描码。

        //    for(i=0;i<buf_len;++i)
        //    {
        //DbgPrint("ctrl2cap: %2x\r\n", buf[i]);
        //        if(buf[i]==0x3a)
        //        {
        //            DbgPrint("SSSSSS");
        //        }
        //    }

    }
    gC2pKeyCount--;

    if( Irp->PendingReturned )
    { 
        IoMarkIrpPending( Irp ); 
    } 
    return Irp->IoStatus.Status;
}

NTSTATUS c2pDispatchRead( 
    IN PDEVICE_OBJECT DeviceObject, 
    IN PIRP Irp ) 
{ 
    NTSTATUS status = STATUS_SUCCESS; 
    PC2P_DEV_EXT devExt; 
    PIO_STACK_LOCATION currentIrpStack; 
    KEVENT waitEvent;
    KeInitializeEvent( &waitEvent, NotificationEvent, FALSE );

    if (Irp->CurrentLocation == 1) 
    { 
        ULONG ReturnedInformation = 0; 
        KdPrint(("Dispatch encountered bogus current location\n")); 
        status = STATUS_INVALID_DEVICE_REQUEST; 
        Irp->IoStatus.Status = status; 
        Irp->IoStatus.Information = ReturnedInformation; 
        IoCompleteRequest(Irp, IO_NO_INCREMENT); 
        return(status); 
    } 

    // 全局变量键计数器加1
    gC2pKeyCount++;

    // 得到设备扩展。目的是之后为了获得下一个设备的指针。
    devExt =
        (PC2P_DEV_EXT)DeviceObject->DeviceExtension;

    // 设置回调函数并把IRP传递下去。 之后读的处理也就结束了。
    // 剩下的任务是要等待读请求完成。
    currentIrpStack = IoGetCurrentIrpStackLocation(Irp); 
    IoCopyCurrentIrpStackLocationToNext(Irp);
    IoSetCompletionRoutine( Irp, c2pReadComplete, 
        DeviceObject, TRUE, TRUE, TRUE ); 
    return  IoCallDriver( devExt->LowerDeviceObject, Irp );     
}

VOID  HookSSDT(ULONG_PTR ulIndex)
{
    PULONG32  ServiceTableBase = NULL;
    ServiceTableBase = (PULONG32)((PSYSTEM_SERVICE_TABLE32)SSDTDescriptor)->ServiceTableBase;    //数组首地址
    Old_NtSetInformationFileWinXP  = (pfnNtSetInformationFile)ServiceTableBase[ulIndex];      //先保存原先的函数地址

    WPOFF();  
    ServiceTableBase[ulIndex] = (ULONG32)Fake_NtSetInformationFileWinXP;  //将KeBugCheckEx函数的偏移地址放入SSDT表中
    WPON();    
}

VOID HookWrite(ULONG_PTR ulIndex)
{
    PULONG32  ServiceTableBase = NULL;
    ServiceTableBase = (PULONG32)((PSYSTEM_SERVICE_TABLE32)SSDTDescriptor)->ServiceTableBase;    //数组首地址
    Old_NtWriteFileWinXP  = (pfnNtWriteFile)ServiceTableBase[ulIndex];      //先保存原先的函数地址

    WPOFF();  
    ServiceTableBase[ulIndex] = (ULONG32)Fake_NtWriteFileWinXP;  //将KeBugCheckEx函数的偏移地址放入SSDT表中
    WPON();
}

VOID HookDelete(ULONG_PTR ulIndex)
{
    PULONG32  ServiceTableBase = NULL;
    ServiceTableBase = (PULONG32)((PSYSTEM_SERVICE_TABLE32)SSDTDescriptor)->ServiceTableBase;    //数组首地址
    Old_NtDeleteFileWinXP  = (pfnNtDeleteFile)ServiceTableBase[ulIndex];      //先保存原先的函数地址

    WPOFF();  
    ServiceTableBase[ulIndex] = (ULONG32)Fake_NtDeleteFileWinXP;  //将KeBugCheckEx函数的偏移地址放入SSDT表中
    WPON();
}


VOID
    UnHookSSDT(ULONG_PTR ulIndex)
{
    PULONG32  ServiceTableBase = NULL;
    ServiceTableBase=(PULONG32)((PSYSTEM_SERVICE_TABLE32)SSDTDescriptor)->ServiceTableBase;

    WPOFF();
    ServiceTableBase[ulIndex] = (ULONG32)Old_NtSetInformationFileWinXP;
    WPON();
}

VOID
    UnHookSSDTWrite(ULONG_PTR ulIndex)
{

    PULONG32  ServiceTableBase = NULL;
    ServiceTableBase=(PULONG32)((PSYSTEM_SERVICE_TABLE32)SSDTDescriptor)->ServiceTableBase;

    WPOFF();
    ServiceTableBase[ulIndex] = (ULONG32)Old_NtWriteFileWinXP;
    WPON();

}

VOID
    UnHookSSDTDelete(ULONG_PTR ulIndex)
{
    PULONG32  ServiceTableBase = NULL;
    ServiceTableBase=(PULONG32)((PSYSTEM_SERVICE_TABLE32)SSDTDescriptor)->ServiceTableBase;

    WPOFF();
    ServiceTableBase[ulIndex] = (ULONG32)Old_NtDeleteFileWinXP;
    WPON();
}


NTSTATUS Fake_NtSetInformationFileWinXP(
    __in HANDLE FileHandle,
    __out PIO_STATUS_BLOCK IoStatusBlock,
    __in_bcount(Length) PVOID FileInformation,
    __in ULONG Length,
    __in FILE_INFORMATION_CLASS FileInformationClass
    )
{
    NTSTATUS Status;
    PFILE_OBJECT hObject;
    POBJECT_NAME_INFORMATION ObjetNameInfor;  

    Status = ObReferenceObjectByHandle(FileHandle,FILE_READ_DATA,0,KernelMode,&hObject, 0);
    //通过进程句柄获取EProcess对象

    if (NT_SUCCESS(IoQueryFileDosDeviceName((PFILE_OBJECT)hObject, &ObjetNameInfor)))  
    {  
        if(wcsstr((ObjetNameInfor->Name).Buffer,L"D:\\Shine.txt"))
        {
            if(FileInformationClass == FileRenameInformation)
            {
                return STATUS_ACCESS_DENIED;
            }
        }
    }  

    return Old_NtSetInformationFileWinXP(FileHandle,IoStatusBlock,FileInformation,Length,FileInformationClass);
}

NTSTATUS
    Fake_NtWriteFileWinXP (
    __in HANDLE FileHandle,
    __in_opt HANDLE Event,
    __in_opt PIO_APC_ROUTINE ApcRoutine,
    __in_opt PVOID ApcContext,
    __out PIO_STATUS_BLOCK IoStatusBlock,
    __in_bcount(Length) PVOID Buffer,
    __in ULONG Length,
    __in_opt PLARGE_INTEGER ByteOffset,
    __in_opt PULONG Key
    )
{
    NTSTATUS Status;
    PFILE_OBJECT hObject;
    POBJECT_NAME_INFORMATION ObjetNameInfor;  

    Status = ObReferenceObjectByHandle(FileHandle,FILE_READ_DATA,0,KernelMode,&hObject, 0);
    //通过进程句柄获取EProcess对象

    if (NT_SUCCESS(IoQueryFileDosDeviceName((PFILE_OBJECT)hObject, &ObjetNameInfor)))  
    {  
        if(wcsstr((ObjetNameInfor->Name).Buffer,L"D:\\Shine.txt"))
        {
            return STATUS_ACCESS_DENIED;
        }
    }  

    return Old_NtWriteFileWinXP(FileHandle,Event,ApcRoutine,ApcContext,IoStatusBlock,Buffer,Length,ByteOffset,Key);
}


NTSTATUS Fake_NtDeleteFileWinXP(
    __in POBJECT_ATTRIBUTES ObjectAttributes
    )
{
    if(wcsstr((ObjectAttributes->ObjectName)->Buffer,L"D:\\Shine.txt"))
    {
        return STATUS_ACCESS_DENIED;
    }
    return Old_NtDeleteFileWinXP(ObjectAttributes);
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

LONG GetSSDTApiFunctionIndexFromNtdll(char* szFindFunctionName)
{

    NTSTATUS Status = STATUS_UNSUCCESSFUL;
    PVOID    MapBase = NULL;
    PIMAGE_NT_HEADERS       NtHeader;
    PIMAGE_EXPORT_DIRECTORY ExportTable;
    ULONG*  FunctionAddresses;
    ULONG*  FunctionNames;
    USHORT* FunctionIndexs;
    ULONG   ulIndex;
    ULONG   i;
    CHAR*   szFunctionName;
    SIZE_T  ViewSize=0;
    ULONG_PTR ulFunctionAddress;
    WCHAR wzNtdll[] = L"\\SystemRoot\\System32\\ntdll.dll";

    Status = MapFileInUserSpace(wzNtdll, NtCurrentProcess(), &MapBase, &ViewSize);
    if (!NT_SUCCESS(Status))
    {
        return STATUS_UNSUCCESSFUL;
    }
    else
    {
        __try{
            NtHeader = RtlImageNtHeader(MapBase);
            if (NtHeader && NtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress){
                ExportTable =(IMAGE_EXPORT_DIRECTORY*)((ULONG_PTR)MapBase + NtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);
                FunctionAddresses = (ULONG*)((ULONG_PTR)MapBase + ExportTable->AddressOfFunctions);
                FunctionNames = (ULONG*)((ULONG_PTR)MapBase + ExportTable->AddressOfNames);
                FunctionIndexs = (USHORT*)((ULONG_PTR)MapBase + ExportTable->AddressOfNameOrdinals);
                for(i = 0; i < ExportTable->NumberOfNames; i++)
                {
                    szFunctionName = (LPSTR)((ULONG_PTR)MapBase + FunctionNames[i]);
                    if (_stricmp(szFunctionName, szFindFunctionName) == 0) 
                    {
                        ulIndex = FunctionIndexs[i]; 
                        ulFunctionAddress = (ULONG_PTR)((ULONG_PTR)MapBase + FunctionAddresses[ulIndex]);
                        ulIndex=*(ULONG*)(ulFunctionAddress+IndexOffsetOfFunction);
                        break;
                    }
                }
            }
        }__except(EXCEPTION_EXECUTE_HANDLER)
        {

        }
    }

    if (ulIndex == -1)
    {
        DbgPrint("%s Get Index Error\n", szFindFunctionName);
    }

    ZwUnmapViewOfSection(NtCurrentProcess(), MapBase);
    return ulIndex;
}

NTSTATUS 
    MapFileInUserSpace(WCHAR* wzFilePath,IN HANDLE hProcess OPTIONAL,
    OUT PVOID *BaseAddress,
    OUT PSIZE_T ViewSize OPTIONAL)
{
    NTSTATUS Status = STATUS_INVALID_PARAMETER;
    HANDLE   hFile = NULL;
    HANDLE   hSection = NULL;
    OBJECT_ATTRIBUTES oa;
    SIZE_T MapViewSize = 0;
    IO_STATUS_BLOCK Iosb;
    UNICODE_STRING uniFilePath;

    if (!wzFilePath || !BaseAddress){
        return Status;
    }

    RtlInitUnicodeString(&uniFilePath, wzFilePath);
    InitializeObjectAttributes(&oa,
        &uniFilePath,
        OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
        NULL,
        NULL
        );

    Status = IoCreateFile(&hFile,
        GENERIC_READ | SYNCHRONIZE,
        &oa,
        &Iosb,
        NULL,
        FILE_ATTRIBUTE_NORMAL,
        FILE_SHARE_READ,
        FILE_OPEN,
        FILE_SYNCHRONOUS_IO_NONALERT,
        NULL,
        0,
        CreateFileTypeNone,
        NULL,
        IO_NO_PARAMETER_CHECKING
        );

    if (!NT_SUCCESS(Status))
    {
        return Status;
    }

    oa.ObjectName = NULL;
    Status = ZwCreateSection(&hSection,
        SECTION_QUERY | SECTION_MAP_READ,
        &oa,
        NULL,
        PAGE_WRITECOPY,
        SEC_IMAGE,
        hFile
        );
    ZwClose(hFile);
    if (!NT_SUCCESS(Status))
    {
        return Status;
    }

    if (!hProcess){
        hProcess = NtCurrentProcess();
    }

    Status = ZwMapViewOfSection(hSection, 
        hProcess, 
        BaseAddress, 
        0, 
        0, 
        0, 
        ViewSize ? ViewSize : &MapViewSize, 
        ViewUnmap, 
        0, 
        PAGE_WRITECOPY
        );
    ZwClose(hSection);
    if (!NT_SUCCESS(Status))
    {
        return Status;
    }

    return Status;
}

ULONG_PTR GetFunctionAddressByIndexFromSSDT32(ULONG_PTR ulIndex,ULONG_PTR SSDTDescriptor)
{
    ULONG_PTR ServiceTableBase= 0 ;
    PSYSTEM_SERVICE_TABLE32 SSDT = (PSYSTEM_SERVICE_TABLE32)SSDTDescriptor;

    ServiceTableBase=(ULONG)(SSDT ->ServiceTableBase);

    return (*(PULONG_PTR)(ServiceTableBase + 4 * ulIndex));
}

VOID WPOFF()
{
    ULONG_PTR cr0 = 0;
    Irql = KeRaiseIrqlToDpcLevel();
    cr0 =__readcr0();
    cr0 &= 0xfffffffffffeffff;
    __writecr0(cr0);
    //_disable();
}

VOID WPON()
{
    ULONG_PTR cr0=__readcr0();
    cr0 |= 0x10000;
    //_enable();
    __writecr0(cr0);
    KeLowerIrql(Irql);
}


