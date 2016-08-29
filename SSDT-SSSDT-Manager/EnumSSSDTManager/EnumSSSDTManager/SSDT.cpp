// SSDT.cpp : 实现文件
//

#include "stdafx.h"
#include "EnumSSSDTManager.h"
#include "SSDT.h"
#include "SSDTFunc.h"
#include "afxdialogex.h"
HANDLE g_hDevice = NULL;

// CSSDT 对话框
SSDT_INFOR  SSDTInfor[0x1000] = {0};
IMPLEMENT_DYNAMIC(CSSDT, CDialogEx)

CSSDT::CSSDT(CWnd* pParent /*=NULL*/)
	: CDialogEx(CSSDT::IDD, pParent)
	, m_Num1(0)
{
	m_ServiceTable         = 0;
	m_ServiceTableBase     = 0;
	m_NtosModuleBase     = 0 ;
	m_TempNtoskModuleBase = 0;
	m_bOk = FALSE;
	m_ShowHook = FALSE;
	memset(m_CurrentFunctionCode,0,CODE_LENGTH);
}

CSSDT::~CSSDT()
{
}

void CSSDT::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_SSDT, m_ControlListSSDTInfor);
	DDX_Text(pDX, IDC_EDIT_NUM1, m_Num1);
}


BEGIN_MESSAGE_MAP(CSSDT, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON_ENUMFUNC, &CSSDT::OnBnClickedButtonEnumfunc)
	ON_COMMAND(ID_RESUME_RESUMESSD, &CSSDT::OnResumeResumessd)
	ON_COMMAND(ID_RESUME_RESUMEINLINEHOOK, &CSSDT::OnResumeResumeinlinehook)
	ON_COMMAND(ID_RESUME_SHOWHOOK, &CSSDT::OnResumeShowhook)
	ON_NOTIFY(NM_RCLICK, IDC_LIST_SSDT, &CSSDT::OnRclickListSsdt)
END_MESSAGE_MAP()



BOOL CSSDT::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
	m_ControlListSSDTInfor.SetExtendedStyle(LVS_EX_FULLROWSELECT);
	m_ControlListSSDTInfor.InsertColumn(0, L"序号", LVCFMT_LEFT, 100);
	m_ControlListSSDTInfor.InsertColumn(1, L"函数名称", LVCFMT_LEFT, 200);
	m_ControlListSSDTInfor.InsertColumn(2, L"当前地址", LVCFMT_LEFT, 100);
	m_ControlListSSDTInfor.InsertColumn(3, L"状态",LVCFMT_LEFT,80);
	m_ControlListSSDTInfor.InsertColumn(4, L"原始地址", LVCFMT_LEFT, 100);
	m_ControlListSSDTInfor.InsertColumn(5, L"当前地址所属模块",LVCFMT_LEFT,300);
	

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}


// CSSDT 消息处理程序

