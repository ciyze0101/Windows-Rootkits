#include "precomp.h"

#include "main.h"

//入口点函数
NTSTATUS DriverEntry(PDRIVER_OBJECT pDriverObject, PUNICODE_STRING pRegistryPath)
{
	NTSTATUS ntStatus = STATUS_SUCCESS;
	
	KdPrint(("test.sys loaded!\n"));
	
	return ntStatus;
}