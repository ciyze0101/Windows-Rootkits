// SSSDT.cpp : 实现文件
//

#include "stdafx.h"
#include "EnumSSSDTManager.h"
#include "SSSDT.h"
#include "SSSDTFunc.h"
#include "afxdialogex.h"


// CSSSDT 对话框


HANDLE g_hDeviceSSS = NULL;
SSSDT_INFOR  SSSDTInfor[0x1000] = {0};
IMPLEMENT_DYNAMIC(CSSSDT, CDialogEx)

CSSSDT::CSSSDT(CWnd* pParent /*=NULL*/)
	: CDialogEx(CSSSDT::IDD, pParent)
	, m_Num2(0)
{
	m_ServiceTable         = 0;
	m_ServiceTableBase     = 0;
	m_Win32kModuleBase     = 0 ;
	m_TempWin32kModuleBase = 0;
	m_bOk = FALSE;
	m_ShowHook = FALSE;
	memset(m_CurrentFunctionCode,0,CODE_LENGTH);
}

CSSSDT::~CSSSDT()
{
}

void CSSSDT::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_SSSDT, m_ControlListSSSDTInfor);
	DDX_Text(pDX, IDC_EDIT_NUM2, m_Num2);
}


BEGIN_MESSAGE_MAP(CSSSDT, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON_SSSDT, &CSSSDT::OnBnClickedButtonSssdt)
	ON_COMMAND(ID_RESUME_RESUMESSD, &CSSSDT::OnResumeResumessd)
	ON_NOTIFY(NM_RCLICK, IDC_LIST_SSSDT, &CSSSDT::OnRclickListSssdt)
	ON_COMMAND(ID_RESUME_SHOWHOOK, &CSSSDT::OnResumeShowhook)
	ON_UPDATE_COMMAND_UI(IDR_MENU, &CSSSDT::OnUpdateIdrMenu)
	ON_COMMAND(ID_RESUME_RESUMEINLINEHOOK, &CSSSDT::OnResumeResumeinlinehook)
END_MESSAGE_MAP()


// CSSSDT 消息处理程序




//初始化List Control
BOOL CSSSDT::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化

	m_ControlListSSSDTInfor.SetExtendedStyle(LVS_EX_FULLROWSELECT);
	m_ControlListSSSDTInfor.InsertColumn(0, L"序号", LVCFMT_LEFT, 100);
	m_ControlListSSSDTInfor.InsertColumn(1, L"函数名称", LVCFMT_LEFT, 200);
	m_ControlListSSSDTInfor.InsertColumn(2, L"当前地址", LVCFMT_LEFT, 100);
	m_ControlListSSSDTInfor.InsertColumn(3, L"状态",LVCFMT_LEFT,80);
	m_ControlListSSSDTInfor.InsertColumn(4, L"原始地址", LVCFMT_LEFT, 100);
	m_ControlListSSSDTInfor.InsertColumn(5, L"当前地址所属模块",LVCFMT_LEFT,300);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}


void CSSSDT::OnBnClickedButtonSssdt()
{
	// TODO: 在此添加控件通知处理程序代
	OpenDeviceCommon.g_hDevice =  OpenDeviceCommon.OpenDevice(L"\\\\.\\SSSDTManagerLink");
	if (OpenDeviceCommon.g_hDevice==(HANDLE)-1)
	{
		MessageBox(L"打开设备失败");

		return;
	}

	if(OpenDeviceCommon.SendIoControlCode(0,NULL,INITIALIZE)==FALSE)//在Ring0获得KeServiceDescriptorTableShadow表地址
	{
		CloseHandle(OpenDeviceCommon.g_hDevice);
		return;
	}


	EnumSSSDTInfor(SSSDTInfor);
	
	CloseHandle(OpenDeviceCommon.g_hDevice);
}


