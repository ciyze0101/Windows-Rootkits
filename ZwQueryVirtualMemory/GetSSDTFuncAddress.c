#include "GetSSDTFuncAddress.h"
#include "Common.h"

ULONG_PTR   IndexOffset = 0;
extern WIN_VERSION WinVersion;

ULONG_PTR  GetFuncAddress(char* szFuncName)
{
    ULONG_PTR SSDTDescriptor = 0;
    ULONG_PTR ulIndex = 0;
    ULONG_PTR SSDTFuncAddress = 0;

    WinVersion = GetWindowsVersion();

    switch(WinVersion)
    {
#ifdef _WIN64
    case WINDOWS_7_7601:
        {
            SSDTDescriptor = GetKeServiceDescriptorTable64();
            IndexOffset = 4;
            break;
        }
#else
    case WINDOWS_XP:
        {
            SSDTDescriptor = (ULONG_PTR)GetFunctionAddressByName(L"KeServiceDescriptorTable");
            IndexOffset = 1;
            break;
        }
#endif
    default:
        return 0;
    }

    ulIndex = GetSSDTApiFunIndex(szFuncName);
    SSDTFuncAddress =  GetSSDTApiFunAddress(ulIndex,SSDTDescriptor);
    return SSDTFuncAddress;
}



ULONG_PTR GetSSDTApiFunAddress(ULONG_PTR ulIndex,ULONG_PTR SSDTDescriptor)
{
    ULONG_PTR  SSDTFuncAddress = 0;
    switch(WinVersion)
    {
#ifdef _WIN64
    case WINDOWS_7_7601:
        {
            SSDTFuncAddress = GetSSDTFunctionAddress64(ulIndex,SSDTDescriptor);
            break;
        }
#else
    case WINDOWS_XP:
        {
            SSDTFuncAddress = GetSSDTFunctionAddress32(ulIndex,SSDTDescriptor);
            break;
        }
#endif
    default: 
        return 0;
    }

    return 0;
}


ULONG_PTR GetSSDTFunctionAddress32(ULONG_PTR ulIndex,ULONG_PTR SSDTDescriptor)
{
    ULONG_PTR ServiceTableBase= 0 ;
    PSYSTEM_SERVICE_TABLE32 SSDT = (PSYSTEM_SERVICE_TABLE32)SSDTDescriptor;

    ServiceTableBase=(ULONG_PTR)(SSDT ->ServiceTableBase);

    return (ULONG_PTR)(((ULONG*)ServiceTableBase)[(ULONG)ulIndex]);
}



ULONG_PTR GetSSDTFunctionAddress64(ULONG_PTR ulIndex,ULONG_PTR SSDTDescriptor)
{
    LONG dwTemp=0;
    ULONG_PTR qwTemp=0;
    ULONG_PTR ServiceTableBase= 0 ;
    ULONG_PTR FuncAddress =0;
    PSYSTEM_SERVICE_TABLE64 SSDT = (PSYSTEM_SERVICE_TABLE64)SSDTDescriptor;
    ServiceTableBase=(ULONG_PTR)(SSDT ->ServiceTableBase);
    qwTemp = ServiceTableBase + 4 * ulIndex;
    dwTemp = *(PLONG)qwTemp;
    dwTemp = dwTemp>>4;
    FuncAddress = ServiceTableBase + (ULONG_PTR)dwTemp;
    return FuncAddress;
}


LONG GetSSDTApiFunIndex(IN LPSTR lpszFunName)
{
    LONG Index = -1;
    NTSTATUS Status = STATUS_UNSUCCESSFUL;
    PVOID    MapBase = NULL;
    PIMAGE_NT_HEADERS  NtHeader;
    PIMAGE_EXPORT_DIRECTORY ExportTable;
    ULONG*  FunctionAddresses;
    ULONG*  FunctionNames;
    USHORT* FunIndexs;
    ULONG   ulFunIndex;
    ULONG   i;
    CHAR*   FunName;
    SIZE_T  ViewSize=0;
    ULONG_PTR FunAddress;
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
                ExportTable =(IMAGE_EXPORT_DIRECTORY *)((ULONG_PTR)MapBase + NtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);
                FunctionAddresses = (ULONG*)((ULONG_PTR)MapBase + ExportTable->AddressOfFunctions);
                FunctionNames = (ULONG*)((ULONG_PTR)MapBase + ExportTable->AddressOfNames);
                FunIndexs = (USHORT*)((ULONG_PTR)MapBase + ExportTable->AddressOfNameOrdinals);
                for(i = 0; i < ExportTable->NumberOfNames; i++)
                {
                    FunName = (LPSTR)((ULONG_PTR)MapBase + FunctionNames[i]);
                    if (_stricmp(FunName, lpszFunName) == 0) 
                    {
                        ulFunIndex = FunIndexs[i]; 
                        FunAddress = (ULONG_PTR)((ULONG_PTR)MapBase + FunctionAddresses[ulFunIndex]);
                        Index=*(ULONG*)(FunAddress+IndexOffset);
                        break;
                    }
                }
            }
        }__except(EXCEPTION_EXECUTE_HANDLER)
        {
        }
    }

    if (Index == -1)
    {
        DbgPrint("%s Get Index Error\n", lpszFunName);
    }

    ZwUnmapViewOfSection(NtCurrentProcess(), MapBase);
    return Index;
}




ULONG_PTR GetKeServiceDescriptorTable64()
{
    PUCHAR StartSearchAddress = (PUCHAR)__readmsr(0xC0000082);
    PUCHAR EndSearchAddress = StartSearchAddress + 0x500;
    PUCHAR i = NULL;
    UCHAR b1=0,b2=0,b3=0;
    ULONG_PTR Temp = 0;
    ULONG_PTR Address = 0;
    for(i=StartSearchAddress;i<EndSearchAddress;i++)
    {
        if( MmIsAddressValid(i) && MmIsAddressValid(i+1) && MmIsAddressValid(i+2) )
        {
            b1=*i;
            b2=*(i+1);
            b3=*(i+2);
            if( b1==0x4c && b2==0x8d && b3==0x15 ) //4c8d15
            {
                memcpy(&Temp,i+3,4);
                Address = (ULONG_PTR)Temp + (ULONG_PTR)i + 7;
                return Address;
            }
        }
    }
    return 0;
}




NTSTATUS 
    MapFileInUserSpace(IN LPWSTR lpszFileName,IN HANDLE ProcessHandle OPTIONAL,
    OUT PVOID *BaseAddress,
    OUT PSIZE_T ViewSize OPTIONAL)
{
    NTSTATUS Status = STATUS_INVALID_PARAMETER;
    HANDLE   hFile = NULL;
    HANDLE   hSection = NULL;
    OBJECT_ATTRIBUTES oa;
    SIZE_T MapViewSize = 0;
    IO_STATUS_BLOCK Iosb;
    UNICODE_STRING uniFileName;

    if (!lpszFileName || !BaseAddress){
        return Status;
    }

    RtlInitUnicodeString(&uniFileName, lpszFileName);
    InitializeObjectAttributes(&oa,
        &uniFileName,
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
        DbgPrint("ZwCreateFile Failed! Error=%08x\n",Status);
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
        DbgPrint("ZwCreateSection Failed! Error=%08x\n",Status);
        return Status;
    }

    if (!ProcessHandle){
        ProcessHandle = NtCurrentProcess();
    }

    Status = ZwMapViewOfSection(hSection, 
        ProcessHandle, 
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
        DbgPrint("ZwMapViewOfSection Failed! Error=%08x\n",Status);
        return Status;
    }

    return Status;
}