void CSSDT::OnBnClickedButtonEnumfunc()
{
	m_ControlListSSDTInfor.DeleteAllItems();
	// TODO: 在此添加控件通知处理程序代码
	OpenDeviceSSDT.g_hDevice =  OpenDeviceSSDT.OpenDevice(L"\\\\.\\SSSDTManagerLink");
	if (OpenDeviceSSDT.g_hDevice==(HANDLE)-1)
	{
		MessageBox(L"打开设备失败");

		return;
	}

	if(OpenDeviceSSDT.SendIoControlCode(0,NULL,SSDTINITIALIZE)==FALSE) //
	{
		CloseHandle(OpenDeviceSSDT.g_hDevice);
		return;
	}

	EnumSSDTInfor(SSDTInfor);




	CloseHandle(OpenDeviceSSDT.g_hDevice);
}
BOOL CSSDT::EnumSSDTInfor(PSSDT_INFOR SSDTInfor)
{
	m_ControlListSSDTInfor.DeleteAllItems();
	ULONG_PTR SSDTFunctionCount = 0;
	ULONG_PTR HookedFunctionCount = 0;


	PVOID SSDTOriAddr = 0;
	PVOID CurrentFunctionAddress = 0;
	PVOID OriginalFunctionAddress = 0;
	int i = 0;
	WCHAR wzModuleName[60]= {0};
#ifdef _WIN64



	for (i=0;i<sizeof(szWin7FunctionNameSSDT)/100;i++)
	{
		
		
		OpenDeviceSSDT.SendIoControlCode(i,&CurrentFunctionAddress,GET_SSDT_CURRENT_FUNC_ADDR);
	
		SendIoControlCode(i,&CurrentFunctionAddress,GET_SSDT_MODULE_NAME,wzModuleName);
		OriginalFunctionAddress = (PVOID)GetOriginalSSDTFunctionAddress(i);
	
		SSDTInfor[SSDTFunctionCount].FunctionIndex = i;
		SSDTInfor[SSDTFunctionCount].CurrentFunctionAddress = CurrentFunctionAddress;
		SSDTInfor[SSDTFunctionCount].OriginalFunctionAddress = OriginalFunctionAddress;
		strcpy(SSDTInfor[SSDTFunctionCount].szFunctionName,szWin7FunctionNameSSDT[i]);
		wcscpy(SSDTInfor[SSDTFunctionCount].wzModule,wzModuleName);

	

		if (CurrentFunctionAddress!=OriginalFunctionAddress)
		{
			HookedFunctionCount++;
		}

		SSDTFunctionCount++;
	}
#else
	for (i=0;i<sizeof(szWinXPFunctionNameSSDT)/100;i++)
	{


		OpenDeviceSSDT.SendIoControlCode(i,&CurrentFunctionAddress,GET_SSDT_CURRENT_FUNC_ADDR);

		SendIoControlCode(i,&CurrentFunctionAddress,GET_SSDT_MODULE_NAME,wzModuleName);
		OriginalFunctionAddress = (PVOID)GetOriginalSSDTFunctionAddress(i);

		SSDTInfor[SSDTFunctionCount].FunctionIndex = i;
		SSDTInfor[SSDTFunctionCount].CurrentFunctionAddress = CurrentFunctionAddress;
		SSDTInfor[SSDTFunctionCount].OriginalFunctionAddress = OriginalFunctionAddress;
		strcpy(SSDTInfor[SSDTFunctionCount].szFunctionName,szWinXPFunctionNameSSDT[i]);
		wcscpy(SSDTInfor[SSDTFunctionCount].wzModule,wzModuleName);
			  



		if (CurrentFunctionAddress!=OriginalFunctionAddress)
		{
			HookedFunctionCount++;
		}

		SSDTFunctionCount++;
	}


#endif

	AddItemToControlList(SSDTFunctionCount,SSDTInfor);
	m_Num1 = SSDTFunctionCount;
	UpdateData(FALSE);

	return TRUE;
}


ULONG_PTR CSSDT::GetOriginalSSDTFunctionAddress(ULONG ulIndex)
{
	if(m_ServiceTableBase==0 )
	{


		if(SendIoControlCode(0,NULL,GET_SSDT_SERVERICE_BASE,NULL)==FALSE)
		{		
			return 0;
		}


// 			CString strOriginalAddress;
// 		strOriginalAddress.Format(L"0x%p",m_ServiceTableBase);
// 		MessageBox(strOriginalAddress,L"m_ServiceTableBase");

	}

	if(m_NtosModuleBase==0)
	{

		//WCHAR wzNtosModuleName[MODULE_LENGTH] = L"ntkrnlpa.exe";
		WCHAR wzNtosModuleName[MODULE_LENGTH] = L"ntoskrnl.exe";
		if(SendIoControlCode(0,NULL,GET_SSDT_SYS_MODULE_INFOR,wzNtosModuleName)==FALSE)
		{
			return 0;
		}
// 		CString strOriginalAddress;
// 		strOriginalAddress.Format(L"m_NtosModule:0x%p",m_NtosModuleBase);

	}

	if( m_TempNtoskModuleBase==0 )
	{

		MakeTempWin32kFile();
		//m_TempWin32kModuleBase = LoadLibrary(m_strTempWin32kFilePath);
		m_TempNtoskModuleBase = LoadLibraryEx(m_strTempNtosFilePath,0, DONT_RESOLVE_DLL_REFERENCES);
	}


	if (m_bOk==FALSE)
	{
		if(!FixRelocTable((ULONG_PTR)m_TempNtoskModuleBase,(ULONG_PTR)m_NtosModuleBase))
		{
			return 0;
		}

		m_bOk = TRUE;
	}

	ULONG_PTR RVA = (ULONG_PTR)m_ServiceTableBase - (ULONG_PTR)m_NtosModuleBase;
	ULONG_PTR OriginalFunctionAddress = *(ULONG_PTR*)((ULONG_PTR)m_TempNtoskModuleBase+RVA+sizeof(ULONG_PTR)*ulIndex);



	return OriginalFunctionAddress;
}


