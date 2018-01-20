// Monitor.cpp : 实现文件
//

#include "stdafx.h"
#include "ProcessManager.h"
#include "Common.h"
#include "Monitor.h"
#include "afxdialogex.h"
#include "MonitorProcess.h"
extern CCommon Common;

#define WM_DLG  WM_USER+10
// CMonitor 对话框
HANDLE  g_hEvent[3] = {0};
IMPLEMENT_DYNAMIC(CMonitor, CDialogEx)

CMonitor::CMonitor(CWnd* pParent /*=NULL*/)
	: CDialogEx(CMonitor::IDD, pParent)
	, m_ShowMoni(_T(""))
{

}

CMonitor::~CMonitor()
{
}

void CMonitor::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_ShowMonitor, m_ShowMonitor);
	DDX_Text(pDX, IDC_EDIT_ShowMonitor, m_ShowMoni);
}


BEGIN_MESSAGE_MAP(CMonitor, CDialogEx)
	ON_MESSAGE(WM_DLG,OnNotifyDlg)  
	ON_BN_CLICKED(IDC_BUTTONOPEN, &CMonitor::OnBnClickedButtonopen)
	ON_BN_CLICKED(IDC_BUTTONSUS, &CMonitor::OnBnClickedButtonsus)
	ON_BN_CLICKED(IDC_BUTTON_GOON, &CMonitor::OnBnClickedButtonGoon)
	ON_BN_CLICKED(IDC_BUTTON_STOP, &CMonitor::OnBnClickedButtonStop)
END_MESSAGE_MAP()


// CMonitor 消息处理程序


BOOL CMonitor::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}



DWORD CMonitor::ThreadProc(LPVOID lPParam)
{
	CMonitor* This = (CMonitor*)lPParam;
	DWORD dwReturnSize = 0;
	DWORD dwRet = 0;

	ULONG i = 0;
	for (i=0;i<3;i++)
	{
		g_hEvent[i] = CreateEvent(NULL,TRUE,FALSE,NULL);//创建一个初始未受信，手动受信的事件
	}

	dwRet = DeviceIoControl(Common.DeviceHandle,CTL_MONITORPROCESS,
		g_hEvent,
		sizeof(ULONG_PTR)*3,
		NULL,
		0,
		&dwReturnSize,
		NULL);
	if(dwRet==0)
	{
		::MessageBox(NULL,L"Error",L"Error",1);
	}
	while (This->m_bOk)
	{
		while (WaitForSingleObject(g_hEvent[0],1000)==WAIT_OBJECT_0)
		{
			memset(&This->Msg,0,sizeof(This->Msg));

			dwRet = DeviceIoControl(Common.DeviceHandle,CTL_GETRPROCESSINFOR,
				NULL,
				0,
				&This->Msg,
				sizeof(This->Msg),
				&dwReturnSize,
				NULL);


			if (dwRet==0)
			{


			}

			This->SendMessage(WM_DLG,NULL,NULL);


			Sleep(1);
		}

	}

	return 0;
}



LRESULT CMonitor::OnNotifyDlg(WPARAM wParam,LPARAM lParam)
{
	
	MonitorProcess *dlg = new MonitorProcess(NULL);

	dlg->Create(IDD_DIALOG_MESSAGE,0);


	if (Msg.ulCreate==0)
	{
		CString  strTemp = L"进程销毁\r\n";


		strTemp+=Msg.wzProcessPath;


		dlg->m_EditControl.SetWindowText(strTemp);
	}

	else
	{
		CString  strTemp = L"进程创建\r\n";
		
		strTemp+=Msg.wzProcessPath;
	
		dlg->m_EditControl.SetWindowText(strTemp);
		m_ShowMoni+=strTemp;
		m_ShowMoni+="\r\n";
		UpdateData(FALSE);
	}

	dlg->ShowWindow(SW_SHOW);

	return TRUE;
}


void CMonitor::OnBnClickedButtonopen()
{
	m_bOk = TRUE;
	m_hThread = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)ThreadProc,this,0,&m_dwThreadID);
}


void CMonitor::OnBnClickedButtonsus()
{
	
}


void CMonitor::OnBnClickedButtonGoon()
{
	//TerminateThread()
}


void CMonitor::OnBnClickedButtonStop()
{
	TerminateThread(m_hThread,0);
	BOOL dwRet;
	ULONG dwReturnSize;
	dwRet = DeviceIoControl(Common.DeviceHandle,CTL_SUSMONITOR,
		NULL,
		0,
		NULL,
		0,
		&dwReturnSize,
		NULL);
}
