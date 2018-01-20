// EnumProcess.cpp : 实现文件
//

#include "stdafx.h"
#include "ProcessManager.h"
#include "CEnumProcess.h"
#include "afxdialogex.h"
#include "resource.h"

// CEnumProcess 对话框
extern CCommon Common;
IMPLEMENT_DYNAMIC(CEnumProcess, CDialogEx)

CEnumProcess::CEnumProcess(CWnd* pParent /*=NULL*/)
	: CDialogEx(CEnumProcess::IDD, pParent)
{
	ProcessInformation = NULL;
}

CEnumProcess::~CEnumProcess()
{
}

void CEnumProcess::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_PROCESS, m_List_EnumProcess);
}


BEGIN_MESSAGE_MAP(CEnumProcess, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON_PSPCIDTABLE, &CEnumProcess::OnBnClickedButtonPspcidtable)
	ON_BN_CLICKED(IDC_BUTTON_ACTIVEPROCESSLINKS, &CEnumProcess::OnBnClickedButtonActiveprocesslinks)
	ON_BN_CLICKED(IDC_BUTTON_OPENPROCESS, &CEnumProcess::OnBnClickedButtonOpenprocess)
	ON_NOTIFY(NM_RCLICK, IDC_LIST_PROCESS, &CEnumProcess::OnRclickListProcess)
	ON_COMMAND(ID_MENU_HIDE, &CEnumProcess::OnMenuHide)
END_MESSAGE_MAP()


// CEnumProcess 消息处理程序


void CEnumProcess::OnBnClickedButtonPspcidtable()
{

	EnumProcess(CTL_ENUMPROCESSPSPCIDTABLE);
}


void CEnumProcess::OnBnClickedButtonActiveprocesslinks()
{
	EnumProcess(CTL_ENUMPROCESSACTIVELIST);
}


void CEnumProcess::OnBnClickedButtonOpenprocess()
{
	EnumProcess(CTL_ENUMPROCESSBYOPENPROCESS);
}

VOID CEnumProcess::EnumProcess(ULONG_PTR Code)
{
	m_List_EnumProcess.DeleteAllItems();
	ULONG_PTR ulCount = 0x1000;
	BOOL bRet = FALSE;
	DWORD ulReturnSize = 0;
	do 
	{
		ULONG_PTR ulSize = 0;
		if(ProcessInformation)
		{
			free(ProcessInformation);
			ProcessInformation = NULL;
		}
		ulSize = sizeof(PROCESS_INFORMATION_OWN)+ulCount*sizeof(PROCESS_INFORMATION_ENTRY);
		ProcessInformation = (PPROCESS_INFORMATION_OWN)malloc(ulSize);
		if(!ProcessInformation)
		{
			break;
		}

		memset(ProcessInformation,0,ulSize);
		bRet = DeviceIoControl(Common.DeviceHandle,Code,
			NULL,
			NULL,
			ProcessInformation,
			ulSize,
			&ulReturnSize,
			NULL);
		ulCount = ProcessInformation->NumberOfEntry + 1000;

	} while (bRet==FALSE&&GetLastError() == ERROR_INSUFFICIENT_BUFFER );
	if (bRet&& ProcessInformation )
	{
		CString Num;
		Num.Format(L"%d",ProcessInformation->NumberOfEntry);
		MessageBox(Num,L"2");
		AddItemToControlList(ProcessInformation);
	}

	if (ProcessInformation)
	{
		free(ProcessInformation);
		ProcessInformation = NULL;
	}
}

