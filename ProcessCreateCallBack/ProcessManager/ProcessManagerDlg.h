
// ProcessManagerDlg.h : 头文件
//

#pragma once
#include "afxcmn.h"
#include "ProcessManager.h"
#include "Process.h"


#define UM_ICONNOTIFY   WM_USER+1
// CProcessManagerDlg 对话框
class CProcessManagerDlg : public CDialogEx
{
// 构造
public:
	CProcessManagerDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_PROCESSMANAGER_DIALOG };
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持
	CProcess Process;
	NOTIFYICONDATA  m_nid;
	VOID CProcessManagerDlg::ContructNotifyConData();
// 实现
protected:
	HICON m_hIcon;
	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnIconNotify(WPARAM wParam,LPARAM lParam);
	DECLARE_MESSAGE_MAP()
public:
	CTabCtrl m_TabMain;
	afx_msg void OnSelchangeTabMain(NMHDR *pNMHDR, LRESULT *pResult);
	virtual void OnCancel();
};