BOOL CSSDT::MakeTempWin32kFile()
{
	WCHAR wzBuffer[MAX_PATH] = {0};
	DWORD dwReturn = GetEnvironmentVariable(L"TEMP",wzBuffer,MAX_PATH);

	if (dwReturn==0)
	{
		return FALSE;
	}

	m_strTempNtosFilePath = wzBuffer;
#ifdef _WIN64
	m_strTempNtosFilePath += L"\\ntoskrnl.exe";
#else
	m_strTempNtosFilePath += L"\\ntkrnlpa.exe";//xp下表示支持PAE  如果不支持PAE 则是ntoskrnl
#endif
	
	//获得Win32k.sys 路径

	dwReturn = GetSystemDirectory(wzBuffer,MAX_PATH);

	if (dwReturn==0)
	{
		return FALSE;
	}

	m_strNtoskFilePath = wzBuffer;

#ifdef _WIN64
	m_strNtoskFilePath+=L"\\ntoskrnl.exe";
#else
	m_strNtoskFilePath+=L"\\ntkrnlpa.exe";
#endif
	
	
	if(!CopyFile(m_strNtoskFilePath,m_strTempNtosFilePath,0))
	{
		return FALSE;
	}


	return TRUE;
}



int CSSDT::FixRelocTable(ULONG_PTR NewModuleBase, ULONG_PTR OriginalModuleBase)
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


