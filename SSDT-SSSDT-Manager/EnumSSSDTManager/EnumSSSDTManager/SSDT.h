#pragma once
#include "afxcmn.h"
#include <WinIoCtl.h>
#include <vector>
#include "OpenDevice.h"
#include <IMAGEHLP.H>
#pragma comment(lib,"ImageHlp.lib")
using namespace std;
#define ENUM_SSDT 100
#define ENUM_RING3 200

#define IOCTL_GET_SSDT_FUNCTIONADDRESS	CTL_CODE(FILE_DEVICE_UNKNOWN, 0x805, METHOD_NEITHER, FILE_ANY_ACCESS)
#define IOCTL_GET_SSDT_SERVERICE_BASE	CTL_CODE(FILE_DEVICE_UNKNOWN, 0x806, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_GET_SSDT_MODULENAME \
	CTL_CODE(FILE_DEVICE_UNKNOWN,0x841,METHOD_BUFFERED,FILE_ANY_ACCESS)

#define IOCTL_UNHOOK_SSDT \
	CTL_CODE(FILE_DEVICE_UNKNOWN,0x850,METHOD_BUFFERED,FILE_ANY_ACCESS)

#define IOCTL_RESUME_SSDT_INLINEHOOK	CTL_CODE(FILE_DEVICE_UNKNOWN, 0x851, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_GET_SSDT_CURRENT_FUNC_CODE	CTL_CODE(FILE_DEVICE_UNKNOWN, 0x852, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define GET_SSDT_SERVERICE_BASE 80
#define GET_SSDT_SYS_MODULE_INFOR 90
#define GET_SSDT_CURRENT_FUNC_CODE 140

// CSSDT 对话框
typedef struct _SSDT_INFOR
{
	ULONG  FunctionIndex;
	PVOID  CurrentFunctionAddress;
	PVOID  OriginalFunctionAddress;
	char   szFunctionName[60];
	WCHAR  wzModule[60];
	UCHAR  szCurrentFunctionCode[CODE_LENGTH];
	UCHAR  szOriginalFunctionCode[CODE_LENGTH];
}SSDT_INFOR,*PSSDT_INFOR;


typedef struct _SSDT_INFOR_FOR
{
	ULONG  FunctionIndex;
	PVOID  CurrentFunctionAddress;
	PVOID  OriginalFunctionAddress;
	char   szFunctionName[60];
}SSDT_INFOR_FOR,*PSSDT_INFOR_FOR;


typedef struct _SSDT_INFORMATION
{
	ULONG_PTR    NumberOfFunction;
	SSDT_INFOR_FOR SSDT[1];
}SSDT_INFORMATION, *PSSDT_INFORMATION;



class CSSDT : public CDialogEx
{
	DECLARE_DYNAMIC(CSSDT)

public:
	CSSDT(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CSSDT();
	BOOL CSSDT::EnumSSDTInfor();
	VOID CSSDT::AddItemToControlList(SSDT_INFOR_FOR SSDTInfor);

	CString  m_strTempNtosFilePath;
	CString  m_strNtoskFilePath;
	PVOID    m_ServiceTable;
	PVOID    m_ServiceTableBase;
	PVOID    m_NtosModuleBase;
	PVOID    m_TempNtoskModuleBase;
	BOOL     m_bOk;
	BOOL m_ShowHook;
	UCHAR    m_CurrentFunctionCode[CODE_LENGTH];
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

	void CSSDT::GetOriginalSSDTFunctionCode(ULONG_PTR CurrentFunctionAddress, PUCHAR szBuffer, SIZE_T Length)
	{
		ULONG_PTR OriginalFunctionAddress = CurrentFunctionAddress - (ULONG_PTR)m_NtosModuleBase+(ULONG_PTR)m_TempNtoskModuleBase;
		memcpy(szBuffer,(PVOID)OriginalFunctionAddress, Length);
		//memcpy(szBuffer,(PVOID)CurrentFunctionAddress,Length);
	}
	ULONG_PTR CSSDT::GetOriginalSSDTFunctionAddress(ULONG ulIndex);
	BOOL CSSDT::SendIoControlCode(ULONG ulIndex,PVOID* FuntionAddress,ULONG_PTR ulControlCode,WCHAR* wzSysModuleName);
	BOOL CSSDT::MakeTempWin32kFile();
	int CSSDT::FixRelocTable(ULONG_PTR NewModuleBase, ULONG_PTR OriginalModuleBase);
// 对话框数据
	enum { IDD = IDD_DIALOG_SSDT };

	ULONG_PTR RVAToOffset(PIMAGE_NT_HEADERS NTHeader, ULONG_PTR ulRVA);   

	COpenDevice OpenDeviceSSDT;
	BOOL CSSDT::EnumSSDTInfor(PSSDT_INFOR SSDTInfor);
	VOID CSSDT::AddItemToControlList(ULONG SSDTFunctionCount,PSSDT_INFOR SSSDTInfor);
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	vector<SSDT_INFOR_FOR> m_Vector;
	DECLARE_MESSAGE_MAP()
public:
	CListCtrl m_ControlListSSDTInfor;
	virtual BOOL OnInitDialog();
	UINT m_Num1;
	afx_msg void OnBnClickedButtonEnumfunc();
	afx_msg void OnResumeResumessd();
	afx_msg void OnResumeResumeinlinehook();
	afx_msg void OnResumeShowhook();
	afx_msg void OnRclickListSsdt(NMHDR *pNMHDR, LRESULT *pResult);
};
