#include "precomp.h"

#include "main.h"
#include "MemLoadDriver.h"

//卸载例程
VOID DriverUnload(PDRIVER_OBJECT pDriverObject)
{
	KdPrint(("-->%s %d\n", __FUNCTION__, __LINE__));
	
	KdPrint(("<--%s %d\n", __FUNCTION__, __LINE__));
}

//入口点函数
NTSTATUS DriverEntry(PDRIVER_OBJECT pDriverObject, PUNICODE_STRING pRegistryPath)
{
	UNICODE_STRING ustr = { 0 };
	KdPrint(("-->%s %d\n", __FUNCTION__, __LINE__));

	//设置卸载函数
	pDriverObject->DriverUnload = DriverUnload;

	//获取函数地址RtlImageDirectoryEntryToData
	RtlInitUnicodeString(&ustr, L"RtlImageDirectoryEntryToData");
	fun_RtlImageDirectoryEntryToData = (Fun_RtlImageDirectoryEntryToData)MmGetSystemRoutineAddress(&ustr);
	if (fun_RtlImageDirectoryEntryToData == NULL)
	{
		KdPrint(("%s %d: MmGetSystemRoutineAddress RtlImageDirectoryEntryToData failed\n", __FUNCTION__, __LINE__));
		goto End;
	}

	//获取函数地址IoCreateDriver
	RtlInitUnicodeString(&ustr, L"IoCreateDriver");
	fun_IoCreateDriver = (Fun_IoCreateDriver)MmGetSystemRoutineAddress(&ustr);
	if (fun_IoCreateDriver == NULL)
	{
		KdPrint(("%s %d: MmGetSystemRoutineAddress IoCreateDriver failed\n", __FUNCTION__, __LINE__));
		goto End;
	}

	//获取函数地址RtlQueryModuleInformation
	RtlInitUnicodeString(&ustr, L"RtlQueryModuleInformation");
	fun_RtlQueryModuleInformation = (Fun_RtlQueryModuleInformation)MmGetSystemRoutineAddress(&ustr);
	if (fun_RtlQueryModuleInformation == NULL)
	{
		KdPrint(("%s %d: MmGetSystemRoutineAddress RtlQueryModuleInformation failed\n", __FUNCTION__, __LINE__));
		goto End;
	}

	//内存加载驱动模块，文件路径为DRIVER_TO_MEMLOAD，驱动名为DRIVER_NAME
	if (!MemLoadDriverByFilePath(DRIVER_TO_MEMLOAD, DRIVER_NAME))
	{
		KdPrint(("%s %d: MemLoadDriverByFilePath failed\n", __FUNCTION__, __LINE__));
	}
	else
	{
		KdPrint(("MemLoadDriverByFilePath Success!\n"));
	}

End:
	KdPrint(("<--%s %d\n", __FUNCTION__, __LINE__));
	return STATUS_SUCCESS;
}