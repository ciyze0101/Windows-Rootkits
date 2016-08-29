

#ifndef CXX_PROCESSMANAGERRING0_H
#	include "ProcessManagerRing0.h"
#include "EnumProcess.h"
#include "HideProcess.h"
#include "MonitorWin7.h"
#endif




WIN_VERSION WinVersion = WINDOWS_UNKNOW;
ULONG_PTR ProcessIdOfEprocess = 0;
ULONG_PTR ProcessImageNameOfEprocess = 0;
ULONG_PTR ObjectTableOffsetOf_EPROCESS = 0;


ULONG_PTR ulProcessParametersOfPeb = 0;
ULONG_PTR ulImagePathNameOfProcessParameters = 0;


ULONG_PTR Active_List = 0;

ULONG_PTR ObjectHeaderSize = 0;
ULONG_PTR ObjectTypeOffsetOf_Object_Header =0;

ULONG_PTR PebOfEProcess = 0;
ULONG_PTR FatherOfEprocess = 0;

ULONG_PTR PspCidTable = 0;

PEPROCESS g_EProcess = NULL;
MSG         Msg = {0};
PKEVENT         EventArray[3] = {0}; 

ERESOURCE  Resource;
KMUTEX  Mutex;

BOOLEAN         IsClear = FALSE;
NTSTATUS
DriverEntry(IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING pRegistryString)
{
	NTSTATUS		status = STATUS_SUCCESS;
	UNICODE_STRING  ustrLinkName;
	UNICODE_STRING  ustrDevName;  
	PDEVICE_OBJECT  pDevObj;
	int i = 0;

	for (i=0;i<IRP_MJ_MAXIMUM_FUNCTION;i++)
	{
		DriverObject->MajorFunction[i] = DefaultPassThrough;
	}
	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DispatchDeviceControl;

	// Unload routine
	DriverObject->DriverUnload = DriverUnload;

	// Initialize the device name.
	RtlInitUnicodeString(&ustrDevName, DEVICE_NAME);

	// Create the device object and device extension
	status = IoCreateDevice(DriverObject, 
				0,
				&ustrDevName, 
				FILE_DEVICE_UNKNOWN,
				0,
				FALSE,
				&pDevObj);

	if(!NT_SUCCESS(status))
	{
		DbgPrint("[ProcessManagerRing0] Error, IoCreateDevice = 0x%x\r\n", status);
		return status;
	}

	RtlInitUnicodeString(&ustrLinkName,LINK_NAME);

	// Create a symbolic link to allow USER applications to access it. 
	status = IoCreateSymbolicLink(&ustrLinkName, &ustrDevName);  
	
	if(!NT_SUCCESS(status))
	{
		DbgPrint("[ProcessManagerRing0] Error, IoCreateSymbolicLink = 0x%x\r\n", status);
		
		IoDeleteDevice(pDevObj); 
		return status;
	}	


	WinVersion = GetWindowsVersion();
	switch(WinVersion)
	{
	case WINDOWS_XP:
		{
			ProcessIdOfEprocess = 0x84;
			ProcessImageNameOfEprocess = 0x174;

			ObjectTableOffsetOf_EPROCESS = 0x200;
			
			ulProcessParametersOfPeb = 0x10;
			ulImagePathNameOfProcessParameters = 0x38;

			ObjectHeaderSize = 0x18;
			ObjectTypeOffsetOf_Object_Header = 0x8;

			Active_List = 0x88;

			PebOfEProcess = 0x1b0;

			FatherOfEprocess = 0x14c;
			break;
		}
	case WINDOWS_7:
		{
			ProcessIdOfEprocess  = 0x180;
			ProcessImageNameOfEprocess = 0x2e0;
			ObjectTableOffsetOf_EPROCESS = 0x200;		

			ulProcessParametersOfPeb = 0x020;
			ulImagePathNameOfProcessParameters = 0x060;
			Active_List = 0x188;

			ObjectHeaderSize = 0x30;
			
			PebOfEProcess = 0x338;
			FatherOfEprocess = 0x290; 
			break;

		}
	}

	PspCidTable = GetPspCidTableAddress();
	g_EProcess = PsGetCurrentProcess();

	return STATUS_SUCCESS;
}

