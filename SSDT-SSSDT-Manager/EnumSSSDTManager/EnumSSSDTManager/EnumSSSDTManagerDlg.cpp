
// EnumSSSDTManagerDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "EnumSSSDTManager.h"
#include "EnumSSSDTManagerDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


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


// CEnumSSSDTManagerDlg 对话框




CEnumSSSDTManagerDlg::CEnumSSSDTManagerDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CEnumSSSDTManagerDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	for (int i=0;i<5;i++)
	{
		m_Dlg[i] = NULL;
	}
}

void CEnumSSSDTManagerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TAB_SSSDT, m_Tab);
}

BEGIN_MESSAGE_MAP(CEnumSSSDTManagerDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_SSSDT, &CEnumSSSDTManagerDlg::OnSelchangeTabSssdt)
END_MESSAGE_MAP()


// CEnumSSSDTManagerDlg 消息处理程序

BOOL CEnumSSSDTManagerDlg::OnInitDialog()
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
	InitTab();



	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}


VOID  CEnumSSSDTManagerDlg::InitTab()
{
	m_Tab.InsertItem(0,_T("SSSDT"));
	m_Tab.InsertItem(1,_T("SSDT"));


//	SSSDTDlg.Create(IDD_DIALOG_SSSDT,&m_Tab);
//	SSDTDlg.Create(IDD_DIALOG_SSDT,&m_Tab);

	SSSDTDlg.Create(IDD_DIALOG_SSSDT,GetDlgItem(IDC_TAB_SSSDT));
	SSDTDlg.Create(IDD_DIALOG_SSDT,GetDlgItem(IDC_TAB_SSSDT));

	m_Dlg[0] = &SSSDTDlg;
	m_Dlg[1] = &SSDTDlg;



	CRect rc;
	m_Tab.GetClientRect(rc);
	rc.top +=20;
	rc.bottom -= 4;
	rc.left += 4;
	rc.right -= 4;
	SSSDTDlg.MoveWindow(rc);
	SSDTDlg.MoveWindow(rc);

	m_Tab.SetCurSel(0);
	SSSDTDlg.ShowWindow(TRUE);
}


void CEnumSSSDTManagerDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CEnumSSSDTManagerDlg::OnPaint()
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
HCURSOR CEnumSSSDTManagerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}





void CEnumSSSDTManagerDlg::OnSelchangeTabSssdt(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: 在此添加控件通知处理程序代码

	for(int i=0;i<2;i++)
	{
		if (m_Dlg[i]!=NULL)
		{
			m_Dlg[i]->ShowWindow(SW_HIDE);
		}

	}
	m_CurSelTab = m_Tab.GetCurSel();


	if (m_Dlg[m_CurSelTab]!=NULL)
	{
		m_Dlg[m_CurSelTab]->ShowWindow(SW_SHOW);
	}
	*pResult = 0;
}
