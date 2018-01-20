#include "StdAfx.h"
#include "Common.h"


CCommon::CCommon(void)
{
	DeviceHandle = NULL;
}


CCommon::~CCommon(void)
{
}


HANDLE OpenDevice(LPCTSTR wzLinkPath)
{
	HANDLE hDevice = CreateFile(wzLinkPath,
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	if (hDevice == INVALID_HANDLE_VALUE)
	{
	}
	return hDevice;
}