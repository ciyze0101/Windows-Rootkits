

#ifndef CXX_FILEPROTECTX86_H
#define CXX_FILEPROTECTX86_H


#include <ntifs.h>
#include <devioctl.h>
NTSTATUS
    DriverEntry(IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING RegisterPath);


#include <ntimage.h>


#define SEC_IMAGE 0x01000000

typedef struct _SYSTEM_SERVICE_TABLE32 {
    PVOID   ServiceTableBase;
    PVOID   ServiceCounterTableBase;
    ULONG32 NumberOfServices;
    PVOID   ParamTableBase;
} SYSTEM_SERVICE_TABLE32, *PSYSTEM_SERVICE_TABLE32;



typedef
NTSTATUS
    (*pfnNtSetInformationFile) (
    __in HANDLE FileHandle,
    __out PIO_STATUS_BLOCK IoStatusBlock,
    __in_bcount(Length) PVOID FileInformation,
    __in ULONG Length,
    __in FILE_INFORMATION_CLASS FileInformationClass
    );
NTSTATUS Fake_NtSetInformationFileWinXP(
    __in HANDLE FileHandle,
    __out PIO_STATUS_BLOCK IoStatusBlock,
    __in_bcount(Length) PVOID FileInformation,
    __in ULONG Length,
    __in FILE_INFORMATION_CLASS FileInformationClass
    );


typedef
    NTSTATUS
    (*pfnNtDeleteFile) (
    __in POBJECT_ATTRIBUTES ObjectAttributes
    );


NTSTATUS Fake_NtDeleteFileWinXP(
    __in POBJECT_ATTRIBUTES ObjectAttributes
    );


typedef
NTSTATUS
    (*pfnNtWriteFile) (
    __in HANDLE FileHandle,
    __in_opt HANDLE Event,
    __in_opt PIO_APC_ROUTINE ApcRoutine,
    __in_opt PVOID ApcContext,
    __out PIO_STATUS_BLOCK IoStatusBlock,
    __in_bcount(Length) PVOID Buffer,
    __in ULONG Length,
    __in_opt PLARGE_INTEGER ByteOffset,
    __in_opt PULONG Key
    );
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
    );
typedef
NTSTATUS
(*pfnNtCreateFile) (
    __out PHANDLE FileHandle,
    __in ACCESS_MASK DesiredAccess,
    __in POBJECT_ATTRIBUTES ObjectAttributes,
    __out PIO_STATUS_BLOCK IoStatusBlock,
    __in_opt PLARGE_INTEGER AllocationSize,
    __in ULONG FileAttributes,
    __in ULONG ShareAccess,
    __in ULONG CreateDisposition,
    __in ULONG CreateOptions,
    __in_bcount_opt(EaLength) PVOID EaBuffer,
    __in ULONG EaLength
    );


    NTSTATUS
    Fake_NtCreateFileWinXP (
    __out PHANDLE FileHandle,
    __in ACCESS_MASK DesiredAccess,
    __in POBJECT_ATTRIBUTES ObjectAttributes,
    __out PIO_STATUS_BLOCK IoStatusBlock,
    __in_opt PLARGE_INTEGER AllocationSize,
    __in ULONG FileAttributes,
    __in ULONG ShareAccess,
    __in ULONG CreateDisposition,
    __in ULONG CreateOptions,
    __in_bcount_opt(EaLength) PVOID EaBuffer,
    __in ULONG EaLength
    );

VOID  HookSSDT(ULONG_PTR ulIndex);

VOID HookDelete(ULONG_PTR ulIndex);
VOID HookWrite(ULONG_PTR ulIndex);


VOID
    UnHookSSDTDelete(ULONG_PTR ulIndex);
VOID
    UnHookSSDTWrite(ULONG_PTR ulIndex);
VOID
    UnHookSSDT(ULONG_PTR ulIndex);
VOID WPON();
VOID WPOFF();
LONG GetSSDTApiFunctionIndexFromNtdll(char* szFindFunctionName);
NTSTATUS 
    MapFileInUserSpace(WCHAR* wzFilePath,IN HANDLE hProcess OPTIONAL,
    OUT PVOID *BaseAddress,
    OUT PSIZE_T ViewSize OPTIONAL);



NTSYSAPI
    PIMAGE_NT_HEADERS
    NTAPI
    RtlImageNtHeader(PVOID Base);