VOID CEnumProcess::AddItemToControlList(PPROCESS_INFORMATION_OWN ProcessInformation)
{

	
	int i =0;
	for(i>0;i<ProcessInformation->NumberOfEntry;i++)
	{
		WCHAR wzImageName[50];
// 		WCHAR* DestPoint = wcsrchr(ProcessInformation->Entry[i].ProcessPath,'\\');    //查找最后一次出现   一个字节
// 		if(DestPoint!=NULL)
// 		{
// 			wcscpy(wzImageName,DestPoint);
// 		}
		mbstowcs(wzImageName,ProcessInformation->Entry[i].ProcessName,MAX_PATH);
		m_List_EnumProcess.InsertItem(i,wzImageName);
		CString Pid;
		Pid.Format(L"%d",ProcessInformation->Entry[i].Pid);
		m_List_EnumProcess.SetItemText(i,1,Pid);
		CString Father;
		Father.Format(L"%d",ProcessInformation->Entry[i].ParentId);
		m_List_EnumProcess.SetItemText(i,2,Father);
		m_List_EnumProcess.SetItemText(i,3,ProcessInformation->Entry[i].ProcessPath);
		CString Eprocess;
		Eprocess.Format(L"0x%p",ProcessInformation->Entry[i].Eprocess);
		m_List_EnumProcess.SetItemText(i,4,Eprocess);
	}
}

BOOL CEnumProcess::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化

	m_List_EnumProcess.SetExtendedStyle(LVS_EX_FULLROWSELECT);
	m_List_EnumProcess.InsertColumn(0, L"映像名称", LVCFMT_LEFT, 100);
	m_List_EnumProcess.InsertColumn(1, L"进程ID", LVCFMT_LEFT, 100);
	m_List_EnumProcess.InsertColumn(2, L"父进程ID", LVCFMT_LEFT, 100);
	m_List_EnumProcess.InsertColumn(3, L"映像路径", LVCFMT_LEFT, 300);
	m_List_EnumProcess.InsertColumn(4, L"EPROCESS", LVCFMT_LEFT, 200);
	//m_List_EnumProcess.InsertColumn(5, L"应用层访问", LVCFMT_LEFT, 100);
	//m_List_EnumProcess.InsertColumn(6, L"文件厂商", LVCFMT_LEFT, 100);


	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}


void CEnumProcess::OnRclickListProcess(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	CMenu    popup;
	popup.LoadMenu(IDR_MENU1);               //加载菜单资源
	CMenu*    pM = popup.GetSubMenu(0);                 //获得菜单的子项
	CPoint    p;
	GetCursorPos(&p);
	int    count = pM->GetMenuItemCount();
	if (m_List_EnumProcess.GetSelectedCount() == 0)         //如果没有选中
	{ 
		for (int i = 0;i<count;i++)
		{
			pM->EnableMenuItem(i, MF_BYPOSITION | MF_DISABLED | MF_GRAYED);          //菜单全部变灰
		}
	}
	else{
		POSITION Pos = m_List_EnumProcess.GetFirstSelectedItemPosition(); 
		int iItem = m_List_EnumProcess.GetNextSelectedItem(Pos); 
	//	strProtect = m_ControlListMemoryInfor.GetItemText(iItem,2);  
	//	if(strProtect==L"No Access"||strProtect==L""||strProtect==L"Read"||strProtect==L"ReadExecute")
		//{
		//	pM->EnableMenuItem(3, MF_BYPOSITION | MF_DISABLED | MF_GRAYED);          //内存清零变灰色
		//}
	}
	pM->TrackPopupMenu(TPM_LEFTALIGN, p.x, p.y, this);


	*pResult = 0;
}


void CEnumProcess::OnMenuHide()
{

	ULONG ulReturnSize = 0;
	POSITION Pos = m_List_EnumProcess.GetFirstSelectedItemPosition(); 
	int iItem = m_List_EnumProcess.GetNextSelectedItem(Pos); 
	if (iItem==-1)
	{
		return;
	}
	CString Pid = m_List_EnumProcess.GetItemText(iItem,1);  
	ULONG_PTR ProcessId = 0;
#ifdef _WIN64
	ProcessId=_ttoi64(Pid);
#else
	ProcessId=_ttoi(Pid);
#endif
	BOOL bRet = DeviceIoControl(Common.DeviceHandle,CTL_HIDEPROCESS,
		(PVOID)ProcessId,
		sizeof(ULONG_PTR),
		NULL,
		NULL,
		&ulReturnSize,
		NULL);

}