BOOL CSSSDT::EnumSSSDTInfor(PSSSDT_INFOR SSSDTInfor)
{
	m_ControlListSSSDTInfor.DeleteAllItems();
	ULONG_PTR SSSDTFunctionCount = 0;
	ULONG_PTR HookedFunctionCount = 0;


	PVOID SSSDTOriAddr = 0;
	PVOID CurrentFunctionAddress = 0;
	PVOID OriginalFunctionAddress = 0;
	WCHAR wzModuleName[60]= {0};

	int i = 0;
#ifdef _WIN64



	for (i=0;i<sizeof(szWin7FunctionName)/100;i++)
	{
		
		
		OpenDeviceCommon.SendIoControlCode(i,&CurrentFunctionAddress,GET_SSSDT_CURRENT_FUNC_ADDR);
	
		SendIoControlCode(i,&CurrentFunctionAddress,GET_MODULE_NAME,wzModuleName);
		OriginalFunctionAddress = (PVOID)GetOriginalSSSDTFunctionAddress(i);

		SSSDTInfor[SSSDTFunctionCount].FunctionIndex = i;
		SSSDTInfor[SSSDTFunctionCount].CurrentFunctionAddress = CurrentFunctionAddress;
		SSSDTInfor[SSSDTFunctionCount].OriginalFunctionAddress = OriginalFunctionAddress;
		strcpy(SSSDTInfor[SSSDTFunctionCount].szFunctionName,szWin7FunctionName[i]);
		wcscpy(SSSDTInfor[SSSDTFunctionCount].wzModule,wzModuleName);


		if (CurrentFunctionAddress!=OriginalFunctionAddress)
		{
			HookedFunctionCount++;
		}

		SSSDTFunctionCount++;
	}
#else
	for (i=0;i<sizeof(szWinXPFunctionName)/100;i++)
	{


		OpenDeviceCommon.SendIoControlCode(i,&CurrentFunctionAddress,GET_SSSDT_CURRENT_FUNC_ADDR);
		SendIoControlCode(i,&CurrentFunctionAddress,GET_MODULE_NAME,wzModuleName);
		OriginalFunctionAddress = (PVOID)GetOriginalSSSDTFunctionAddress(i);

		SSSDTInfor[SSSDTFunctionCount].FunctionIndex = i;
		SSSDTInfor[SSSDTFunctionCount].CurrentFunctionAddress = CurrentFunctionAddress;
		SSSDTInfor[SSSDTFunctionCount].OriginalFunctionAddress = OriginalFunctionAddress;
		strcpy(SSSDTInfor[SSSDTFunctionCount].szFunctionName,szWinXPFunctionName[i]);
		wcscpy(SSSDTInfor[SSSDTFunctionCount].wzModule,wzModuleName);

		if (CurrentFunctionAddress!=OriginalFunctionAddress)
		{
			HookedFunctionCount++;
		}


		SSSDTFunctionCount++;
	}


#endif

	AddItemToControlList(SSSDTFunctionCount,SSSDTInfor);
	m_Num2 = SSSDTFunctionCount;
	UpdateData(FALSE);

	return TRUE;
}



