#ifndef CXX_SSSDTMANAGER_H
#	include "SSSDTManager.h"
#include "common.h"
#include "GetService.h"
#include "SSDT.h"
#include "SSSDT.h"
#endif

KIRQL Irql;
WIN_VERSION WinVersion = WINDOWS_UNKNOW;

ULONG_PTR  SSDTDescriptor = 0;
ULONG_PTR  SSSDTDescriptor = 0;

PDRIVER_OBJECT   CurrentDriverObject = NULL;
PVOID            SysModuleBsse    = NULL;
ULONG_PTR        ulSysModuleSize    = 0;

PVOID            SysSSDTModuleBase    = NULL;
ULONG_PTR        ulSSDTSysModuleSize    = 0;
NTSTATUS
DriverEntry(IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING RegistryString)
{
	NTSTATUS		Status = STATUS_SUCCESS;
	ULONG			        i;
	UNICODE_STRING	        uniDeviceName;
	UNICODE_STRING	        uniLinkName;
	PDEVICE_OBJECT	        DeviceObject;
	RtlInitUnicodeString(&uniDeviceName,DEVICE_NAME);
	RtlInitUnicodeString(&uniLinkName,LINK_NAME);

	//创建设备对象;
	Status = IoCreateDevice(DriverObject,0,&uniDeviceName,FILE_DEVICE_UNKNOWN,0,FALSE,&DeviceObject);
	if (!NT_SUCCESS(Status))
	{
		return Status;
	}

	//创建符号链接;
	Status = IoCreateSymbolicLink(&uniLinkName,&uniDeviceName);

	if (!NT_SUCCESS(Status))
	{
		IoDeleteDevice(DeviceObject);
		return Status;
	}

	for (i = 0; i<IRP_MJ_MAXIMUM_FUNCTION; i ++)
	{
		DriverObject->MajorFunction[i] = DefaultPassThrough;
	}
	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = ControlPassThrough;
	DriverObject->DriverUnload = UnloadDriver;

	CurrentDriverObject = DriverObject;
	WinVersion = GetWindowsVersion();

	return Status;
}




