#include "Common.h"


ULONG_PTR  ObjectTableOffsetOf_EPROCESS = 0;
ULONG_PTR  PreviousModeOffsetOf_KTHREAD = 0;
ULONG_PTR  IndexOffsetOfFunction = 0;
ULONG_PTR  SSDTDescriptor = 0;

ULONG_PTR  HighUserAddress = 0;

WIN_VERSION WinVersion = WINDOWS_UNKNOW;
ULONG_PTR LdrInPebOffset =  0;
ULONG_PTR ModListInLdrOffset = 0;
ULONG_PTR  ObjectHeaderSize = 0;
ULONG_PTR ObjectTypeOffsetOf_Object_Header =0;



WIN_VERSION GetWindowsVersion()
{
    RTL_OSVERSIONINFOEXW osverInfo = {sizeof(osverInfo)}; 
    pfnRtlGetVersion RtlGetVersion = NULL;
    WIN_VERSION WinVersion;
    WCHAR szRtlGetVersion[] = L"RtlGetVersion";

    RtlGetVersion = (pfnRtlGetVersion)GetFunctionAddressByName(szRtlGetVersion); 

    if (RtlGetVersion)
    {
        RtlGetVersion((PRTL_OSVERSIONINFOW)&osverInfo); 
    } 
    else 
    {
        PsGetVersion(&osverInfo.dwMajorVersion, &osverInfo.dwMinorVersion, &osverInfo.dwBuildNumber, NULL);
    }

    if(osverInfo.dwMajorVersion == 6 && osverInfo.dwMinorVersion == 1 && osverInfo.dwBuildNumber == 7600)
    {
        DbgPrint("WINDOWS 7\r\n");
        WinVersion = WINDOWS_7_7600;
    }
    else if(osverInfo.dwMajorVersion == 6 && osverInfo.dwMinorVersion == 1 && osverInfo.dwBuildNumber == 7601)
    {
        DbgPrint("WINDOWS 7\r\n");
        WinVersion = WINDOWS_7_7601;
    }
    else if(osverInfo.dwMajorVersion == 6 && osverInfo.dwMinorVersion == 2 && osverInfo.dwBuildNumber == 9200)
    {
        DbgPrint("WINDOWS 8\r\n");
        WinVersion = WINDOWS_8_9200;
    }
    else if(osverInfo.dwMajorVersion == 6 && osverInfo.dwMinorVersion == 3 && osverInfo.dwBuildNumber == 9600)
    {
        DbgPrint("WINDOWS 8.1\r\n");
        WinVersion = WINDOWS_8_9600;
    }
    else if(osverInfo.dwMajorVersion == 10 && osverInfo.dwMinorVersion == 0 && osverInfo.dwBuildNumber == 10240)
    {
        DbgPrint("WINDOWS 10 10240\r\n");
        WinVersion = WINDOWS_10_10240;
    }
    else if(osverInfo.dwMajorVersion == 10 && osverInfo.dwMinorVersion == 0 && osverInfo.dwBuildNumber == 10586)
    {
        DbgPrint("WINDOWS 10 10586\r\n");
        WinVersion = WINDOWS_10_10586;
    }
    else if(osverInfo.dwMajorVersion == 10 && osverInfo.dwMinorVersion == 0 && osverInfo.dwBuildNumber == 14393)
    {
        DbgPrint("WINDOWS 10 14393\r\n");
        WinVersion = WINDOWS_10_14393;
    }
    else if(osverInfo.dwMajorVersion == 10 && osverInfo.dwMinorVersion == 0 && osverInfo.dwBuildNumber == 15063)
    {
        DbgPrint("WINDOWS 10 15063\r\n");
        WinVersion = WINDOWS_10_15063;
    }
    else if(osverInfo.dwMajorVersion == 10 && osverInfo.dwMinorVersion == 0 && osverInfo.dwBuildNumber == 16299)
    {
        DbgPrint("WINDOWS 10 16299\r\n");
        WinVersion = WINDOWS_10_16299;
    }
    else if(osverInfo.dwMajorVersion == 10 && osverInfo.dwMinorVersion == 0 && osverInfo.dwBuildNumber == 17134)
    {
        DbgPrint("WINDOWS 10 17134\r\n");
        WinVersion = WINDOWS_10_17134;
    }
    else
    {
        DbgPrint("This is a new os\r\n");
        WinVersion = WINDOWS_UNKNOW;
    }

    return WinVersion;
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


VOID InitGlobalVariable()
{
    WinVersion = GetWindowsVersion();
    switch(WinVersion)
    {
#ifdef _WIN32
    case WINDOWS_XP:
        {
            ObjectHeaderSize = 0x18;
            ObjectTypeOffsetOf_Object_Header = 0x8;
            LdrInPebOffset  = 0x00c;
            ModListInLdrOffset  = 0x00c;
            ObjectHeaderSize  = 0x18;
            ObjectTableOffsetOf_EPROCESS = 0x0c4;
            PreviousModeOffsetOf_KTHREAD = 0x140;
            HighUserAddress = 0x80000000;

            break;
        }
#else
    case WINDOWS_7_7601:
        {
            LdrInPebOffset = 0x018;
            ModListInLdrOffset = 0x010;
            ObjectTableOffsetOf_EPROCESS = 0x200;
            PreviousModeOffsetOf_KTHREAD = 0x1f6;
            HighUserAddress   = 0x80000000000;
            break;
        }
#endif
    default:
        return;
    }
}



BOOLEAN IsRealProcess(PEPROCESS EProcess) 
{ 
    ULONG_PTR    ObjectType; 
    ULONG_PTR    ObjectTypeAddress; 
    BOOLEAN bRet = FALSE;

    ULONG_PTR ProcessType = ((ULONG_PTR)*PsProcessType);

    if (ProcessType && EProcess && MmIsAddressValid((PVOID)(EProcess)))
    { 
        ObjectType = KeGetObjectType((PVOID)EProcess);   //*PsProcessType  
        if (ObjectType && 
            ProcessType == ObjectType &&
            !IsProcessDie(EProcess))    
        {
            bRet = TRUE; 
        }
    } 

    return bRet; 
} 



ULONG_PTR KeGetObjectType(PVOID Object)
{
    ULONG_PTR ObjectType = 0;
    pfnObGetObjectType        ObGetObjectType = NULL;    

    if (NULL == MmIsAddressValid ||!Object||!MmIsAddressValid(Object))
    {
        return 0;
    }

    if (WinVersion==WINDOWS_XP)
    {
        ULONG SizeOfObjectHeader = 0, ObjectTypeOffset = 0;
        ULONG_PTR ObjectTypeAddress = 0;

        ObjectTypeAddress = (ULONG_PTR)Object - ObjectHeaderSize + ObjectTypeOffsetOf_Object_Header;

        if (MmIsAddressValid((PVOID)ObjectTypeAddress))
        { 
            ObjectType = *(ULONG_PTR*)ObjectTypeAddress;
        }
    }
    else 
    {
        //高版本使用函数

        ObGetObjectType = (pfnObGetObjectType)GetFunctionAddressByName(L"ObGetObjectType");


        if (ObGetObjectType)
        {
            ObjectType = ObGetObjectType(Object);
        }
    }

    return ObjectType;
}

BOOLEAN IsProcessDie(PEPROCESS EProcess)
{
    BOOLEAN bDie = FALSE;

    if (MmIsAddressValid &&
        EProcess && 
        MmIsAddressValid(EProcess) &&
        MmIsAddressValid((PVOID)((ULONG_PTR)EProcess + ObjectTableOffsetOf_EPROCESS)))
    {
        PVOID ObjectTable = *(PVOID*)((ULONG_PTR)EProcess + ObjectTableOffsetOf_EPROCESS );

        if (!ObjectTable||!MmIsAddressValid(ObjectTable) )
        {
            DbgPrint("Process is Die\r\n");
            bDie = TRUE;
        }
    }
    else
    {
        DbgPrint("Process is Die2\r\n");
        bDie = TRUE;
    }
    return bDie;
}




CHAR ChangePreMode(PETHREAD EThread)
{

    CHAR PreMode = *(PCHAR)((ULONG_PTR)EThread + PreviousModeOffsetOf_KTHREAD);
    *(PCHAR)((ULONG_PTR)EThread + PreviousModeOffsetOf_KTHREAD) = KernelMode;
    return PreMode;
}

VOID RecoverPreMode(PETHREAD EThread, CHAR PreMode)
{
    *(PCHAR)((ULONG_PTR)EThread + PreviousModeOffsetOf_KTHREAD) = PreMode;
}



BOOLEAN NtPathToDosPathW(WCHAR* wzFullNtPath,WCHAR* wzFullDosPath)
{
    WCHAR wzDosDevice[4] = {0};
    WCHAR wzNtDevice[64] = {0};
    WCHAR *RetStr = NULL;
    size_t NtDeviceLen = 0;
    short i = 0;
    if(!wzFullNtPath||!wzFullDosPath)
    {
        return FALSE;
    }
    for(i=65;i<26+65;i++)
    {
        wzDosDevice[0] = i;
        wzDosDevice[1] = L':';
        if(NtQueryDosDevice(wzDosDevice,wzNtDevice,64))
        {
            if(wzNtDevice)
            {
                NtDeviceLen = wcslen(wzNtDevice);
                if(!_wcsnicmp(wzNtDevice,wzFullNtPath,NtDeviceLen))
                {
                    wcscpy(wzFullDosPath,wzDosDevice);
                    wcscat(wzFullDosPath,wzFullNtPath+NtDeviceLen);
                    return TRUE;
                }
            }
        }
    }

    return FALSE;
}

ULONG
    NtQueryDosDevice(WCHAR* wzDosDevice,WCHAR* wzNtDevice,
    ULONG ucchMax)
{
    NTSTATUS Status;
    POBJECT_DIRECTORY_INFORMATION ObjectDirectoryInfor;
    OBJECT_ATTRIBUTES oa;
    UNICODE_STRING uniString;
    HANDLE hDirectory;
    HANDLE hDevice;
    ULONG  ulReturnLength;
    ULONG  ulNameLength;
    ULONG  ulLength;
    ULONG       Context;
    BOOLEAN     bRestartScan;
    WCHAR*      Ptr = NULL;
    UCHAR       szBuffer[512] = {0};
    RtlInitUnicodeString (&uniString,L"\\??");
    InitializeObjectAttributes(&oa,
        &uniString,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL); 
    Status = ZwOpenDirectoryObject(&hDirectory,DIRECTORY_QUERY,&oa);
    if(!NT_SUCCESS(Status))
    {
        return 0;
    }
    ulLength = 0;
    if (wzDosDevice != NULL)
    {
        RtlInitUnicodeString (&uniString,(PWSTR)wzDosDevice);
        InitializeObjectAttributes(&oa,&uniString,OBJ_CASE_INSENSITIVE,hDirectory,NULL);
        Status = ZwOpenSymbolicLinkObject(&hDevice,GENERIC_READ,&oa);
        if(!NT_SUCCESS (Status))
        {
            ZwClose(hDirectory);
            return 0;
        }
        uniString.Length = 0;
        uniString.MaximumLength = (USHORT)ucchMax * sizeof(WCHAR);
        uniString.Buffer = wzNtDevice;
        ulReturnLength = 0;
        Status = ZwQuerySymbolicLinkObject (hDevice,&uniString,&ulReturnLength);
        ZwClose(hDevice);
        ZwClose(hDirectory);
        if (!NT_SUCCESS (Status))
        {
            return 0;
        }
        ulLength = uniString.Length / sizeof(WCHAR);
        if (ulLength < ucchMax)
        {
            wzNtDevice[ulLength] = UNICODE_NULL;
            ulLength++;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        bRestartScan = TRUE;
        Context = 0;
        Ptr = wzNtDevice;
        ObjectDirectoryInfor = (POBJECT_DIRECTORY_INFORMATION)szBuffer;
        while (TRUE)
        {
            Status = ZwQueryDirectoryObject(hDirectory,szBuffer,sizeof (szBuffer),TRUE,bRestartScan,&Context,&ulReturnLength);
            if(!NT_SUCCESS(Status))
            {
                if (Status == STATUS_NO_MORE_ENTRIES)
                {
                    *Ptr = UNICODE_NULL;
                    ulLength++;
                    Status = STATUS_SUCCESS;
                }
                else
                {
                    ulLength = 0;
                }
                break;
            }
            if (!wcscmp (ObjectDirectoryInfor->TypeName.Buffer, L"SymbolicLink"))
            {
                ulNameLength = ObjectDirectoryInfor->Name.Length / sizeof(WCHAR);
                if (ulLength + ulNameLength + 1 >= ucchMax)
                {
                    ulLength = 0;
                    break;
                }
                memcpy(Ptr,ObjectDirectoryInfor->Name.Buffer,ObjectDirectoryInfor->Name.Length);
                Ptr += ulNameLength;
                ulLength += ulNameLength;
                *Ptr = UNICODE_NULL;
                Ptr++;
                ulLength++;
            }
            bRestartScan = FALSE;
        }
        ZwClose(hDirectory);
    }
    return ulLength;
}