VOID CSSSDT::AddItemToControlList(ULONG SSSDTFunctionCount,PSSSDT_INFOR SSSDTInfor)
{
	
	int i = 0;
	CString strIndex;
	BOOL bHooked = FALSE;
	for (i=0;i<SSSDTFunctionCount;i++)
	{

		strIndex.Format(L"%d",SSSDTInfor[i].FunctionIndex);
		

		CString strFunctionName(SSSDTInfor[i].szFunctionName);
		

		CString strCurrentAddress;
		strCurrentAddress.Format(L"0x%p",SSSDTInfor[i].CurrentFunctionAddress);
	
		CString strOriginalAddress;
		strOriginalAddress.Format(L"0x%p",SSSDTInfor[i].OriginalFunctionAddress);
		

		
	
		CString strType;
		if (SSSDTInfor[i].OriginalFunctionAddress!=SSSDTInfor[i].CurrentFunctionAddress)
		{
		//	m_ControlListSSSDTInfor.SetItemData(n,1);

			strType = L"SSSDTHook";

			
			bHooked = TRUE;
		}
		else
		{

			//获得原始代码
		


			GetOriginalSSSDTFunctionCode((ULONG_PTR)SSSDTInfor[i].CurrentFunctionAddress,SSSDTInfor[i].szOriginalFunctionCode, CODE_LENGTH);
			//GetOriginalSSSDTFunctionCode((ULONG_PTR)SSSDTInfor[i].OriginalFunctionAddress,SSSDTInfor[i].szOriginalFunctionCode, CODE_LENGTH);

			
			//获得当前代码
			if(SendIoControlCode(i,NULL,GET_SSSDT_CURRENT_FUNC_CODE,NULL)==TRUE)
			{
			
				memcpy(SSSDTInfor[i].szCurrentFunctionCode,m_CurrentFunctionCode,CODE_LENGTH);
				memset(m_CurrentFunctionCode,0,CODE_LENGTH);

				int j = 0;
				for (j=0;j<CODE_LENGTH;j++)
				{

					if (SSSDTInfor[i].szOriginalFunctionCode[j]!=SSSDTInfor[i].szCurrentFunctionCode[j])
					{

						bHooked = TRUE;

						break;
					}
				}
			}
			if (bHooked==TRUE)
			{
		//		m_ControlListSSSDTInfor.SetItemData(n,2);
				strType = L"SSSDTInlineHook";
			}
		
		}

		if (bHooked==FALSE)
		{
			//	m_ControlListSSSDTInfor.SetItemData(n,0);
			strType = L"";
		}

	
		if(m_ShowHook==FALSE)
		{
			int n = m_ControlListSSSDTInfor.InsertItem(m_ControlListSSSDTInfor.GetItemCount(),strIndex);
			m_ControlListSSSDTInfor.SetItemText(n,1,strFunctionName);
			m_ControlListSSSDTInfor.SetItemText(n,2,strCurrentAddress);
			m_ControlListSSSDTInfor.SetItemText(n,3,strType);

			m_ControlListSSSDTInfor.SetItemText(n,4,strOriginalAddress);
			m_ControlListSSSDTInfor.SetItemText(n,5,SSSDTInfor[i].wzModule);
			if(strType==L"SSSDTHook")
			{
				m_ControlListSSSDTInfor.SetItemData(n,1);
			}
			else if(strType==L"SSSDTInlineHook")
			{
				m_ControlListSSSDTInfor.SetItemData(n,2);
			}
			else
			{
				m_ControlListSSSDTInfor.SetItemData(n,0);
			}
		}
		else
		{
			if(strType==L"SSSDTHook"|| strType==L"SSSDTInlineHook")
			{
				int n = m_ControlListSSSDTInfor.InsertItem(m_ControlListSSSDTInfor.GetItemCount(),strIndex);
				m_ControlListSSSDTInfor.SetItemText(n,1,strFunctionName);
				m_ControlListSSSDTInfor.SetItemText(n,2,strCurrentAddress);
				m_ControlListSSSDTInfor.SetItemText(n,3,strType);

				m_ControlListSSSDTInfor.SetItemText(n,4,strOriginalAddress);
				m_ControlListSSSDTInfor.SetItemText(n,5,SSSDTInfor[i].wzModule);
				if(strType==L"SSSDTHook")
				{
					m_ControlListSSSDTInfor.SetItemData(n,1);
				}
				else if(strType==L"SSSDTInlineHook")
				{
					m_ControlListSSSDTInfor.SetItemData(n,2);
				}
				else
				{
					m_ControlListSSSDTInfor.SetItemData(n,0);
				}
			}
		
		}

		
		bHooked = FALSE;


	
		}
	




}

ULONG_PTR CSSSDT::GetOriginalSSSDTFunctionAddress(ULONG ulIndex)
{
	if(m_ServiceTableBase==0 )
	{


		if(SendIoControlCode(0,NULL,GET_SSSDT_SERVERICE_BASE,NULL)==FALSE)
		{		
			return 0;
		}

	}

	if(m_Win32kModuleBase==0)
	{

		WCHAR wzSysModuleName[MODULE_LENGTH] = L"win32k.sys";

		if(SendIoControlCode(0,NULL,GET_SYS_MODULE_INFOR,wzSysModuleName)==FALSE)
		{
			return 0;
		}

	}

	if( m_TempWin32kModuleBase==0 )
	{

		MakeTempWin32kFile();
		//m_TempWin32kModuleBase = LoadLibrary(m_strTempWin32kFilePath);

		m_TempWin32kModuleBase = LoadLibraryEx(m_strTempWin32kFilePath,0, DONT_RESOLVE_DLL_REFERENCES);
		//m_TempWin32kModuleBase = LoadLibraryEx(L"E:\\win32k.sys",0, DONT_RESOLVE_DLL_REFERENCES);
	}


	if (m_bOk==FALSE)
	{
		if(!FixRelocTable((ULONG_PTR)m_TempWin32kModuleBase,(ULONG_PTR)m_Win32kModuleBase))
		{
			return 0;
		}

		m_bOk = TRUE;
	}

	ULONG_PTR RVA = (ULONG_PTR)m_ServiceTableBase - (ULONG_PTR)m_Win32kModuleBase;
	ULONG_PTR OriginalFunctionAddress = *(ULONG_PTR*)((ULONG_PTR)m_TempWin32kModuleBase+RVA+sizeof(ULONG_PTR)*ulIndex);



	return OriginalFunctionAddress;
}


