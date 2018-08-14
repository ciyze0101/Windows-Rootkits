

#ifndef CXX_FILEPROTECTX64_H
#define CXX_FILEPROTECTX64_H

#include <ntifs.h>
#include <devioctl.h>

NTSTATUS
    DriverEntry(IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING RegisterPath);

VOID UnloadDriver(PDRIVER_OBJECT  DriverObject);




typedef struct _LDR_DATA_TABLE_ENTRY64
{
    LIST_ENTRY64    InLoadOrderLinks;
    LIST_ENTRY64    InMemoryOrderLinks;
    LIST_ENTRY64    InInitializationOrderLinks;
    PVOID            DllBase;
    PVOID            EntryPoint;
    ULONG            SizeOfImage;
    UNICODE_STRING    FullDllName;
    UNICODE_STRING     BaseDllName;
    ULONG            Flags;
    USHORT            LoadCount;
    USHORT            TlsIndex;
    PVOID            SectionPointer;
    ULONG            CheckSum;
    PVOID            LoadedImports;
    PVOID            EntryPointActivationContext;
    PVOID            PatchInformation;
    LIST_ENTRY64    ForwarderLinks;
    LIST_ENTRY64    ServiceTagLinks;
    LIST_ENTRY64    StaticLinks;
    PVOID            ContextInformation;
    ULONG64            OriginalBase;
    LARGE_INTEGER    LoadTime;
} LDR_DATA_TABLE_ENTRY64, *PLDR_DATA_TABLE_ENTRY64;



typedef struct _OBJECT_TYPE_INITIALIZER                                                                                                                                        
{
    UINT16       Length;
    union                                                                                                                                                                       
    {
        UINT8        ObjectTypeFlags;
        struct                                                                                                                                                                
        {
            UINT8        CaseInsensitive : 1;                                                                                     UINT8        UnnamedObjectsOnly : 1;                                                                                  UINT8        UseDefaultObject : 1;                                                                                    UINT8        SecurityRequired : 1;                                                                                    UINT8        MaintainHandleCount : 1;                                                                                 UINT8        MaintainTypeList : 1;                                                                                    UINT8        SupportsObjectCallbacks : 1;                                                                                                                         
        };
    };
    ULONG32      ObjectTypeCode;
    ULONG32      InvalidAttributes;
    struct _GENERIC_MAPPING GenericMapping;                                                                                                                                     
    ULONG32      ValidAccessMask;
    ULONG32      RetainAccess;
    enum _POOL_TYPE PoolType;
    ULONG32      DefaultPagedPoolCharge;
    ULONG32      DefaultNonPagedPoolCharge;
    PVOID        DumpProcedure;
    PVOID        OpenProcedure;
    PVOID         CloseProcedure;
    PVOID         DeleteProcedure;
    PVOID         ParseProcedure;
    PVOID        SecurityProcedure;
    PVOID         QueryNameProcedure;
    PVOID         OkayToCloseProcedure;
}OBJECT_TYPE_INITIALIZER, *POBJECT_TYPE_INITIALIZER;


typedef struct _OBJECT_TYPE_TEMP                   
{
    struct _LIST_ENTRY TypeList;             
    struct _UNICODE_STRING Name;             
    VOID*        DefaultObject;
    UINT8        Index;
    UINT8        _PADDING0_[0x3];
    ULONG32      TotalNumberOfObjects;
    ULONG32      TotalNumberOfHandles;
    ULONG32      HighWaterNumberOfObjects;
    ULONG32      HighWaterNumberOfHandles;
    UINT8        _PADDING1_[0x4];
    struct _OBJECT_TYPE_INITIALIZER TypeInfo; 
    ULONG64 TypeLock;          
    ULONG32      Key;
    UINT8        _PADDING2_[0x4];
    struct _LIST_ENTRY CallbackList;        
}OBJECT_TYPE_TEMP, *POBJECT_TYPE_TEMP;



VOID EnableObType(POBJECT_TYPE ObjectType);
UNICODE_STRING  GetFilePathByFileObject(PVOID FileObject);
OB_PREOP_CALLBACK_STATUS PreCallBack(PVOID RegistrationContext, POB_PRE_OPERATION_INFORMATION OperationInformation);
NTSTATUS ProtectFileByObRegisterCallbacks();


#endif    




