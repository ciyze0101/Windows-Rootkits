#include "MonitorWin7.h"
#include "EnumProcess.h"
extern ERESOURCE  Resource;
extern KMUTEX  Mutex;
extern PKEVENT  EventArray[3]; 
extern MSG         Msg;
extern BOOLEAN         IsClear;

NTSTATUS EventToKernelEvent(PULONG_PTR InputBuffer,ULONG_PTR InSize)
{
	NTSTATUS   Status = STATUS_SUCCESS;
	PULONG_PTR HandleArray = NULL;
	ULONG i = 0;
	HandleArray = (PULONG_PTR)InputBuffer;   //这里兼容32Bit 与 64 位

	for (i=0;i<InSize;i++)
	{
		if (EventArray[i]!=NULL)
		{
			ObDereferenceObject(EventArray[i]);

			EventArray[i] = NULL;
		}
	}

	for (i=0;i<InSize;i++)
	{
		Status = ObReferenceObjectByHandle((HANDLE)HandleArray[i],
			SYNCHRONIZE,
			*ExEventObjectType,
			KernelMode,
			&EventArray[i],
			NULL);


		//DbgPrint("%p\r\n",EventArray[i]);
		if (!NT_SUCCESS(Status))
		{
			break;
		}
	}

	if (Status!=STATUS_SUCCESS)
	{
		for (i=0;i<InSize;i++)
		{
			if (EventArray[i]!=NULL)
			{
				ObDereferenceObject(EventArray[i]);
				EventArray[i] = NULL;
				DbgPrint("Error\r\n");
			}
		}
	}


	return Status;

}



NTSTATUS  RegisterProcessFilter()
{
	NTSTATUS  Status;
	DbgPrint("register");

#ifdef _WIN64
	Status = PsSetCreateProcessNotifyRoutineEx((PCREATE_PROCESS_NOTIFY_ROUTINE_EX)ProcessCallBackWin7,FALSE);   //添加一个 进程 创建的回调Notity

	if (!NT_SUCCESS(Status))
		{
			DbgPrint("%x",Status);
			return Status;
		}
	return Status;

#else
#endif
}






VOID
	ProcessCallBackWin7(PEPROCESS  EProcess,HANDLE  ProcessId,PPS_CREATE_NOTIFY_INFO  CreateInfo)
{

	NTSTATUS  Status;
	WCHAR  wzProcessPath[512] = {0};
	DbgPrint("CallBack");
	if (CreateInfo)
	{
		KeWaitForSingleObject(&Mutex,Executive,KernelMode,FALSE,NULL);
		if (GetProcessPathBySectionObject(EProcess,wzProcessPath)==TRUE)
		{
			memset(&Msg,0,sizeof(Msg));
			Msg.ulCreate = TRUE;
			memcpy(&Msg.wzProcessPath,wzProcessPath,wcslen(wzProcessPath)*sizeof(WCHAR));

			memset(wzProcessPath,0,sizeof(wzProcessPath));


			AcquireResourceExclusive(&Resource);

			KeSetEvent(EventArray[0],IO_NO_INCREMENT,FALSE);	  
			KeResetEvent(EventArray[0]);
			Status = KeWaitForMultipleObjects(2,
				&EventArray[1],                        
				WaitAny,
				Executive,
				KernelMode,
				FALSE,
				NULL,
				NULL);

			if (Status == 0)   
			{	

			}

			else
			{

				CreateInfo->CreationStatus = STATUS_UNSUCCESSFUL;
			}

			ReleaseResource(&Resource);

		}
		KeReleaseMutex(&Mutex,
			FALSE);
	}
}





//请求一个互斥资源
FORCEINLINE
	VOID
	AcquireResourceExclusive(IN OUT PERESOURCE Resource)
{
	ASSERT(KeGetCurrentIrql() <= APC_LEVEL);
	ASSERT(ExIsResourceAcquiredExclusiveLite(Resource) || !ExIsResourceAcquiredSharedLite(Resource));

	KeEnterCriticalRegion();
	(VOID)ExAcquireResourceExclusiveLite(Resource,TRUE);
}


//释放互斥资源
FORCEINLINE
	VOID
	ReleaseResource(IN OUT PERESOURCE Resource)
{
	ASSERT(KeGetCurrentIrql() <= APC_LEVEL);
	ASSERT(ExIsResourceAcquiredExclusiveLite(Resource) || ExIsResourceAcquiredSharedLite(Resource));

	ExReleaseResourceLite(Resource);
	KeLeaveCriticalRegion();
}



VOID DestroyResource()
{

	ULONG i = 0;
	if (IsClear==FALSE)
	{
		ExDeleteResourceLite(&Resource);


		for (i=0;i<3;i++)
		{
			if (EventArray[i]!=NULL)
			{
				ObDereferenceObject(EventArray[i]);

				EventArray[i] = NULL;
			}
		}

#ifdef _WIN64
		PsSetCreateProcessNotifyRoutineEx(ProcessCallBackWin7,TRUE); 
#else

#endif

		IsClear = TRUE;
	}



}