VOID
DriverUnload(IN PDRIVER_OBJECT DriverObject)
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
	DestroyResource();
	return;
}




NTSTATUS 
DispatchDeviceControl(IN PDEVICE_OBJECT DeviceObject, IN PIRP pIrp)
{
	NTSTATUS status               = STATUS_INVALID_DEVICE_REQUEST;	 // STATUS_UNSUCCESSFUL
	PIO_STACK_LOCATION pIrpStack  = IoGetCurrentIrpStackLocation(pIrp);
	ULONG uIoControlCode          = 0;
	PVOID InputBuffer			  = NULL;
	PVOID OutputBuffer               = NULL;
	ULONG uInSize                 = 0;
	ULONG uOutSize                = 0;

	// Get the IoCtrl Code
	uIoControlCode = pIrpStack->Parameters.DeviceIoControl.IoControlCode;

	InputBuffer = pIrpStack->Parameters.DeviceIoControl.Type3InputBuffer;
	uInSize = pIrpStack->Parameters.DeviceIoControl.InputBufferLength;
	uOutSize = pIrpStack->Parameters.DeviceIoControl.OutputBufferLength;

	OutputBuffer = pIrp->UserBuffer;

		DbgPrint("[ProcessManagerRing0jjjjjj]!\r\n");
	switch(uIoControlCode)
	{
		case CTL_ENUMPROCESSBYOPENPROCESS:
			{			
				DbgPrint("[ProcessManagerRing0] EnumByOpen\r\n");
				
				if (!MmIsAddressValid(OutputBuffer))
				{
					pIrp->IoStatus.Status = STATUS_UNSUCCESSFUL;
					pIrp->IoStatus.Information = 0;
					break;
				}
				__try
				{
					ProbeForWrite(OutputBuffer,uOutSize,sizeof(PVOID));
					status = EnumProcessByOpenProcess((PVOID)OutputBuffer,uOutSize);      
					pIrp->IoStatus.Information = 0;    
					status = pIrp->IoStatus.Status = status;
				}
				__except(EXCEPTION_EXECUTE_HANDLER)
				{
						
					pIrp->IoStatus.Information = 0;
					status = pIrp->IoStatus.Status = STATUS_UNSUCCESSFUL;
				}
				pIrp->IoStatus.Information = 0;
				status = pIrp->IoStatus.Status = status;
				break;
			}
		case CTL_ENUMPROCESSACTIVELIST:
			{
				DbgPrint("[ProcessManagerRing0] EnumByProcessActiveList!\r\n");

				if (!MmIsAddressValid(OutputBuffer))
				{
					pIrp->IoStatus.Status = STATUS_UNSUCCESSFUL;
					pIrp->IoStatus.Information = 0;
					break;
				}
				__try
				{
					ProbeForWrite(OutputBuffer,uOutSize,sizeof(PVOID));
					status = EnumProcessByActiveList((PPROCESS_INFORMATION_OWN)OutputBuffer,uOutSize);
					pIrp->IoStatus.Information = 0;    
					status = pIrp->IoStatus.Status = status;
				}
				__except(EXCEPTION_EXECUTE_HANDLER)
				{

					pIrp->IoStatus.Information = 0;
					status = pIrp->IoStatus.Status = STATUS_UNSUCCESSFUL;
				}
				pIrp->IoStatus.Information = 0;
				status = pIrp->IoStatus.Status = status;
				break;
			}
		case CTL_ENUMPROCESSPSPCIDTABLE:
			{
				DbgPrint("[ProcessManagerRing0] EnumByPspCidTable!\r\n");

				if (!MmIsAddressValid(OutputBuffer))
				{
					pIrp->IoStatus.Status = STATUS_UNSUCCESSFUL;
					pIrp->IoStatus.Information = 0;
					break;
				}
				__try
				{
					ProbeForWrite(OutputBuffer,uOutSize,sizeof(PVOID));
					status = EnumProcessByPspCidTable((PPROCESS_INFORMATION_OWN)OutputBuffer,uOutSize);
					pIrp->IoStatus.Information = 0;    
					status = pIrp->IoStatus.Status = status;
				}
				__except(EXCEPTION_EXECUTE_HANDLER)
				{

					pIrp->IoStatus.Information = 0;
					status = pIrp->IoStatus.Status = STATUS_UNSUCCESSFUL;
				}
				pIrp->IoStatus.Information = 0;
				status = pIrp->IoStatus.Status = status;
				break;
			}
		case CTL_HIDEPROCESS:
			{
				__try
				{

						RemoveNodeFromActiveProcessLinks((ULONG_PTR)InputBuffer);

						DbgPrint("[ProcessManagerRing0]RemovePspCidTable!\r\n");




						DbgPrint("2!\n");

						EraseObjectFromHandleTable1((ULONG_PTR)InputBuffer);
						pIrp->IoStatus.Information = 0;
						status = pIrp->IoStatus.Status = STATUS_SUCCESS;
					
				
			}
			__except(EXCEPTION_EXECUTE_HANDLER)
			{

				DbgPrint("NO\r\n");
				pIrp->IoStatus.Information = 0;
				status = pIrp->IoStatus.Status = STATUS_UNSUCCESSFUL;
			}
				break;
				
			

			
			}
		case CTL_MONITORPROCESS:
			{
				KeInitializeMutant(&Mutex,0);
				ExInitializeResourceLite(&Resource);   //初始化一个互斥资源防止 进程干扰  DriverEntry中初始化
				IsClear = FALSE;
				__try{
					DbgPrint("Jion");
					if (InputBuffer!=NULL&&uInSize==sizeof(ULONG_PTR)*3)
					{		
						DbgPrint("us");
						status = EventToKernelEvent((PULONG_PTR)InputBuffer,uInSize/sizeof(ULONG_PTR));

						if (!NT_SUCCESS(status))
						{
							pIrp->IoStatus.Information = 0;
							status = pIrp->IoStatus.Status = status;

							break;
						}
						if(WinVersion==WINDOWS_7)
						{
						
							status =  RegisterProcessFilter();
							if (!NT_SUCCESS(status))
							{
								DbgPrint("gg");
								DestroyResource();
								pIrp->IoStatus.Information = 0;
								status = pIrp->IoStatus.Status = status;

								break;
							}
						}
						else if(WinVersion == WINDOWS_XP)
						{

						}
					}
					pIrp->IoStatus.Information = 0;
					status = pIrp->IoStatus.Status = STATUS_SUCCESS;
				}
				__except(EXCEPTION_EXECUTE_HANDLER)
				{
					DbgPrint("No Set");
					pIrp->IoStatus.Information = 0;
					status = pIrp->IoStatus.Status = STATUS_UNSUCCESSFUL;
				}

				DbgPrint("madan?");
				break;
			}
		case CTL_GETRPROCESSINFOR:
			{
				__try{
				
					DbgPrint("Print");
					memcpy(OutputBuffer,&Msg,sizeof(MSG));
					pIrp->IoStatus.Information = 0;
					status = pIrp->IoStatus.Status = STATUS_SUCCESS;
				}
				__except(EXCEPTION_EXECUTE_HANDLER)
				{
					DbgPrint("No Get");
					pIrp->IoStatus.Information = 0;
					status = pIrp->IoStatus.Status = STATUS_UNSUCCESSFUL;
				}

				break;
			}
		case  CTL_SUSMONITOR:
			{
				DestroyResource();
				pIrp->IoStatus.Information = 0;
				status = pIrp->IoStatus.Status = STATUS_SUCCESS;
				break;
			}


		default:
			{
				
				pIrp->IoStatus.Status = STATUS_UNSUCCESSFUL;
				pIrp->IoStatus.Information = 0;
				break;
			}
	}

	IoCompleteRequest(pIrp,IO_NO_INCREMENT);
	return status;
}



NTSTATUS
	DefaultPassThrough(PDEVICE_OBJECT  DeviceObject,PIRP Irp)
{
	Irp->IoStatus.Information = 0;
	Irp->IoStatus.Status = STATUS_SUCCESS;
	IoCompleteRequest(Irp,IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}