BOOL CSSSDT::MakeTempWin32kFile()
{
	WCHAR wzBuffer[MAX_PATH] = {0};
	DWORD dwReturn = GetEnvironmentVariable(L"TEMP",wzBuffer,MAX_PATH);

	if (dwReturn==0)
	{
		return FALSE;
	}

	m_strTempWin32kFilePath = wzBuffer;

	m_strTempWin32kFilePath += L"\\Win32k.sys";



	//获得Win32k.sys 路径

	dwReturn = GetSystemDirectory(wzBuffer,MAX_PATH);

	if (dwReturn==0)
	{
		return FALSE;
	}

	m_strWin32kFilePath = wzBuffer;

	m_strWin32kFilePath+=L"\\Win32k.sys";
	if(!CopyFile(m_strWin32kFilePath,m_strTempWin32kFilePath,0))
	{
		return FALSE;
	}


	return TRUE;
}



int CSSSDT::FixRelocTable(ULONG_PTR NewModuleBase, ULONG_PTR OriginalModuleBase)
{
	PIMAGE_DOS_HEADER		DosHeader;
	PIMAGE_NT_HEADERS		NtHeader;
	PIMAGE_BASE_RELOCATION	RelocTable;
	ULONG i,dwOldProtect;
	DosHeader = (PIMAGE_DOS_HEADER)NewModuleBase;
	if (DosHeader->e_magic != IMAGE_DOS_SIGNATURE)
	{
		return 0;
	}
	NtHeader = (PIMAGE_NT_HEADERS)((ULONG_PTR)NewModuleBase + DosHeader->e_lfanew );
	if (NtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size)//是否存在重定位表
	{
		RelocTable=(PIMAGE_BASE_RELOCATION)((ULONG_PTR)NewModuleBase + NtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress);
		do
		{
			//RelocTable->SizeOfBlock 一块的大小
			ULONG	ulNumOfReloc = (RelocTable->SizeOfBlock-sizeof(IMAGE_BASE_RELOCATION))/2;   //重定向的个数   Short【】【】【】【】【】【】
			SHORT	MiniOffset   = 0;
			PUSHORT RelocData    = (PUSHORT)((ULONG_PTR)RelocTable+sizeof(IMAGE_BASE_RELOCATION));  //重定向的项
			for (i=0; i<ulNumOfReloc; i++) 
			{
				PULONG_PTR RelocAddress;//需要重定位的地址

				if (((*RelocData)>>12)==IMAGE_REL_BASED_DIR64||((*RelocData)>>12)==IMAGE_REL_BASED_HIGHLOW)//判断重定位类型是否为IMAGE_REL_BASED_HIGHLOW[32]或IMAGE_REL_BASED_DIR64[64]
				{

					MiniOffset=(*RelocData)&0xFFF;//小偏移    //取出低12

					RelocAddress=(PULONG_PTR)(NewModuleBase+RelocTable->VirtualAddress+MiniOffset);

					VirtualProtect((PVOID)RelocAddress,sizeof(ULONG_PTR),PAGE_EXECUTE_READWRITE, &dwOldProtect);

					*RelocAddress=*RelocAddress+OriginalModuleBase-NtHeader->OptionalHeader.ImageBase;

					VirtualProtect((PVOID)RelocAddress, sizeof(ULONG_PTR),dwOldProtect,&dwOldProtect);
				}
				//下一个重定位数据
				RelocData++;
			}
			//下一个重定位块
			RelocTable=(PIMAGE_BASE_RELOCATION)((ULONG_PTR)RelocTable+RelocTable->SizeOfBlock);
		}
		while (RelocTable->VirtualAddress);
		return TRUE;
	}
	return FALSE;
}

