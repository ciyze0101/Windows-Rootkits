
// ProcessManagerDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ProcessManager.h"
#include "ProcessManagerDlg.h"
#include "afxdialogex.h"
#include "Resource.h"
#include "Common.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CCommon Common;

int dpix;
int dpiy;

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CProcessManagerDlg 对话框




CProcessManagerDlg::CProcessManagerDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CProcessManagerDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CProcessManagerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TAB1, m_TabMain);
}

BEGIN_MESSAGE_MAP(CProcessManagerDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_MESSAGE(UM_ICONNOTIFY, (LRESULT (__thiscall CWnd::*)(WPARAM,LPARAM))OnIconNotify)  
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB1, &CProcessManagerDlg::OnSelchangeTabMain)
END_MESSAGE_MAP()


// CProcessManagerDlg 消息处理程序

BOOL CProcessManagerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码



	m_TabMain.InsertItem(0,L"进程");
	//m_TabMain.InsertItem(0,L"模块");
	//m_TabMain.InsertItem(0,L"内核钩子");

	
	Process.Create(IDD_DIALOG_PROCESS,GetDlgItem(IDC_TAB1));
	//Process.Create(IDD_DIALOG1,GetDlgItem(IDC_TAB1));

	CRect tabRect;
	GetWindowRect(&tabRect);
	CPaintDC dc(this);
	dpix = GetDeviceCaps(dc.m_hDC,LOGPIXELSX);
	dpiy = GetDeviceCaps(dc.m_hDC,LOGPIXELSY);
	tabRect.bottom += (LONG)(1+21*(dpiy/96.0));
	MoveWindow(&tabRect);
	m_TabMain.GetClientRect(&tabRect);    // 获取标签控件客户区Rect   



	// 调整tabRect，使其覆盖范围适合放置标签页   
	tabRect.left += 1;                  
	tabRect.right -= 1;   
	tabRect.top += 25;   
	tabRect.bottom -= 1;   

	// 根据调整好的tabRect放置m_jzmDlg子对话框，并设置为显示   
	Process.SetWindowPos(NULL, tabRect.left, tabRect.top, tabRect.Width(), tabRect.Height(), SWP_SHOWWINDOW);
	//Process.SetWindowPos(NULL, tabRect.left, tabRect.top, tabRect.Width(), tabRect.Height(), SWP_HIDEWINDOW)

	Common.DeviceHandle = OpenDevice(L"\\\\.\\ProcessManagerLinkName");//ProcessManagerLinkName
	if(Common.DeviceHandle ==(HANDLE)-1)
	{
		MessageBox(L"打开设备失败");
		return FALSE;
	}

	VOID ContructNotifyConData();
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CProcessManagerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CProcessManagerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CProcessManagerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CProcessManagerDlg::OnSelchangeTabMain(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: 在此添加控件通知处理程序代码
	ULONG        m_SelectTab = 0;
	m_SelectTab = m_TabMain.GetCurSel();
//	g_SelectTab = m_SelectTab;
	CRect tabRect;   
	m_TabMain.GetClientRect(&tabRect);    // 获取标签控件客户区Rect   
	// 调整tabRect，使其覆盖范围适合放置标签页   
	tabRect.left += 1;                  
	tabRect.right -= 1;   
	tabRect.top += 25;   
	tabRect.bottom -= 1;   
	switch(m_SelectTab)
	{
	case 0:
		{
			Process.SetWindowPos(NULL, tabRect.left, tabRect.top, tabRect.Width(), tabRect.Height(), SWP_SHOWWINDOW);
			//m_PeTableDlg.SetWindowPos(NULL, tabRect.left, tabRect.top, tabRect.Width(), tabRect.Height(), SWP_HIDEWINDOW);
			//m_PeHeaderDlg.CheckPEHeader();
			break;
		}
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
		{
			//m_PeHeaderDlg.SetWindowPos(NULL, tabRect.left, tabRect.top, tabRect.Width(), tabRect.Height(), SWP_HIDEWINDOW);
			//m_PeTableDlg.SetWindowPos(NULL, tabRect.left, tabRect.top, tabRect.Width(), tabRect.Height(), SWP_SHOWWINDOW);
			//m_PeTableDlg.ShowPeTable(m_szFileData,PeType);
			break;
		}
	}
	*pResult = 0;
}



void CProcessManagerDlg::OnCancel()
{
	// TODO: 在此添加专用代码和/或调用基类

	Shell_NotifyIcon(NIM_DELETE, &m_nid);
	CloseHandle(Common.DeviceHandle);
	CDialogEx::OnCancel();
}


VOID CProcessManagerDlg::ContructNotifyConData()
{
	m_nid.cbSize = sizeof(NOTIFYICONDATA);    
	m_nid.hWnd = m_hWnd;          
	m_nid.uID = IDR_MAINFRAME;     
	m_nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;   
	m_nid.uCallbackMessage = UM_ICONNOTIFY;            
	m_nid.hIcon = m_hIcon;                            
	CString strTemp=L"人弱就要变强，哪能随便被虐.........";       
	lstrcpyn(m_nid.szTip,strTemp, sizeof(m_nid.szTip) / sizeof(m_nid.szTip[0]));
	Shell_NotifyIcon(NIM_ADD, &m_nid);   //显示托盘

}


void CProcessManagerDlg::OnIconNotify(WPARAM wParam, LPARAM lParam)
{
/*	switch ((UINT)lParam)
	{
	case WM_RBUTTONDOWN: 
		CMenu menu;
		menu.LoadMenu(IDR_MENU_NOTIFY);
		CPoint point;
		GetCursorPos(&point);
		SetForegroundWindow();   //设置当前窗口
		menu.GetSubMenu(0)->TrackPopupMenu(
			TPM_LEFTBUTTON|TPM_RIGHTBUTTON, 
			point.x, point.y, this, NULL); 
		PostMessage(WM_USER, 0, 0);
		break;
	}
*/
}