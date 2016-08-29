#pragma once
#include "afxcmn.h"
#include <WinIoCtl.h>
#include <afxtempl.h>
#include "OpenDevice.h"

// CSSSDT 对话框
#define IOCTL_GET_SSSDT_SERVERICE_BASE	CTL_CODE(FILE_DEVICE_UNKNOWN, 0x802, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_UNHOOK_SSSDT \
	CTL_CODE(FILE_DEVICE_UNKNOWN,0x831,METHOD_BUFFERED,FILE_ANY_ACCESS)

#define IOCTL_RESUME_SSSDT_INLINEHOOK	CTL_CODE(FILE_DEVICE_UNKNOWN, 0x810, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_GET_SSSDT_CURRENT_FUNC_CODE	CTL_CODE(FILE_DEVICE_UNKNOWN, 0x811, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define GET_SSSDT_SERVERICE_BASE 50
#define GET_SYS_MODULE_INFOR 60

#define GET_SSSDT_CURRENT_FUNC_CODE 120




typedef struct _SYSTEM_SERVICE_TABLE{
	PVOID       ServiceTableBase;
	PVOID       ServiceCounterTableBase;
	ULONG_PTR   NumberOfServices;
	PVOID       ParamTableBase;
} SYSTEM_SERVICE_TABLE, *PSYSTEM_SERVICE_TABLE;

typedef struct _SSSDT_INFOR
{
	ULONG  FunctionIndex;
	PVOID  CurrentFunctionAddress;
	PVOID  OriginalFunctionAddress;
	char   szFunctionName[60];
	WCHAR  wzModule[60];
	UCHAR  szCurrentFunctionCode[CODE_LENGTH];
	UCHAR  szOriginalFunctionCode[CODE_LENGTH];
}SSSDT_INFOR,*PSSSDT_INFOR;

class CSSSDT : public CDialogEx
{
	DECLARE_DYNAMIC(CSSSDT)

public:
	CSSSDT(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CSSSDT();
	COpenDevice OpenDeviceCommon;

	CString  m_strTempWin32kFilePath;
	CString  m_strWin32kFilePath;
	PVOID    m_ServiceTable;
	PVOID    m_ServiceTableBase;
	PVOID    m_Win32kModuleBase;
	PVOID    m_TempWin32kModuleBase;
	BOOL     m_bOk;
	UCHAR    m_CurrentFunctionCode[CODE_LENGTH];


	HANDLE CSSSDT::OpenDevice(LPCTSTR wzLinkPath)
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

	BOOL CSSSDT::EnumSSSDTInfor(PSSSDT_INFOR SSSDTInfor);
	VOID CSSSDT::AddItemToControlList(ULONG SSSDTFunctionCount,PSSSDT_INFOR SSSDTInfor);
	ULONG_PTR CSSSDT::GetOriginalSSSDTFunctionAddress(ULONG ulIndex);
	BOOL CSSSDT::SendIoControlCode(ULONG ulIndex,PVOID* FuntionAddress,ULONG_PTR ulControlCode,WCHAR* wzSysModuleName);
	BOOL CSSSDT::MakeTempWin32kFile();
	int CSSSDT::FixRelocTable(ULONG_PTR NewModuleBase, ULONG_PTR OriginalModuleBase);
	BOOL m_ShowHook;

	void CSSSDT::GetOriginalSSSDTFunctionCode(ULONG_PTR CurrentFunctionAddress, PUCHAR szBuffer, SIZE_T Length)
	{
		ULONG_PTR OriginalFunctionAddress = CurrentFunctionAddress - (ULONG_PTR)m_Win32kModuleBase+(ULONG_PTR)m_TempWin32kModuleBase;
		memcpy(szBuffer,(PVOID)OriginalFunctionAddress, Length);
		//memcpy(szBuffer,(PVOID)CurrentFunctionAddress,Length);
	}
// 对话框数据
	enum { IDD = IDD_DIALOG_SSSDT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CListCtrl m_ControlListSSSDTInfor;
	afx_msg void OnBnClickedButtonSssdt();
	virtual BOOL OnInitDialog();
	UINT m_Num2;
	afx_msg void OnResumeResumessd();
	afx_msg void OnRclickListSssdt(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnResumeShowhook();
	afx_msg void OnUpdateIdrMenu(CCmdUI *pCmdUI);
	afx_msg void OnResumeResumeinlinehook();
};


