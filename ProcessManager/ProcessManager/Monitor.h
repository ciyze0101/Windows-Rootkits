#pragma once
#include "resource.h"
#include "afxwin.h"
// CMonitor 对话框

class CMonitor : public CDialogEx
{
	DECLARE_DYNAMIC(CMonitor)

public:
	CMonitor(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CMonitor();

// 对话框数据
	enum { IDD = IDD_DIALOG_MONITOR };
		HANDLE    m_hThread;
		static  DWORD WINAPI  ThreadProc(LPVOID lPParam);
		DWORD     m_dwThreadID;

		BOOL      m_bOk;
		struct  
		{
			ULONG  ulCreate;
			WCHAR  wzProcessPath[512];

		}Msg;


protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	afx_msg LRESULT OnNotifyDlg(WPARAM wParam,LPARAM lParam);
	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	CEdit m_ShowMonitor;
	CString m_ShowMoni;
	afx_msg void OnBnClickedButtonopen();
	afx_msg void OnBnClickedButtonsus();
	afx_msg void OnBnClickedButtonGoon();
	afx_msg void OnBnClickedButtonStop();
};