BOOL CSSDT::SendIoControlCode(ULONG ulIndex,PVOID* FuntionAddress,ULONG_PTR ulControlCode,WCHAR* wzSysModuleName)
{

	BOOL bRet = FALSE;
	DWORD ulReturnSize = 0;

	g_hDevice = OpenDevice(L"\\\\.\\SSSDTManagerLink");

	if (g_hDevice==(HANDLE)-1)
	{
		MessageBox(L"打开设备失败");

		return FALSE;
	}
	if (ulControlCode==GET_SSDT_CURRENT_FUNC_CODE)
	{

		bRet = DeviceIoControl(g_hDevice,IOCTL_GET_SSDT_CURRENT_FUNC_CODE,
			&ulIndex,
			sizeof(ULONG),
			&m_CurrentFunctionCode,
			CODE_LENGTH,
			&ulReturnSize,
			NULL);

	}
	if(ulControlCode==GET_SSDT_MODULE_NAME)
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

		bRet = DeviceIoControl(g_hDevice,IOCTL_GET_SSDT_MODULENAME,
			&Data,
			sizeof(_DATA_),
			wzSysModuleName,
			60*sizeof(WCHAR),
			&ulReturnSize,
			NULL);
	}
	if (ulControlCode==GET_SSDT_SERVERICE_BASE)
	{
		bRet = DeviceIoControl(g_hDevice,IOCTL_GET_SSDT_SERVERICE_BASE,
			NULL,
			0,
			&m_ServiceTableBase,
			sizeof(PVOID),
			&ulReturnSize,
			NULL);

	}

	if (ulControlCode==GET_SSDT_SYS_MODULE_INFOR)  
	{
		struct _DATA_ 
		{
			PVOID     SysModuleBase;
			ULONG_PTR ulSysModuleSize;
		}Data;

		memset(&Data,0,sizeof(Data));

		bRet = DeviceIoControl(g_hDevice,CTL_GET_SSDT_SYS_MODULE_INFOR,
			wzSysModuleName,
			MODULE_LENGTH,
			&Data,
			sizeof(Data),
			&ulReturnSize,
			NULL);
		m_NtosModuleBase = Data.SysModuleBase;

	}

	CloseHandle(g_hDevice);
	return bRet;
}
VOID CSSDT::AddItemToControlList(ULONG SSDTFunctionCount,PSSDT_INFOR SSDTInfor)
{
	int i = 0;
	CString strIndex;
	BOOL bHooked = FALSE;
	for (i=0;i<SSDTFunctionCount;i++)
	{

		strIndex.Format(L"%d",SSDTInfor[i].FunctionIndex);


		CString strFunctionName(SSDTInfor[i].szFunctionName);


		CString strCurrentAddress;
		strCurrentAddress.Format(L"0x%p",SSDTInfor[i].CurrentFunctionAddress);

		CString strOriginalAddress;
		strOriginalAddress.Format(L"0x%p",SSDTInfor[i].OriginalFunctionAddress);




		CString strType;
		if (SSDTInfor[i].OriginalFunctionAddress!=SSDTInfor[i].CurrentFunctionAddress)
		{
			//	m_ControlListSSSDTInfor.SetItemData(n,1);

			strType = L"SSDTHook";


			bHooked = TRUE;
		}
		else
		{

			//获得原始代码



			GetOriginalSSDTFunctionCode((ULONG_PTR)SSDTInfor[i].CurrentFunctionAddress,SSDTInfor[i].szOriginalFunctionCode, CODE_LENGTH);
			//GetOriginalSSDTFunctionCode((ULONG_PTR)SSDTInfor[i].OriginalFunctionAddress,SSDTInfor[i].szOriginalFunctionCode, CODE_LENGTH);


			//获得当前代码
			if(SendIoControlCode(i,NULL,GET_SSDT_CURRENT_FUNC_CODE,NULL)==TRUE)
			{

				memcpy(SSDTInfor[i].szCurrentFunctionCode,m_CurrentFunctionCode,CODE_LENGTH);
				memset(m_CurrentFunctionCode,0,CODE_LENGTH);

				int j = 0;
				for (j=0;j<CODE_LENGTH;j++)
				{

					if (SSDTInfor[i].szOriginalFunctionCode[j]!=SSDTInfor[i].szCurrentFunctionCode[j])
					{

						bHooked = TRUE;

						break;
					}
				}
			}
			if (bHooked==TRUE)
			{
				//		m_ControlListSSSDTInfor.SetItemData(n,2);
				strType = L"SSDTInlineHook";
			}

		}

		if (bHooked==FALSE)
		{
			//	m_ControlListSSSDTInfor.SetItemData(n,0);
			strType = L"";
		}

		if(m_ShowHook==FALSE)
		{
			int n = m_ControlListSSDTInfor.InsertItem(m_ControlListSSDTInfor.GetItemCount(),strIndex);
			m_ControlListSSDTInfor.SetItemText(n,1,strFunctionName);
			m_ControlListSSDTInfor.SetItemText(n,2,strCurrentAddress);
			m_ControlListSSDTInfor.SetItemText(n,3,strType);

			m_ControlListSSDTInfor.SetItemText(n,4,strOriginalAddress);
		
		//	m_ControlListSSDTInfor.SetItemText(n,5,SSDTInfor[i].wzModule);
			m_ControlListSSDTInfor.SetItemText(n,5,SSDTInfor[i].wzModule);
			if(strType==L"SSDTHook")
			{
				m_ControlListSSDTInfor.SetItemData(n,1);
			}
			else if(strType==L"SSDTInlineHook")
			{
				m_ControlListSSDTInfor.SetItemData(n,2);
			}
			else
			{
				m_ControlListSSDTInfor.SetItemData(n,0);
			}
		}
		else
		{
			if(strType==L"SSDTHook"|| strType==L"SSDTInlineHook")
			{
				int n = m_ControlListSSDTInfor.InsertItem(m_ControlListSSDTInfor.GetItemCount(),strIndex);
				m_ControlListSSDTInfor.SetItemText(n,1,strFunctionName);
				m_ControlListSSDTInfor.SetItemText(n,2,strCurrentAddress);
				m_ControlListSSDTInfor.SetItemText(n,3,strType);

				m_ControlListSSDTInfor.SetItemText(n,4,strOriginalAddress);
			//	m_ControlListSSDTInfor.SetItemText(n,5,SSDTInfor[i].wzModule);
				m_ControlListSSDTInfor.SetItemText(n,5,SSDTInfor[i].wzModule);
				if(strType==L"SSDTHook")
				{
					m_ControlListSSDTInfor.SetItemData(n,1);
				}
				else if(strType==L"SSDTInlineHook")
				{
					m_ControlListSSDTInfor.SetItemData(n,2);
				}
				else
				{
					m_ControlListSSDTInfor.SetItemData(n,0);
				}
			}

		}


		bHooked = FALSE;



	}





}







ULONG_PTR CSSDT::RVAToOffset(PIMAGE_NT_HEADERS NTHeader, ULONG_PTR ulRVA)  
{
	PIMAGE_SECTION_HEADER SectionHeader =
		(PIMAGE_SECTION_HEADER)((ULONG_PTR)NTHeader + sizeof(IMAGE_NT_HEADERS));  //获得节表  

	for(int i = 0; i < NTHeader->FileHeader.NumberOfSections; i++)
	{

		if(ulRVA >= SectionHeader[i].VirtualAddress && ulRVA < 
			(SectionHeader[i].VirtualAddress 
			+ SectionHeader[i].SizeOfRawData))
		{
			//文件偏移
			return SectionHeader[i].PointerToRawData +   //该块在磁盘文件中的偏移
				(ulRVA - SectionHeader[i].VirtualAddress);  //加上虚拟偏移-块区偏移
		}
	}

	return 0;
}















