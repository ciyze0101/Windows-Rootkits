

#ifndef CXX_PROTECTPROCESSX64_H
#    include "ProtectProcessx64.h"
#endif


PVOID obHandle;//定义一个void*类型的变量，它将会作为ObRegisterCallbacks函数的第2个参数。

NTSTATUS
DriverEntry(IN PDRIVER_OBJECT pDriverObj, IN PUNICODE_STRING pRegistryString)
{
    NTSTATUS status = STATUS_SUCCESS;
    PLDR_DATA_TABLE_ENTRY64 ldr;

    pDriverObj->DriverUnload = DriverUnload;
    // 绕过MmVerifyCallbackFunction。
    ldr = (PLDR_DATA_TABLE_ENTRY64)pDriverObj->DriverSection;
    ldr->Flags |= 0x20;

    ProtectProcess(TRUE);

    return STATUS_SUCCESS;
}



NTSTATUS ProtectProcess(BOOLEAN Enable)
{
    OB_CALLBACK_REGISTRATION obReg;
    OB_OPERATION_REGISTRATION opReg;

    memset(&obReg, 0, sizeof(obReg));
    obReg.Version = ObGetFilterVersion();
    obReg.OperationRegistrationCount = 1;
    obReg.RegistrationContext = NULL;
    RtlInitUnicodeString(&obReg.Altitude, L"321000");
    memset(&opReg, 0, sizeof(opReg)); //初始化结构体变量

    //下面 请注意这个结构体的成员字段的设置
    opReg.ObjectType = PsProcessType;
    opReg.Operations = OB_OPERATION_HANDLE_CREATE|OB_OPERATION_HANDLE_DUPLICATE; 
    opReg.PreOperation = (POB_PRE_OPERATION_CALLBACK)&preCall; //在这里注册一个回调函数指针
    obReg.OperationRegistration = &opReg; //注意这一条语句

    return ObRegisterCallbacks(&obReg, &obHandle); //在这里注册回调函数
}


OB_PREOP_CALLBACK_STATUS 
    preCall(PVOID RegistrationContext, POB_PRE_OPERATION_INFORMATION pOperationInformation)
{
    HANDLE pid = PsGetProcessId((PEPROCESS)pOperationInformation->Object);
    char szProcName[16]={0};
    UNREFERENCED_PARAMETER(RegistrationContext);
    strcpy(szProcName,GetProcessImageNameByProcessID((ULONG)pid));
    if( !_stricmp(szProcName,"calc.exe") )
    {
        if (pOperationInformation->Operation == OB_OPERATION_HANDLE_CREATE)
        {
            if ((pOperationInformation->Parameters->CreateHandleInformation.OriginalDesiredAccess & PROCESS_TERMINATE) == PROCESS_TERMINATE)
            {
                //Terminate the process, such as by calling the user-mode TerminateProcess routine..
                pOperationInformation->Parameters->CreateHandleInformation.DesiredAccess &= ~PROCESS_TERMINATE;
            }
            if ((pOperationInformation->Parameters->CreateHandleInformation.OriginalDesiredAccess & PROCESS_VM_OPERATION) == PROCESS_VM_OPERATION)
            {
                //Modify the address space of the process, such as by calling the user-mode WriteProcessMemory and VirtualProtectEx routines.
                pOperationInformation->Parameters->CreateHandleInformation.DesiredAccess &= ~PROCESS_VM_OPERATION;
            }
            if ((pOperationInformation->Parameters->CreateHandleInformation.OriginalDesiredAccess & PROCESS_VM_READ) == PROCESS_VM_READ)
            {
                //Read to the address space of the process, such as by calling the user-mode ReadProcessMemory routine.
                pOperationInformation->Parameters->CreateHandleInformation.DesiredAccess &= ~PROCESS_VM_READ;
            }
            if ((pOperationInformation->Parameters->CreateHandleInformation.OriginalDesiredAccess & PROCESS_VM_WRITE) == PROCESS_VM_WRITE)
            {
                //Write to the address space of the process, such as by calling the user-mode WriteProcessMemory routine.
                pOperationInformation->Parameters->CreateHandleInformation.DesiredAccess &= ~PROCESS_VM_WRITE;
            }
        }
    }
    return OB_PREOP_SUCCESS;
}


/*
OpenProcess 会一直走入回调中  直接蓝屏
char*
    GetProcessImageNameByProcessID(ULONG ulProcessID)
{
    CLIENT_ID Cid;    
    HANDLE    hProcess;
    NTSTATUS  Status;
    OBJECT_ATTRIBUTES  oa;
    PEPROCESS  EProcess = NULL;

    Cid.UniqueProcess = (HANDLE)ulProcessID;
    Cid.UniqueThread = 0;

    InitializeObjectAttributes(&oa,0,0,0,0);
    Status = ZwOpenProcess(&hProcess,PROCESS_ALL_ACCESS,&oa,&Cid);    //hProcess
    //强打开进程获得句柄
    if (!NT_SUCCESS(Status))
    {
        return FALSE;
    }
    Status = ObReferenceObjectByHandle(hProcess,FILE_READ_DATA,0,
        KernelMode,&EProcess, 0);
    //通过句柄获取EProcess
    if (!NT_SUCCESS(Status))
    {
        ZwClose(hProcess);
        return FALSE;
    }
    ObDereferenceObject(EProcess);
    //最好判断
    ZwClose(hProcess);
    //通过EProcess获得进程名称
    return (char*)PsGetProcessImageFileName(EProcess);     
    
}
*/

char*
    GetProcessImageNameByProcessID(ULONG ulProcessID)
{
    NTSTATUS  Status;
    PEPROCESS  EProcess = NULL;

    Status = PsLookupProcessByProcessId((HANDLE)ulProcessID,&EProcess);    //hProcess

    //通过句柄获取EProcess
    if (!NT_SUCCESS(Status))
    {
        return FALSE;
    }
    ObDereferenceObject(EProcess);
    //通过EProcess获得进程名称
    return (char*)PsGetProcessImageFileName(EProcess);
}



VOID
DriverUnload(IN PDRIVER_OBJECT pDriverObj)
{    
    UNREFERENCED_PARAMETER(pDriverObj);
    DbgPrint("driver unloading...\n");

    ObUnRegisterCallbacks(obHandle); //obHandle是上面定义的 PVOID obHandle;
}



