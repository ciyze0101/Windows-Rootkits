#pragma once

#include <ntifs.h>
#include <ntddk.h>
#include <WINDEF.H>
#include <tchar.h>
#include <ntimage.h>
#include <ntstrsafe.h>

#define DRIVER_TO_MEMLOAD L"\\??\\C:\\test.sys"
#define DRIVER_NAME L"\\FileSystem\\test"

typedef PVOID(NTAPI *Fun_RtlImageDirectoryEntryToData)(
	IN PVOID Base,
	IN BOOLEAN MappedAsImage,
	IN USHORT DirectoryEntry,
	OUT PULONG Size
	);
Fun_RtlImageDirectoryEntryToData fun_RtlImageDirectoryEntryToData;

typedef NTSTATUS(NTAPI *Fun_IoCreateDriver)(
	IN PUNICODE_STRING DriverName,
	IN PDRIVER_INITIALIZE InitializationFunction
	);
Fun_IoCreateDriver fun_IoCreateDriver;

typedef NTSTATUS(NTAPI *Fun_RtlQueryModuleInformation)(
	ULONG *InformationLength,
	ULONG SizePerModule,
	PVOID InformationBuffer
	);
Fun_RtlQueryModuleInformation fun_RtlQueryModuleInformation;