PVOID  GetFunctionAddressByNameFromSSDT(CHAR* szFunctionName,ULONG_PTR SSDTDescriptor);
ULONG_PTR GetFunctionAddressByIndexFromSSDT32(ULONG_PTR ulIndex,ULONG_PTR SSDTDescriptor);
PVOID 
    GetFunctionAddressByNameFromNtosExport(WCHAR *wzFunctionName);





// Kbdclass驱动的名字
#define KBD_DRIVER_NAME  L"\\Driver\\Kbdclass"

typedef struct _C2P_DEV_EXT 
{ 
    // 这个结构的大小
    ULONG NodeSize; 
    // 过滤设备对象
    PDEVICE_OBJECT pFilterDeviceObject;
    // 同时调用时的保护锁
    KSPIN_LOCK IoRequestsSpinLock;
    // 进程间同步处理  
    KEVENT IoInProgressEvent; 
    // 绑定的设备对象
    PDEVICE_OBJECT TargetDeviceObject; 
    // 绑定前底层设备对象
    PDEVICE_OBJECT LowerDeviceObject; 
} C2P_DEV_EXT, *PC2P_DEV_EXT;

extern POBJECT_TYPE IoDriverObjectType;


// Kbdclass驱动的名字
#define KBD_DRIVER_NAME  L"\\Driver\\Kbdclass"

#define  DELAY_ONE_MICROSECOND  (-10)
#define  DELAY_ONE_MILLISECOND (DELAY_ONE_MICROSECOND*1000)
#define  DELAY_ONE_SECOND (DELAY_ONE_MILLISECOND*1000)


// 这个函数是事实存在的，只是文档中没有公开。声明一下
// 就可以直接使用了。
NTSTATUS
    ObReferenceObjectByName(
    PUNICODE_STRING ObjectName,
    ULONG Attributes,
    PACCESS_STATE AccessState,
    ACCESS_MASK DesiredAccess,
    POBJECT_TYPE ObjectType,
    KPROCESSOR_MODE AccessMode,
    PVOID ParseContext,
    PVOID *Object
    );


NTSTATUS 
    c2pDevExtInit( 
    IN PC2P_DEV_EXT devExt, 
    IN PDEVICE_OBJECT pFilterDeviceObject, 
    IN PDEVICE_OBJECT pTargetDeviceObject, 
    IN PDEVICE_OBJECT pLowerDeviceObject ) ;

NTSTATUS 
    c2pAttachDevices( 
    IN PDRIVER_OBJECT DriverObject, 
    IN PUNICODE_STRING RegistryPath 
    ) ;


NTSTATUS c2pDispatchRead( 
    IN PDEVICE_OBJECT DeviceObject, 
    IN PIRP Irp ) ;

NTSTATUS c2pReadComplete( 
    IN PDEVICE_OBJECT DeviceObject, 
    IN PIRP Irp, 
    IN PVOID Context 
    ) ;

NTSTATUS c2pPnP( 
    IN PDEVICE_OBJECT DeviceObject, 
    IN PIRP Irp 
    ) ;

NTSTATUS c2pPower( 
    IN PDEVICE_OBJECT DeviceObject, 
    IN PIRP Irp 
    ) ;

NTSTATUS c2pDispatchGeneral( 
    IN PDEVICE_OBJECT DeviceObject, 
    IN PIRP Irp 
    ) ;


typedef struct _KEYBOARD_INPUT_DATA
{
    USHORT UnitId;
    USHORT MakeCode;
    USHORT Flags;
    USHORT Reserved;
    ULONG ExtraInformation;
}KEYBOARD_INPUT_DATA,*PKEYBOARD_INPUT_DATA;
//Flags可能取值
#define KEY_MAKE 0
#define KEY_BREAK 1
#define KEY_E0  2
#define KEY_E1  4
#define KEY_TERMSRV_SET_LED 8
#define KEY_TERMSRV_SHADOW 0x10
#define KEY_TERMSRV_VKPACKET 0x20

#define  S_SHIFT 1
#define  S_CAPS  2
#define  S_NUM   4

extern 
    UCHAR *
    PsGetProcessImageFileName(
    __in PEPROCESS Process
    );


VOID 
    c2pUnload(IN PDRIVER_OBJECT DriverObject) ;


#endif    

