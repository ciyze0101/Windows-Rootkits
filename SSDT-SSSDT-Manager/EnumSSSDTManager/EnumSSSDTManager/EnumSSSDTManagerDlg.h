
// EnumSSSDTManagerDlg.h : 头文件
//

#pragma once
#include "SSDT.h"
#include "SSSDT.h"

// CEnumSSSDTManagerDlg 对话框
class CEnumSSSDTManagerDlg : public CDialogEx
{
// 构造
public:
	CEnumSSSDTManagerDlg(CWnd* pParent = NULL);	// 标准构造函数

	CSSSDT SSSDTDlg;
	CSSDT  SSDTDlg;
	ULONG       m_CurSelTab;
	CDialog*    m_Dlg[5];
	VOID  CEnumSSSDTManagerDlg::InitTab();

// 对话框数据
	enum { IDD = IDD_ENUMSSSDTMANAGER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CTabCtrl m_Tab;
	afx_msg void OnSelchangeTabSssdt(NMHDR *pNMHDR, LRESULT *pResult);
};
