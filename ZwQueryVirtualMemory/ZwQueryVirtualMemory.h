
#ifndef CXX_ZWQUERYVIRTUALMEMORY_H
#define CXX_ZWQUERYVIRTUALMEMORY_H


#include <ntifs.h>
#include <devioctl.h>
typedef unsigned long DWORD;

NTSTATUS DriverEntry(IN PDRIVER_OBJECT pDriverObj, IN PUNICODE_STRING pRegistryString);

VOID DriverUnload(IN PDRIVER_OBJECT pDriverObj);


typedef enum _MEMORY_INFORMATION_CLASS
{
    MemoryBasicInformation,  //内存基本信息
    MemoryWorkingSetList,
    MemorySectionName        //内存映射文件名信息  
}MEMORY_INFORMATION_CLASS;


typedef NTSTATUS
    (*pfnNtQueryVirtualMemory)(HANDLE ProcessHandle,PVOID BaseAddress,
    MEMORY_INFORMATION_CLASS MemoryInformationClass,
    PVOID MemoryInformation,
    SIZE_T MemoryInformationLength,
    PSIZE_T ReturnLength);

//MemoryBasicInformation 
typedef struct _MEMORY_BASIC_INFORMATION {  
    PVOID       BaseAddress;           //查询内存块所占的第一个页面基地址
    PVOID       AllocationBase;        //内存块所占的第一块区域基地址，小于等于BaseAddress，
    DWORD       AllocationProtect;     //区域被初次保留时赋予的保护属性
    SIZE_T      RegionSize;            //从BaseAddress开始，具有相同属性的页面的大小，
    DWORD       State;                 //页面的状态，有三种可能值MEM_COMMIT、MEM_FREE和MEM_RESERVE
    DWORD       Protect;               //页面的属性，其可能的取值与AllocationProtect相同
    DWORD       Type;                  //该内存块的类型，有三种可能值：MEM_IMAGE、MEM_MAPPED和MEM_PRIVATE
} MEMORY_BASIC_INFORMATION, *PMEMORY_BASIC_INFORMATION;
NTSTATUS EnumMoudleByNtQueryVirtualMemory(ULONG ProcessId);
//MemorySectionName 
typedef struct _MEMORY_SECTION_NAME  {  
    UNICODE_STRING Name;  
    WCHAR     Buffer[260];  
}MEMORY_SECTION_NAME,*PMEMORY_SECTION_NAME;



#endif