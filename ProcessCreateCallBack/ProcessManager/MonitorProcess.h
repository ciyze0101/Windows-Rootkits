#pragma once
#include "afxwin.h"
#include "resource.h"

// MonitorProcess 对话框

class MonitorProcess : public CDialogEx
{
	DECLARE_DYNAMIC(MonitorProcess)

public:
	MonitorProcess(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~MonitorProcess();
		ULONG   m_ulCount;
// 对话框数据
	enum { IDD = IDD_DIALOG_MESSAGE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CEdit m_EditControl;
	afx_msg void OnBnClickedButtonDeny();
	afx_msg void OnBnClickedButtonAccept();
	virtual BOOL OnInitDialog();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};
