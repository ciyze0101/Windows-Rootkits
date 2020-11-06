#pragma once

#include <ntifs.h>
#include <ntddk.h>
#include <stdlib.h>
#include <windef.h>
#include <ntimage.h>
#include <intrin.h>

#include "MyDebugPrint.h"
#include "PhysicalMemoryOperation.h"
#include "SectionOperation.h"
#include "HiddenExecute.h"
#include "HiddenCallApiTransfer.h"
#include "HiddenFunctions.h"


#define	DEVICE_NAME	L"\\Device\\KernelHiddenExcute"
#define LINK_NAME	L"\\DosDevices\\Global\\KernelHiddenExcute"

#define IOCTL_TEST	CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS) 

PHIDDEN_PAGE_RECORD g_pHiddenPageRecord = NULL;