NTSTATUS
	DefaultPassThrough(PDEVICE_OBJECT  DeviceObject,PIRP Irp)
{
	Irp->IoStatus.Information = 0;
	Irp->IoStatus.Status = STATUS_SUCCESS;

	IoCompleteRequest(Irp,IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}





NTSTATUS
	ControlPassThrough(PDEVICE_OBJECT  DeviceObject,PIRP Irp)
{
	NTSTATUS  Status = STATUS_SUCCESS;
	PIO_STACK_LOCATION   IrpSp;
	PVOID     InputBuffer  = NULL;
	PVOID     OutputBuffer = NULL;
	ULONG_PTR InputSize  = 0;
	ULONG_PTR OutputSize = 0;
	ULONG_PTR IoControlCode = 0;
	PVOID     SSSDTFunctionAddress = NULL;
	PVOID     SSDTFunctionAddress = NULL;
	WCHAR		wzModuleName[30] = {0};
	WCHAR       wzModuleName2[60] = {0};
	WCHAR       wzModuleName3[60] = {0};
#ifdef _WIN64
	PSYSTEM_SERVICE_TABLE64  SSSDTServiceTable = NULL;
	PSYSTEM_SERVICE_TABLE64  SSDTServiceTable = NULL;
#else
	PSYSTEM_SERVICE_TABLE32  SSSDTServiceTable = NULL;
	PSYSTEM_SERVICE_TABLE32  SSDTServiceTable = NULL;
#endif

	IrpSp = IoGetCurrentIrpStackLocation(Irp);
	
	IoControlCode = IrpSp->Parameters.DeviceIoControl.IoControlCode;

	switch(IoControlCode)
	{
	case IOCTL_GET_SSSDTSERVERICE://获得SSSDT表，通过msr寄存器，对应的标志位4c8d1d，找到之后在越过32(win7)字节就是win7下的SSSDT
		{                         //XP找到wzKeAddSystemServiceTable的地址，然后标示为8d88的位置越过16字节即为XP下的SSSDT

			InputBuffer = OutputBuffer = Irp->AssociatedIrp.SystemBuffer;
			InputSize = IrpSp->Parameters.DeviceIoControl.InputBufferLength;
			OutputSize  = IrpSp->Parameters.DeviceIoControl.OutputBufferLength;
#ifdef _WIN64
			SSSDTDescriptor = (ULONG_PTR)GetKeShadowServiceDescriptorTable64();
#else
			SSSDTDescriptor = (ULONG_PTR)GetKeShadowServiceDescriptorTable32();		
#endif
			if (SSSDTDescriptor == 0)
			{
				Irp->IoStatus.Information = 0;
				Status = Irp->IoStatus.Status = STATUS_UNSUCCESSFUL;
				break;
			}

			Irp->IoStatus.Information = 0;
			Status = Irp->IoStatus.Status = STATUS_SUCCESS;
			break;
		}
	case IOCTL_GET_SSSDT_FUNCTIONADDRESS:
		{
			InputBuffer = OutputBuffer = Irp->AssociatedIrp.SystemBuffer;
			InputSize = IrpSp->Parameters.DeviceIoControl.InputBufferLength;
			OutputSize  = IrpSp->Parameters.DeviceIoControl.OutputBufferLength;

			if (SSSDTDescriptor == 0)
			{
				Irp->IoStatus.Information = 0;
				Status = Irp->IoStatus.Status = STATUS_UNSUCCESSFUL;
				break;
			}
			else
			{
#ifdef _WIN64
				SSSDTFunctionAddress = GetSSSDTFunctionAddress64(*(ULONG*)InputBuffer);
#else
				SSSDTFunctionAddress = GetSSSDTFunctionAddress32(*(ULONG*)InputBuffer);
#endif
				if (SSSDTFunctionAddress!=NULL)
				{
					memcpy(OutputBuffer, &SSSDTFunctionAddress,sizeof(PVOID));	
					Irp->IoStatus.Information = sizeof(PVOID);
					Status = Irp->IoStatus.Status = STATUS_SUCCESS;
					break;
				}
				else
				{
					Irp->IoStatus.Information = 0;
					Status = Irp->IoStatus.Status = STATUS_UNSUCCESSFUL;
					break;
				}
			}
		}
	case  IOCTL_GET_MODULENAME: //SSSDT  Current FuncAddress Of Module
		{
			Data2 Data = {0};
			InputBuffer = OutputBuffer = Irp->AssociatedIrp.SystemBuffer;
			InputSize = IrpSp->Parameters.DeviceIoControl.InputBufferLength;
			OutputSize  = IrpSp->Parameters.DeviceIoControl.OutputBufferLength;

			if (InputBuffer!=NULL)
			{ 
				Data.OriginalFunctionAddress = ((pData2)InputBuffer)->OriginalFunctionAddress;
				//通过当前驱动对象的DriverSection里面枚举模块获得模块名称
				if(GetSysModuleByLdrDataTable1((PVOID)Data.OriginalFunctionAddress,(WCHAR*)wzModuleName2)==TRUE)
				{
					memcpy((WCHAR*)OutputBuffer,wzModuleName2,OutputSize);
					Irp->IoStatus.Information = OutputSize;
					Status = Irp->IoStatus.Status = STATUS_SUCCESS;
					break;
				}
			}

			Irp->IoStatus.Information = 0;
			Status = Irp->IoStatus.Status = STATUS_SUCCESS;
			break;
		}
	case IOCTL_GET_SSSDT_SERVERICE_BASE://Ring3在重定向的时候需要当前模块基址和SSSDT算出偏移
		{
/*
#ifdef _WIN64
			SSSDTDescriptor = GetKeShadowServiceDescriptorTable64();
			SSSDTServiceTable = (PSYSTEM_SERVICE_TABLE64)SSSDTDescriptor;
#else
			SSSDTDescriptor = GetKeShadowServiceDescriptorTable32();
			SSSDTServiceTable = (PSYSTEM_SERVICE_TABLE32)SSSDTDescriptor;
#endif
*/
			InputBuffer = OutputBuffer = Irp->AssociatedIrp.SystemBuffer;
			InputSize = IrpSp->Parameters.DeviceIoControl.InputBufferLength;
			OutputSize  = IrpSp->Parameters.DeviceIoControl.OutputBufferLength;

			if (SSSDTDescriptor == 0)
			{
				Irp->IoStatus.Information = 0;
				Status = Irp->IoStatus.Status = STATUS_UNSUCCESSFUL;
				break;
			}

#ifdef _WIN64
			SSSDTServiceTable = (PSYSTEM_SERVICE_TABLE64)SSSDTDescriptor;
#else
			SSSDTServiceTable = (PSYSTEM_SERVICE_TABLE32)SSSDTDescriptor;
#endif
			memcpy(OutputBuffer,&(SSSDTServiceTable->ServiceTableBase),sizeof(PVOID));
			Irp->IoStatus.Information = sizeof(PVOID);
			Status = Irp->IoStatus.Status = STATUS_SUCCESS;
			break;
		}
	case CTL_GET_SYS_MODULE_INFOR:
		{
			InputBuffer = OutputBuffer = Irp->AssociatedIrp.SystemBuffer;
			InputSize = IrpSp->Parameters.DeviceIoControl.InputBufferLength;
			OutputSize  = IrpSp->Parameters.DeviceIoControl.OutputBufferLength;

			if (InputBuffer!=NULL)
			{
				memcpy(wzModuleName,(WCHAR*)InputBuffer,InputSize);
				if(GetSysModuleByLdrDataTable((WCHAR*)wzModuleName)==TRUE)
				{
					DbgPrint("%x\r\n",SysModuleBsse);
					memcpy((PVOID)OutputBuffer,&SysModuleBsse,sizeof(PVOID));
					memcpy(((PULONG_PTR)OutputBuffer)+1,&ulSysModuleSize,sizeof(ULONG_PTR));
					Irp->IoStatus.Information = sizeof(PVOID)+sizeof(ULONG_PTR);
					Status = Irp->IoStatus.Status = STATUS_SUCCESS;
					break;
				}
			}

			Irp->IoStatus.Information = 0;
			Status = Irp->IoStatus.Status = STATUS_SUCCESS;

			break;
		}
	case IOCTL_GET_SSSDT_CURRENT_FUNC_CODE:
		{
			InputBuffer = OutputBuffer = Irp->AssociatedIrp.SystemBuffer;
			InputSize = IrpSp->Parameters.DeviceIoControl.InputBufferLength;
			OutputSize  = IrpSp->Parameters.DeviceIoControl.OutputBufferLength;

			if (SSSDTDescriptor == 0)
			{
				Irp->IoStatus.Information = 0;
				Status = Irp->IoStatus.Status = STATUS_UNSUCCESSFUL;

				break;
			}
			else
			{

#ifdef _WIN64
				SSSDTFunctionAddress = GetSSSDTFunctionAddress64(*(ULONG*)InputBuffer);
#else
				SSSDTFunctionAddress = GetSSSDTFunctionAddress32(*(ULONG*)InputBuffer);
#endif
				if (SSSDTFunctionAddress!=NULL)
				{
					WPOFF();
					if(SafeCopyMemory(OutputBuffer,(VOID*)SSSDTFunctionAddress,(SIZE_T)OutputSize)==FALSE)
					{
						Irp->IoStatus.Information = 0;
						Status = Irp->IoStatus.Status = STATUS_UNSUCCESSFUL;
						WPON();
						break;
					}
					WPON();
					Irp->IoStatus.Information = OutputSize;
					Status = Irp->IoStatus.Status = STATUS_SUCCESS;
					break;
				}

				Irp->IoStatus.Information = 0;
				Status = Irp->IoStatus.Status = STATUS_UNSUCCESSFUL;
				break;
			}
		}
	case IOCTL_RESUME_SSSDT_INLINEHOOK:
		{
			Data0 Data = {0};
			InputBuffer = OutputBuffer = Irp->AssociatedIrp.SystemBuffer;
			InputSize = IrpSp->Parameters.DeviceIoControl.InputBufferLength;
			OutputSize  = IrpSp->Parameters.DeviceIoControl.OutputBufferLength;
			Data.ulIndex = ((pData0)InputBuffer)->ulIndex;
			memcpy(Data.szOriginalFunctionCode,((pData0)InputBuffer)->szOriginalFunctionCode,CODE_LENGTH);

			if (SSSDTDescriptor == 0)
			{
				Irp->IoStatus.Information = 0;
				Status = Irp->IoStatus.Status = STATUS_UNSUCCESSFUL;

				break;
			}

			ResumeSSSDTInlineHook(Data.ulIndex,Data.szOriginalFunctionCode);

			Irp->IoStatus.Information = 0;
			Status = Irp->IoStatus.Status = STATUS_SUCCESS;

			break;
		}
	case IOCTL_UNHOOK_SSSDT:
		{
			Data1 Data={0};
			InputBuffer = OutputBuffer = Irp->AssociatedIrp.SystemBuffer;
			InputSize = IrpSp->Parameters.DeviceIoControl.InputBufferLength;
			OutputSize  = IrpSp->Parameters.DeviceIoControl.OutputBufferLength;

			Data.Index = ((pData1)InputBuffer)->Index;
			Data.OriginalAddress = ((pData1)InputBuffer)->OriginalAddress;

			if (SSSDTDescriptor == 0)
			{
				Irp->IoStatus.Information = 0;
				Status = Irp->IoStatus.Status = STATUS_UNSUCCESSFUL;
				break;
			}
			else
			{
#ifdef _WIN64
				UnHookSSSDTWin7(Data.Index,Data.OriginalAddress);
#else
				UnHookSSSDTWinXP(Data.Index,Data.OriginalAddress);
#endif

				Irp->IoStatus.Information = 0;
				Status = Irp->IoStatus.Status = STATUS_SUCCESS;
				break;
			}

			break;
		}
	case IOCTL_GET_SSDTSERVERICE:
		{

			InputBuffer = OutputBuffer = Irp->AssociatedIrp.SystemBuffer;
			InputSize = IrpSp->Parameters.DeviceIoControl.InputBufferLength;
			OutputSize  = IrpSp->Parameters.DeviceIoControl.OutputBufferLength;
#ifdef _WIN64
			SSDTDescriptor = (ULONG_PTR)GetKeServiceDescriptorTable64();  //获取SSDT表
#else
			SSDTDescriptor = (ULONG_PTR)GetFunctionAddressByNameFromNtosExport(L"KeServiceDescriptorTable");
#endif

			if (SSDTDescriptor == 0)
			{
				Irp->IoStatus.Information = 0;
				Status = Irp->IoStatus.Status = STATUS_UNSUCCESSFUL;
				break;
			}

			Irp->IoStatus.Information = 0;
			Status = Irp->IoStatus.Status = STATUS_SUCCESS;

			break;

		}
	case IOCTL_GET_SDT_FUNCTIONADDRESS://通过索引获得函数地址
		{
			InputBuffer = OutputBuffer = Irp->AssociatedIrp.SystemBuffer;
			InputSize = IrpSp->Parameters.DeviceIoControl.InputBufferLength;
			OutputSize  = IrpSp->Parameters.DeviceIoControl.OutputBufferLength;

			if (SSDTDescriptor == 0)
			{
				Irp->IoStatus.Information = 0;
				Status = Irp->IoStatus.Status = STATUS_UNSUCCESSFUL;
				break;
			}
			else
			{
#ifdef _WIN64
				//win7在SSDT基地址+4*Index里面存放着偏移，偏移右移4位即是SSDT对应函数地址
				SSDTFunctionAddress = GetSSDTFunctionAddress64(*(ULONG*)InputBuffer,SSDTDescriptor);
#else
				//XP在SSDT基地址+4*Index里面存放的即是SSDT对应函数地址
				SSDTFunctionAddress = GetSSDTFunctionAddress32(*(ULONG*)InputBuffer,SSDTDescriptor);
#endif
				if (SSDTFunctionAddress!=NULL)
				{
					memcpy(OutputBuffer, &SSDTFunctionAddress,sizeof(PVOID));	
					Irp->IoStatus.Information = sizeof(PVOID);
					Status = Irp->IoStatus.Status = STATUS_SUCCESS;
					break;
				}
				else
				{
					Irp->IoStatus.Information = 0;
					Status = Irp->IoStatus.Status = STATUS_UNSUCCESSFUL;
					break;
				}
			}
		}
	case  IOCTL_GET_SSDT_MODULENAME:
		{
			Data2 Data1 = {0};
			InputBuffer = OutputBuffer = Irp->AssociatedIrp.SystemBuffer;
			InputSize = IrpSp->Parameters.DeviceIoControl.InputBufferLength;
			OutputSize  = IrpSp->Parameters.DeviceIoControl.OutputBufferLength;

			if (InputBuffer!=NULL)
			{ 
				Data1.OriginalFunctionAddress = ((pData2)InputBuffer)->OriginalFunctionAddress;
				if(GetSysModuleByLdrDataTable2((PVOID)Data1.OriginalFunctionAddress,(WCHAR*)wzModuleName3)==TRUE)
				{
					memcpy((WCHAR*)OutputBuffer,wzModuleName3,OutputSize);
					Irp->IoStatus.Information = OutputSize;
					Status = Irp->IoStatus.Status = STATUS_SUCCESS;
					break;
				}
			}

			Irp->IoStatus.Information = 0;
			Status = Irp->IoStatus.Status = STATUS_SUCCESS;
			break;
		}
	case IOCTL_GET_SSDT_SERVERICE_BASE:
        {
#ifdef _WIN64
	//	SSDTDescriptor = GetKeServiceDescriptorTable64();
		SSDTServiceTable = (PSYSTEM_SERVICE_TABLE64)SSDTDescriptor;
#else
	//	SSDTDescriptor = (ULONG_PTR)GetFunctionAddressByNameFromNtosExport(L"KeServiceDescriptorTable");
		SSDTServiceTable = (PSYSTEM_SERVICE_TABLE32)SSDTDescriptor;
#endif
		InputBuffer = OutputBuffer = Irp->AssociatedIrp.SystemBuffer;
		InputSize = IrpSp->Parameters.DeviceIoControl.InputBufferLength;
		OutputSize  = IrpSp->Parameters.DeviceIoControl.OutputBufferLength;

		if (SSDTDescriptor == 0)
		{
			Irp->IoStatus.Information = 0;
			Status = Irp->IoStatus.Status = STATUS_UNSUCCESSFUL;
			break;
		}
		memcpy(OutputBuffer,&(SSDTServiceTable->ServiceTableBase),sizeof(PVOID));
		Irp->IoStatus.Information = sizeof(PVOID);
		Status = Irp->IoStatus.Status = STATUS_SUCCESS;
		break;
		}
	case CTL_GET_SSDT_SYS_MODULE_INFOR:
		{
			InputBuffer = OutputBuffer = Irp->AssociatedIrp.SystemBuffer;
			InputSize = IrpSp->Parameters.DeviceIoControl.InputBufferLength;
			OutputSize  = IrpSp->Parameters.DeviceIoControl.OutputBufferLength;

			if (InputBuffer!=NULL)
			{
				memcpy(wzModuleName,(WCHAR*)InputBuffer,InputSize);
				if(GetSysModuleByLdrDataTableSSDT((WCHAR*)wzModuleName)==TRUE)
				{
					memcpy((PVOID)OutputBuffer,&SysSSDTModuleBase,sizeof(PVOID));
					memcpy(((PULONG_PTR)OutputBuffer)+1,&ulSSDTSysModuleSize,sizeof(ULONG_PTR));

					Irp->IoStatus.Information = sizeof(PVOID)+sizeof(ULONG_PTR);
					Status = Irp->IoStatus.Status = STATUS_SUCCESS;
					break;
				}
			}

			Irp->IoStatus.Information = 0;
			Status = Irp->IoStatus.Status = STATUS_SUCCESS;
			break;
		}
	case IOCTL_GET_SSDT_CURRENT_FUNC_CODE:
		{
			InputBuffer = OutputBuffer = Irp->AssociatedIrp.SystemBuffer;
			InputSize = IrpSp->Parameters.DeviceIoControl.InputBufferLength;
			OutputSize  = IrpSp->Parameters.DeviceIoControl.OutputBufferLength;

			if (SSDTDescriptor == 0)
			{
				Irp->IoStatus.Information = 0;
				Status = Irp->IoStatus.Status = STATUS_UNSUCCESSFUL;
				break;
			}
			else
			{
#ifdef _WIN64
				SSDTFunctionAddress = GetSSDTFunctionAddress64(*(ULONG*)InputBuffer,SSDTDescriptor);
#else
				SSDTFunctionAddress = GetSSDTFunctionAddress32(*(ULONG*)InputBuffer,SSDTDescriptor);
#endif
				if (SSDTFunctionAddress!=NULL)
				{
					WPOFF();
					if(SafeCopyMemory(OutputBuffer,(VOID*)SSDTFunctionAddress,(SIZE_T)OutputSize)==FALSE)
					{
						Irp->IoStatus.Information = 0;
						Status = Irp->IoStatus.Status = STATUS_UNSUCCESSFUL;
						WPON();
						break;
					}
					WPON();
					Irp->IoStatus.Information = OutputSize;
					Status = Irp->IoStatus.Status = STATUS_SUCCESS;
					break;
				}

				Irp->IoStatus.Information = 0;
				Status = Irp->IoStatus.Status = STATUS_UNSUCCESSFUL;
			    break;
			}
		}
	case IOCTL_RESUME_SSDT_INLINEHOOK:
		{
			Data0 Data = {0};
			InputBuffer = OutputBuffer = Irp->AssociatedIrp.SystemBuffer;
			InputSize = IrpSp->Parameters.DeviceIoControl.InputBufferLength;
			OutputSize  = IrpSp->Parameters.DeviceIoControl.OutputBufferLength;
	    	Data.ulIndex = ((pData0)InputBuffer)->ulIndex;
			memcpy(Data.szOriginalFunctionCode,((pData0)InputBuffer)->szOriginalFunctionCode,CODE_LENGTH);

			if (SSDTDescriptor == 0)
			{
				Irp->IoStatus.Information = 0;
				Status = Irp->IoStatus.Status = STATUS_UNSUCCESSFUL;

				break;
			}

			ResumeSSDTInlineHook(Data.ulIndex,Data.szOriginalFunctionCode);
			Irp->IoStatus.Information = 0;
			Status = Irp->IoStatus.Status = STATUS_SUCCESS;
			break;
		}
	case IOCTL_UNHOOK_SSDT:
		{
			Data1 Data={0};
			InputBuffer = OutputBuffer = Irp->AssociatedIrp.SystemBuffer;
			InputSize = IrpSp->Parameters.DeviceIoControl.InputBufferLength;
			OutputSize  = IrpSp->Parameters.DeviceIoControl.OutputBufferLength;

			Data.Index = ((pData1)InputBuffer)->Index;
			Data.OriginalAddress = ((pData1)InputBuffer)->OriginalAddress;
			if (SSDTDescriptor == 0)
			{
				Irp->IoStatus.Information = 0;
				Status = Irp->IoStatus.Status = STATUS_UNSUCCESSFUL;
				break;
			}
			else
			{
	    		UnHookSSDT(Data.Index,Data.OriginalAddress);
				Irp->IoStatus.Information = 0;
				Status = Irp->IoStatus.Status = STATUS_SUCCESS;
				break;
			}
			break;
		}
	default:
		{
			Irp->IoStatus.Status = STATUS_UNSUCCESSFUL;
			Irp->IoStatus.Information = 0;
			break;
		}
	}

	IoCompleteRequest(Irp,IO_NO_INCREMENT);
	return Status;
}

VOID
UnloadDriver(PDRIVER_OBJECT DriverObject)
{
	UNICODE_STRING  uniLinkName;
	PDEVICE_OBJECT  CurrentDeviceObject;
	PDEVICE_OBJECT  NextDeviceObject;

	RtlInitUnicodeString(&uniLinkName,LINK_NAME);
	IoDeleteSymbolicLink(&uniLinkName);

	if (DriverObject->DeviceObject!=NULL)
	{
		CurrentDeviceObject = DriverObject->DeviceObject;

		while(CurrentDeviceObject!=NULL)
		{
			NextDeviceObject  = CurrentDeviceObject->NextDevice;
			IoDeleteDevice(CurrentDeviceObject);
			CurrentDeviceObject = NextDeviceObject;
		}
	}

	DbgPrint("UnloadDriver\r\n");
}