BOOL CSSSDT::SendIoControlCode(ULONG ulIndex,PVOID* FuntionAddress,ULONG_PTR ulControlCode,WCHAR* wzSysModuleName)
{

	BOOL bRet = FALSE;
	DWORD ulReturnSize = 0;

	g_hDeviceSSS = OpenDevice(L"\\\\.\\SSSDTManagerLink");

	if (g_hDeviceSSS==(HANDLE)-1)
	{
		MessageBox(L"打开设备失败");

		return FALSE;
	}
	if (ulControlCode==GET_SSSDT_SERVERICE_BASE)
	{
		bRet = DeviceIoControl(g_hDeviceSSS,IOCTL_GET_SSSDT_SERVERICE_BASE,
			NULL,
			0,
			&m_ServiceTableBase,
			sizeof(PVOID),
			&ulReturnSize,
			NULL);

	}

	if (ulControlCode==GET_SYS_MODULE_INFOR)  
	{
		struct _DATA_ 
		{
			PVOID     SysModuleBase;
			ULONG_PTR ulSysModuleSize;
		}Data;

		memset(&Data,0,sizeof(Data));
		bRet = DeviceIoControl(g_hDeviceSSS,CTL_GET_SYS_MODULE_INFOR,
			wzSysModuleName,
			MODULE_LENGTH,
			&Data,
			sizeof(Data),
			&ulReturnSize,
			NULL);
		m_Win32kModuleBase = Data.SysModuleBase;

	}
	if(ulControlCode==GET_MODULE_NAME)
	{
		struct _DATA_
		{
			PVOID OriginalAddress;
		}Data;
		memset(&Data,0,sizeof(_DATA_));
		Data.OriginalAddress = *FuntionAddress;
	
	//	CString strOriginalAddress;
		//strOriginalAddress.Format(L"0x%p",*FuntionAddress);
 		//MessageBox(strOriginalAddress,L"OriginalAddress");
	
		bRet = DeviceIoControl(g_hDeviceSSS,IOCTL_GET_MODULENAME,
			&Data,
			sizeof(_DATA_),
			wzSysModuleName,
			60*sizeof(WCHAR),
			&ulReturnSize,
			NULL);
	}
	if (ulControlCode==GET_SSSDT_CURRENT_FUNC_CODE)
	{
		
		bRet = DeviceIoControl(g_hDeviceSSS,IOCTL_GET_SSSDT_CURRENT_FUNC_CODE,
			&ulIndex,
			sizeof(ULONG),
			&m_CurrentFunctionCode,
			CODE_LENGTH,
			&ulReturnSize,
			NULL);
		
	}

	CloseHandle(g_hDeviceSSS);
	return bRet;
}

void CSSSDT::OnResumeResumessd()
{
	// TODO: 在此添加命令处理程序代码
	BOOL bRet = FALSE;


	int iSelect = m_ControlListSSSDTInfor.GetSelectionMark( );                   //获得选择项的索引
	CString Address = m_ControlListSSSDTInfor.GetItemText(iSelect,4);          //通过选项中的索引获得数据0项的IP
	CString Index = m_ControlListSSSDTInfor.GetItemText(iSelect,0);
	g_hDeviceSSS = OpenDevice(L"\\\\.\\SSSDTManagerLink");

	if (g_hDeviceSSS==(HANDLE)-1)
	{
		MessageBox(L"打开设备失败");

		return;
	}
	struct _DATA_
	{
		ULONG Index;
		ULONG_PTR OriginalAddress;
	}Data;

	swscanf(Address.GetBuffer()+2,L"%p",&Data.OriginalAddress);   //将0x格掉
	swscanf(Index.GetBuffer(),L"%d",&Data.Index);   //将0x格掉

	DWORD ulReturnSize = 0;


	bRet = DeviceIoControl(g_hDeviceSSS,IOCTL_UNHOOK_SSSDT,
		&Data,
		sizeof(_DATA_),
		NULL,
		NULL,
		&ulReturnSize,
		NULL);


		if (bRet==FALSE)
		{
			return;
		}


	CloseHandle(g_hDeviceSSS);
	OnBnClickedButtonSssdt();

}

