// MonitorProcess.cpp : 实现文件
//

#include "stdafx.h"
#include "ProcessManager.h"
#include "MonitorProcess.h"
#include "afxdialogex.h"
#include "Monitor.h"

// MonitorProcess 对话框

extern HANDLE g_hEvent[3];

IMPLEMENT_DYNAMIC(MonitorProcess, CDialogEx)

MonitorProcess::MonitorProcess(CWnd* pParent /*=NULL*/)
	: CDialogEx(MonitorProcess::IDD, pParent)
{

}

MonitorProcess::~MonitorProcess()
{
}

void MonitorProcess::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT, m_EditControl);
}


BEGIN_MESSAGE_MAP(MonitorProcess, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON_DENY, &MonitorProcess::OnBnClickedButtonDeny)
	ON_BN_CLICKED(IDC_BUTTON_ACCEPT, &MonitorProcess::OnBnClickedButtonAccept)
	ON_WM_TIMER()
END_MESSAGE_MAP()


// MonitorProcess 消息处理程序


void MonitorProcess::OnBnClickedButtonDeny()
{
	// TODO: 在此添加控件通知处理程序代码
	if (g_hEvent[2]!=NULL)
	{
		SetEvent(g_hEvent[2]);

		ResetEvent(g_hEvent[2]);
	}

	OnOK();
}


void MonitorProcess::OnBnClickedButtonAccept()
{
	// TODO: 在此添加控件通知处理程序代码

	if (g_hEvent[1]!=NULL)
	{
		SetEvent(g_hEvent[1]);

		ResetEvent(g_hEvent[1]);
	}


	OnOK();
}


BOOL MonitorProcess::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化

	m_ulCount = 3;

	ModifyStyleEx(WS_EX_APPWINDOW,WS_EX_TOOLWINDOW);//设置扩展工具窗模式。阻止任务栏显示图标
	CRect rectWorkArea;
	SystemParametersInfoW(SPI_GETWORKAREA,0,&rectWorkArea,SPIF_SENDCHANGE);

	//获得对话框大小
	CRect rectDlg;
	GetWindowRect(&rectDlg);
	int nW = rectDlg.Width();
	int nH = rectDlg.Height();

	//将窗口设置到右下脚
	::SetWindowPos(this->m_hWnd,HWND_BOTTOM,
		rectWorkArea.right-nW-6,rectWorkArea.bottom-nH,
		nW,nH,
		SWP_NOZORDER);

	SetTimer(0,1000,NULL);

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}


void MonitorProcess::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值


	switch (nIDEvent)
	{
	case 0://超时则自动放行
		{
			if(m_ulCount==0)
			{
				KillTimer(nIDEvent);


				if (g_hEvent[1]!=NULL)
				{
					SetEvent(g_hEvent[1]);

					ResetEvent(g_hEvent[1]);
				}
				SendMessage(WM_CLOSE);
					break;
			}
			
			m_ulCount--;
		
		}
		
	}
	CDialogEx::OnTimer(nIDEvent);
}
