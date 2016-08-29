// Process.cpp : 实现文件
//

#include "stdafx.h"
#include "ProcessManager.h"
#include "Process.h"
#include "afxdialogex.h"


// CProcess 对话框



IMPLEMENT_DYNAMIC(CProcess, CDialogEx)

CProcess::CProcess(CWnd* pParent /*=NULL*/)
	: CDialogEx(CProcess::IDD, pParent)
{

}

CProcess::~CProcess()
{
}

void CProcess::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TAB_Process, m_Tab_Process);
}


BEGIN_MESSAGE_MAP(CProcess, CDialogEx)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_Process, &CProcess::OnTcnSelchangeTabProcess)
END_MESSAGE_MAP()


// CProcess 消息处理程序



void CProcess::OnTcnSelchangeTabProcess(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: 在此添加控件通知处理程序代码
	ULONG        m_SelectTab = 0;
	m_SelectTab = m_Tab_Process.GetCurSel();
	CRect tabRect;   
	m_Tab_Process.GetClientRect(&tabRect);    // 获取标签控件客户区Rect   
	// 调整tabRect，使其覆盖范围适合放置标签页   
	tabRect.left += 1;                  
	tabRect.right -= 1;   
	tabRect.top += 25;   
	tabRect.bottom -= 1;   
	switch(m_SelectTab)
	{
	case 0:
		{
			EnumProcess.SetWindowPos(NULL, tabRect.left, tabRect.top, tabRect.Width(), tabRect.Height(), SWP_SHOWWINDOW);
			Monitor.SetWindowPos(NULL, tabRect.left, tabRect.top, tabRect.Width(), tabRect.Height(), SWP_HIDEWINDOW);
			if(::MessageBox(NULL,L"确定离开监控吗？",L"Noti",1)==MB_OK)
			{
				Monitor.m_bOk = FALSE;
			}
			break;
		}
	case 1:
		{
			EnumProcess.SetWindowPos(NULL, tabRect.left, tabRect.top, tabRect.Width(), tabRect.Height(), SWP_HIDEWINDOW);
			Monitor.SetWindowPos(NULL, tabRect.left, tabRect.top, tabRect.Width(), tabRect.Height(), SWP_SHOWWINDOW);
			break;
		}
	}
	*pResult = 0;
}


BOOL CProcess::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
	int dpix;
	int dpiy;


	//初始化Tab键
	m_Tab_Process.InsertItem(0, _T("进程信息"));           
	m_Tab_Process.InsertItem(1, _T("进程监控"));   



	//与两个自对话框关联
	EnumProcess.Create(IDD_DIALOG_ENUMPROCESS,GetDlgItem(IDC_TAB_Process));
	Monitor.Create(IDD_DIALOG_MONITOR,GetDlgItem(IDC_TAB_Process));

	CRect tabRect;
	GetWindowRect(&tabRect);
	CPaintDC dc(this);
	dpix = GetDeviceCaps(dc.m_hDC,LOGPIXELSX);
	dpiy = GetDeviceCaps(dc.m_hDC,LOGPIXELSY);
	tabRect.bottom += (LONG)(1+21*(dpiy/96.0));
	MoveWindow(&tabRect);
	m_Tab_Process.GetClientRect(&tabRect);    // 获取标签控件客户区Rect   

	// 调整tabRect，使其覆盖范围适合放置标签页   
	tabRect.left += 1;                  
	tabRect.right -= 1;   
	tabRect.top += 25;   
	tabRect.bottom -= 1;   

	// 根据调整好的tabRect放置m_jzmDlg子对话框，并设置为显示   
	EnumProcess.SetWindowPos(NULL, tabRect.left, tabRect.top, tabRect.Width(), tabRect.Height(), SWP_SHOWWINDOW);
	Monitor.SetWindowPos(NULL, tabRect.left, tabRect.top, tabRect.Width(), tabRect.Height(), SWP_HIDEWINDOW);



	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}