void CSSSDT::OnResumeResumeinlinehook()
{
	// TODO: 在此添加命令处理程序代码
	BOOL bRet = FALSE;
	DWORD ulReturnSize = 0;

	g_hDeviceSSS = OpenDevice(L"\\\\.\\SSSDTManagerLink");

	if (g_hDeviceSSS==(HANDLE)-1)
	{
		MessageBox(L"打开设备失败");

		return;
	}
	int iSelect = m_ControlListSSSDTInfor.GetSelectionMark( );                   //获得选择项的索引

	CString Index = m_ControlListSSSDTInfor.GetItemText(iSelect,0);

	

		struct _DATA_ 
		{
			ULONG ulIndex;
			UCHAR szOriginalFunctionCode[CODE_LENGTH];
		};

		_DATA_ Data = {0};

		
		swscanf(Index.GetBuffer(),L"%d",&Data.ulIndex);   //将0x格掉
// 		CString a;
// 		a.Format(L"%d",iItem);
// 		MessageBox(a,L"dddddd");
		memcpy(Data.szOriginalFunctionCode,SSSDTInfor[Data.ulIndex].szOriginalFunctionCode,CODE_LENGTH);
		bRet = DeviceIoControl(g_hDeviceSSS,IOCTL_RESUME_SSSDT_INLINEHOOK,
			&Data,
			sizeof(_DATA_),
			NULL,
			NULL,
			&ulReturnSize,
			NULL);
	
	CloseHandle(g_hDeviceSSS);
	OnBnClickedButtonSssdt();

	
}



void CSSSDT::OnRclickListSssdt(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	//GetMenu()->GetSubMenu(0)->CheckMenuItem(0,MF_BYCOMMAND | MF_CHECKED);
	int i = 0;
	CMenu	popup;
	popup.LoadMenu(IDR_MENU);               //加载菜单资源
	CMenu*	pM = popup.GetSubMenu(0);                 //获得菜单的子项
	CPoint	p;
	GetCursorPos(&p);
	int	count = pM->GetMenuItemCount();
	if (m_ControlListSSSDTInfor.GetSelectedCount() == 0)         //如果没有选中
	{ 
		for (int i = 0;i<count;i++)
		{
			pM->EnableMenuItem(i, MF_BYPOSITION | MF_DISABLED | MF_GRAYED);          //菜单全部变灰
		}

	}else
	{
		POSITION Pos = m_ControlListSSSDTInfor.GetFirstSelectedItemPosition(); 
		int iItem = m_ControlListSSSDTInfor.GetNextSelectedItem(Pos); 
		i = m_ControlListSSSDTInfor.GetItemData(iItem);  
		if(i==1)
		{
			pM->EnableMenuItem(1, MF_BYPOSITION | MF_DISABLED | MF_GRAYED);          //内存清零变灰色
		}
		if(i==2)
		{
			pM->EnableMenuItem(0, MF_BYPOSITION | MF_DISABLED | MF_GRAYED);          //内存清零变灰色
		}
		if(i==0)
		{
			pM->EnableMenuItem(0, MF_BYPOSITION | MF_DISABLED | MF_GRAYED);          //内存清零变灰色
			pM->EnableMenuItem(1, MF_BYPOSITION | MF_DISABLED | MF_GRAYED);          //内存清零变灰色
		}

	}

	
	pM->TrackPopupMenu(TPM_LEFTALIGN, p.x, p.y, this);
	*pResult = 0;
}


void CSSSDT::OnResumeShowhook()
{
	// TODO: 在此添加命令处理程序代码
	if(m_ShowHook==FALSE)
	{
		
		//GetMenu()->GetSubMenu(0)->CheckMenuItem(ID_RESUME_SHOWHOOK,MF_BYCOMMAND | MF_CHECKED);
	//	MessageBox(L"aaaaaaaa",L"ddd1");
		m_ShowHook=TRUE;
		OnBnClickedButtonSssdt();
		return;
		
	}
	if(m_ShowHook==TRUE)
	{
	//	MessageBox(L"bnbbbbbbb",L"ddd");
		//	GetMenu()->GetSubMenu(0)->CheckMenuItem(ID_RESUME_SHOWHOOK,MF_BYCOMMAND | MF_UNCHECKED);
	//	GetMenu()->GetSubMenu(0)->CheckMenuItem(2,MF_BYPOSITION | MF_UNCHECKED);  //取消标记
		m_ShowHook=FALSE;
		OnBnClickedButtonSssdt();
		return;
	}


}


void CSSSDT::OnUpdateIdrMenu(CCmdUI *pCmdUI)
{
	// TODO: 在此添加命令更新用户界面处理程序代码
	pCmdUI->SetCheck();
}