VOID CSSDT::AddItemToControlList(SSDT_INFOR_FOR SSDTInfor)
{
		CString strIndex;
		CString strCurrentAddress;
		CString strFunctionName(SSDTInfor.szFunctionName);
		strIndex.Format(L"%d",SSDTInfor.FunctionIndex);
		strCurrentAddress.Format(L"0x%p",SSDTInfor.CurrentFunctionAddress);


		int n = m_ControlListSSDTInfor.InsertItem(m_ControlListSSDTInfor.GetItemCount(),strIndex);
		m_ControlListSSDTInfor.SetItemText(n,1,strFunctionName);
		m_ControlListSSDTInfor.SetItemText(n,2,strCurrentAddress);


}





void CSSDT::OnResumeResumessd()
{
	// TODO: 在此添加命令处理程序代码
	BOOL bRet = FALSE;


	int iSelect = m_ControlListSSDTInfor.GetSelectionMark( );                   //获得选择项的索引
	CString Address = m_ControlListSSDTInfor.GetItemText(iSelect,4);          //通过选项中的索引获得数据0项的IP
	CString Index = m_ControlListSSDTInfor.GetItemText(iSelect,0);
	g_hDevice = OpenDevice(L"\\\\.\\SSSDTManagerLink");

	if (g_hDevice==(HANDLE)-1)
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


	bRet = DeviceIoControl(g_hDevice,IOCTL_UNHOOK_SSDT,
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


	CloseHandle(g_hDevice);
	OnBnClickedButtonEnumfunc();
}


void CSSDT::OnResumeResumeinlinehook()
{
	// TODO: 在此添加命令处理程序代码
	BOOL bRet = FALSE;
	DWORD ulReturnSize = 0;

	g_hDevice = OpenDevice(L"\\\\.\\SSSDTManagerLink");

	if (g_hDevice==(HANDLE)-1)
	{
		MessageBox(L"打开设备失败");

		return;
	}
	int iSelect = m_ControlListSSDTInfor.GetSelectionMark();                   //获得选择项的索引

	CString Index = m_ControlListSSDTInfor.GetItemText(iSelect,0);



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
	memcpy(Data.szOriginalFunctionCode,SSDTInfor[Data.ulIndex].szOriginalFunctionCode,CODE_LENGTH);
	bRet = DeviceIoControl(g_hDevice,IOCTL_RESUME_SSDT_INLINEHOOK,
		&Data,
		sizeof(_DATA_),
		NULL,
		NULL,
		&ulReturnSize,
		NULL);

	CloseHandle(g_hDevice);
	OnBnClickedButtonEnumfunc();

}


void CSSDT::OnResumeShowhook()
{
	// TODO: 在此添加命令处理程序代码
	if(m_ShowHook==FALSE)
	{
		m_ShowHook=TRUE;
		OnBnClickedButtonEnumfunc();
		return;

	}
	if(m_ShowHook==TRUE)
	{
	
		m_ShowHook=FALSE;
		OnBnClickedButtonEnumfunc();
		return;
	}

}


void CSSDT::OnRclickListSsdt(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	int i = 0;
	CMenu	popup;
	popup.LoadMenu(IDR_MENU);               //加载菜单资源
	CMenu*	pM = popup.GetSubMenu(0);                 //获得菜单的子项
	CPoint	p;
	GetCursorPos(&p);
	int	count = pM->GetMenuItemCount();
	if (m_ControlListSSDTInfor.GetSelectedCount() == 0)         //如果没有选中
	{ 
		for (int i = 0;i<count;i++)
		{
			pM->EnableMenuItem(i, MF_BYPOSITION | MF_DISABLED | MF_GRAYED);          //菜单全部变灰
		}

	}else
	{
		POSITION Pos = m_ControlListSSDTInfor.GetFirstSelectedItemPosition(); 
		int iItem = m_ControlListSSDTInfor.GetNextSelectedItem(Pos); 
		i = m_ControlListSSDTInfor.GetItemData(iItem);  
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
