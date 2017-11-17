// ZVERUCLEANER.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "ZVERUCLEANER.h"
#include <CommCtrl.h>
#include <string>
#include <shlobj.h>
#include <sstream>
#include <iostream>
#include <vector>
#include <winuser.h>
#include <algorithm>
#include <fstream>
#include <filesystem>
#include <Shlwapi.h>



#include <stdlib.h>

#include <functional>

#include <commdlg.h>


#pragma comment(lib,"ComCtl32.Lib")

using namespace std;

#define HOT_RUN 55
#define HOT_ABOUT 54
#define HOT_CNTRL_O 53
#define HOT_EXIT 52
#define HOT_REMOVE 51
#define HOT_CNTRL_S 50

#define IDM_RUN_REMOVE 14
#define IDM_FILE_SAVE 1
#define IDM_FILE_OPEN 2
#define IDM_FILE_QUIT 3
#define IDM_INFO_ABOUT 12
#define IDM_RUN_RUN 13
#define IDD_STATUSBAR 4
#define IDD_PROGRESS 5
#define IDM_NEW 6
#define IDM_OPEN 7
#define IDM_SAVE 8
#define IDM_REMOVE 11
#define ID_LIST 9
#define MAX_LOADSTRING 100

struct FileInfo
{
	wstring path;
	wstring name;
	wstring atributes;
	wstring namePath;
	wstring type;
	FileInfo(wstring _path, wstring _name, wstring _atributes)
	{
		path = _path;
		name = _name;
		atributes = _atributes;
		namePath = path + name;
	}
	friend bool operator <(const FileInfo &x, const FileInfo &y) {
		if (x.atributes != y.atributes)
		return x.atributes < y.atributes;
		auto t1 = x.name;
		auto t2 = y.name;
		std::reverse(t1.begin(), t1.end());
		std::reverse(t2.begin(), t2.end());
		return t1 < t2;
	}
};

HWND CreateTrackingToolTip(int toolID, HWND hDlg, HINSTANCE hInst, WCHAR* pText)
{
	// Create a tooltip.
	HWND hwndTT = CreateWindowEx(WS_EX_TOPMOST, TOOLTIPS_CLASS, NULL,
		WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		hDlg, NULL, hInst, NULL);

	if (!hwndTT)
	{
		return NULL;
	}

	// Set up the tool information. In this case, the "tool" is the entire parent window.
	TOOLINFO g_toolItem = { 0 };
	g_toolItem.cbSize = sizeof(TOOLINFO);
	g_toolItem.uFlags = TTF_IDISHWND | TTF_TRACK | TTF_ABSOLUTE;
	g_toolItem.hwnd = hDlg;
	g_toolItem.hinst = hInst;
	g_toolItem.lpszText = pText;
	g_toolItem.uId = (UINT_PTR)hDlg;

	GetClientRect(hDlg, &g_toolItem.rect);

	// Associate the tooltip with the tool window.

	SendMessage(hwndTT, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&g_toolItem);

	return hwndTT;
}


HWND CreateToolTip(int toolID, HWND hDlg, HINSTANCE hInst, PTSTR pszText)
{
	if (!toolID || !hDlg || !pszText)
	{
		return NULL;
	}

	// Get the window of the tool.
	HWND hwndTool = GetDlgItem(hDlg, toolID);
	if (!hwndTool)
	{
		return NULL;
	}

	// Create the tooltip. g_hInst is the global instance handle.
	HWND hwndTip = CreateWindowEx(NULL, TOOLTIPS_CLASS, NULL,
		WS_POPUP | TTS_ALWAYSTIP | TTS_BALLOON,
		CW_USEDEFAULT, CW_USEDEFAULT,
		CW_USEDEFAULT, CW_USEDEFAULT,
		hDlg, NULL,
		hInst, NULL);

	if (!hwndTip)
	{
		return NULL;
	}

	// Associate the tooltip with the tool.
	TOOLINFO toolInfo = { 0 };
	toolInfo.cbSize = sizeof(toolInfo);
	toolInfo.hwnd = hDlg;
	toolInfo.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
	toolInfo.uId = (UINT_PTR)hwndTool;
	toolInfo.lpszText = pszText;
	if (!SendMessage(hwndTip, TTM_ADDTOOL, 0, (LPARAM)&toolInfo))
	{
		DestroyWindow(hwndTip);
		return NULL;
	}

	return hwndTip;
}
// Global Variables:
wstring ProgramLog;
static HWND hStatusBar, hProgressBar;
vector <FileInfo> info;
vector <wstring> emptyFolders;
HWND hWndToolbar;
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
wstring Path;
// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_ZVERUCLEANER, szWindowClass, MAX_LOADSTRING);
	
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_ZVERUCLEANER));

    MSG msg;



	
    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }



    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SMALL));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_ZVERUCLEANER);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
	
    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, 512, 378, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}
HIMAGELIST g_hImageList = NULL;

HWND CreateSimpleToolbar(HWND hWndParent)
{
	const int ID_TB_STANDARD = 0;

	const int ID_IL_STANDARD = 0;
	/*
	

	HWND hWndToolbar = CreateWindowEx(0, TOOLBARCLASSNAME, NULL,
		WS_CHILD | TBSTYLE_TOOLTIPS, 0, 0, 0, 0, hWndParent, (HMENU)ID_TB_STANDARD, hInst, NULL);

	HIMAGELIST hImageList = ImageList_LoadBitmap(hInst, MAKEINTRESOURCEW(IDB_BITMAP1), 32, 0, RGB(255, 0, 255));
	ImageList_Add(hImageList, LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP2)), NULL);
	ImageList_Add(hImageList, LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP3)), NULL);

	SendMessage(hWndToolbar, TB_SETIMAGELIST, (WPARAM)ID_IL_STANDARD, (LPARAM)hImageList);
	SendMessage(hWndToolbar, (UINT)TB_SETHOTIMAGELIST, 0, (LPARAM)hImageList);
	SendMessage(hWndToolbar, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0);

	TBBUTTON tbb[3] =
	{
		{ 0,IDM_NEW,TBSTATE_ENABLED,TBSTYLE_BUTTON },
		{ 1,IDM_OPEN,TBSTATE_ENABLED,TBSTYLE_BUTTON },
		{ 2,IDM_SAVE,TBSTATE_ENABLED,TBSTYLE_BUTTON },
	};



	SendMessage(hWndToolbar, (UINT)TB_ADDBUTTONS, 3, (LPARAM)&tbb);

	SendMessage(hWndToolbar, TB_AUTOSIZE, 0, 0);
	ShowWindow(hWndToolbar, SW_SHOW);
	return hWndToolbar;*/
	// Declare and initialize local constants.
	const int ImageListID = 0;
	const int numButtons = 4;
	const int bitmapSize = 32;

	const DWORD buttonStyles = BTNS_AUTOSIZE;

	// Create the toolbar.
	hWndToolbar = CreateWindowEx(0, TOOLBARCLASSNAME, NULL,
		WS_CHILD | TBSTYLE_WRAPABLE| TBSTYLE_TOOLTIPS, 0, 0, 0, 0,
		hWndParent, NULL, hInst, NULL);

	if (hWndToolbar == NULL)
		return NULL;
	SendMessage(hWndToolbar, TB_BUTTONSTRUCTSIZE, (WPARAM) sizeof(TBBUTTON), 0);

	//Prevents text from displaying
	SendMessage(hWndToolbar, TB_SETMAXTEXTROWS, 0, 0);
	LPWSTR toolStrings[] = {
		L"Новий", L"Відкрити", L"Зберегти",L"Видалити"
	};
	
	HIMAGELIST hImageList = ImageList_LoadBitmap(hInst, MAKEINTRESOURCEW(IDB_BITMAP2), 32, 0, RGB(255, 0, 255));
	ImageList_Add(hImageList, LoadBitmap(hInst, MAKEINTRESOURCEW(IDB_BITMAP1)), NULL);
	ImageList_Add(hImageList, LoadBitmap(hInst, MAKEINTRESOURCEW(IDB_BITMAP4)), NULL);
	ImageList_Add(hImageList, LoadBitmap(hInst, MAKEINTRESOURCEW(IDB_BITMAP3)), NULL);
	
	// Create the image list.
	//TBADDBITMAP tb;
	//tb.hInst = hInst;
	//tb.nID = IDB_BITMAP2;
	//stdidx = SendMessage(hwndTB, TB_ADDBITMAP, 2, (LPARAM)&tb);



	g_hImageList = ImageList_Create(bitmapSize, bitmapSize,   // Dimensions of individual bitmaps.
		ILC_COLOR16 | ILC_MASK,   // Ensures transparent background.
		numButtons, 0);

	/////////////////////////
	SendMessage(hWndToolbar, TB_SETIMAGELIST, (WPARAM)ID_IL_STANDARD, (LPARAM)hImageList);
	SendMessage(hWndToolbar, (UINT)TB_SETHOTIMAGELIST, 0, (LPARAM)hImageList);
	SendMessage(hWndToolbar, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0);
	// Set the image list.
//	SendMessage(hWndToolbar, TB_SETIMAGELIST,
//	(WPARAM)ImageListID,
	//	(LPARAM)g_hImageList);

	// Load the button images.
	//SendMessage(hWndToolbar, TB_LOADIMAGES,
	//(WPARAM)IDB_STD_SMALL_COLOR,
	//(LPARAM)HINST_COMMCTRL);

	// Initialize button info.
	// IDM_NEW, IDM_OPEN, and IDM_SAVE are application-defined command constants.

	TBBUTTON tbButtons[numButtons] =
	{
		{ 0,IDM_NEW,TBSTATE_ENABLED,TBSTYLE_BUTTON },
		{ 1,IDM_OPEN,TBSTATE_ENABLED,TBSTYLE_BUTTON },
		{ 2,IDM_SAVE,TBSTATE_ENABLED,TBSTYLE_BUTTON },
		{ 3,IDM_REMOVE,TBSTATE_ENABLED,TBSTYLE_BUTTON }
		/*{ MAKELONG(STD_FILENEW,  ImageListID), IDM_NEW,  TBSTATE_ENABLED, buttonStyles,{ 0 }, 0, (INT_PTR)L"" },
		{ MAKELONG(STD_FILEOPEN, ImageListID), IDM_OPEN, TBSTATE_ENABLED, buttonStyles,{ 0 }, 0, (INT_PTR)L"" },
		{ MAKELONG(STD_FILESAVE, ImageListID), IDM_SAVE, TBSTATE_ENABLED, buttonStyles,{ 0 }, 0, (INT_PTR)L"" },
		{ MAKELONG(STD_DELETE, ImageListID), IDM_REMOVE, TBSTATE_ENABLED, buttonStyles,{ 0 }, 0, (INT_PTR)L"" }*/
	};


	for (auto index = 0; index < 4; index++) {
		if (*toolStrings[index] != *"")
			tbButtons[index].iString = SendMessage(hWndToolbar, TB_ADDSTRING, 0, (LPARAM)(LPWSTR)toolStrings[index]);
		else tbButtons[index].iString = 0;
	}
	////////////////


	// Add buttons.
	SendMessage(hWndToolbar, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0);
	SendMessage(hWndToolbar, TB_ADDBUTTONS, (WPARAM)numButtons, (LPARAM)&tbButtons);

	// Resize the toolbar, and then show it.
	SendMessage(hWndToolbar, TB_AUTOSIZE, 0, 0);
	ShowWindow(hWndToolbar, TRUE);

	return hWndToolbar;
}


//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//



void AddMenus(HWND hwnd) {

	HMENU hMenubar;
	HMENU hMenu;
	HMENU hInfo;
	HMENU hRun;
	hMenubar = CreateMenu();
	hMenu = CreateMenu();
	hInfo = CreateMenu();
	hRun = CreateMenu();

	AppendMenuW(hMenu, MF_STRING, IDM_FILE_OPEN, L"&Відкрити папку ctrl+o");
	AppendMenuW(hMenu, MF_STRING, IDM_FILE_SAVE, L"&Зберегти звіт ctrl+s");
	AppendMenuW(hMenu, MF_SEPARATOR, 0, NULL);
	AppendMenuW(hMenu, MF_STRING, IDM_FILE_QUIT, L"&Вихід esc");

	AppendMenuW(hRun, MF_STRING, IDM_RUN_RUN, L"&Почати аналіз ctrl+r");
	AppendMenuW(hRun, MF_STRING, IDM_RUN_REMOVE, L"&Видалити файл delete");

	AppendMenuW(hInfo, MF_STRING, IDM_INFO_ABOUT, L"&Про програму ctrl+i");
	


	AppendMenuW(hMenubar, MF_POPUP, (UINT_PTR)hMenu, L"&Файл");
	AppendMenuW(hMenubar, MF_POPUP, (UINT_PTR)hRun, L"&Процес");
	AppendMenuW(hMenubar, MF_POPUP, (UINT_PTR)hInfo, L"&Інформація");
	SetMenu(hwnd, hMenubar);
}

static int CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{

	if (uMsg == BFFM_INITIALIZED)
	{
		std::string tmp = (const char *)lpData;
		//std::cout << "path: " << tmp << std::endl;
		SendMessage(hwnd, BFFM_SETSELECTION, TRUE, lpData);
	}

	return 0;
}
void save()
{
	wchar_t szFileName[500] = L"";             /*буфер под имя файла*/
	OPENFILENAME ofn;
	memset(&ofn, 0, sizeof(ofn));
	GetCurrentDirectory(sizeof(szFileName), szFileName);
	wcscat(szFileName, L"\\*.*");
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = NULL; //hWnd;
	ofn.lpstrFile = szFileName;
	ofn.lpstrFile[0] = L'\0';
	ofn.nMaxFile = sizeof(szFileName);
	ofn.lpstrFilter = L"All\0*.*\0Text\0*.TXT\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;

	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST; //| OFN_FILEMUSTEXIST;

								   //ofn.lpstrFileTitle = L"Save model";
								   //ofn.nMaxFileTitle = sizeof(L"Save model");
	ofn.Flags = OFN_CREATEPROMPT;

	if (GetSaveFileName(&ofn))
	{
		char buffer[200];

		wcstombs(buffer, (LPCWSTR)ofn.lpstrFile, sizeof(buffer));

	
		std::wofstream myFile(buffer, std::ios::out | std::ios::binary);

		myFile.imbue(std::locale(myFile.getloc(), new std::codecvt_utf8_utf16<wchar_t>));
		myFile << ProgramLog;

		
	

	}
}
wstring BrowseFolder(std::string saved_path)
{
	TCHAR path[MAX_PATH];
	
	const char * path_param = saved_path.c_str();

	BROWSEINFO bi = { 0 };
	bi.lpszTitle = (L"Browse for folder...");
	bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
	bi.lpfn = BrowseCallbackProc;
	bi.lParam = (LPARAM)path_param;

	LPITEMIDLIST pidl = SHBrowseForFolder(&bi);

	if (pidl != 0)
	{
		//get the name of the folder and put it in path
		SHGetPathFromIDList(pidl, path);

		//free memory used
		IMalloc * imalloc = 0;
		if (SUCCEEDED(SHGetMalloc(&imalloc)))
		{
			imalloc->Free(pidl);
			imalloc->Release();
		}

		return path;
	}

	return L"";
}

void insertItem(wstring text, HWND& List)
{
	LVITEM item;
	memset(&item, 0, sizeof(item)); // Zero struct's Members

										//  Setting properties Of members:

	item.mask = LVIF_TEXT;   // Text Style
	item.cchTextMax = 256; // Max size of test
	item.iItem = 0;          // choose item  
	item.iSubItem = 0;       // Put in first coluom
	item.pszText = (LPWSTR)text.c_str();; // Text to display (can be from a char variable) (Items)

	SendMessage(List, LVM_INSERTITEM, 0, (LPARAM)&item); // Send info to the Listview


	//item.iSubItem = 1;
	//item.pszText =L"";
	//SendMessage(List, LVM_SETITEM, 0, (LPARAM)&item); // Enter text to SubItems
	
}

void find_files(wstring wrkdir,vector<wstring> &files)
{
	if (experimental::filesystem::is_empty(wrkdir.c_str()))
	{
		emptyFolders.push_back(wrkdir);
	}

	wstring wrkdirtemp = wrkdir;
	if (!wrkdirtemp.empty() && (wrkdirtemp[wrkdirtemp.length() - 1] != L'\\'))
	{
		wrkdirtemp += L"\\";
	}

	WIN32_FIND_DATA file_data = { 0 };
	HANDLE hFile = FindFirstFile((wrkdirtemp + L"*").c_str(), &file_data);
	/////
	
	if (hFile == INVALID_HANDLE_VALUE)
	{

		return;
	}
	

	do
	{
		if (file_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if ((wcscmp(file_data.cFileName, L".") != 0) &&
				(wcscmp(file_data.cFileName, L"..") != 0))
			{
				find_files(wrkdirtemp + file_data.cFileName,files);
			}
		}
		else
		{
			if ((file_data.dwFileAttributes & (FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM)) == 0)
			{
				wstring tmp = wrkdirtemp + file_data.cFileName+ to_wstring(file_data.nFileSizeLow);

				files.push_back(tmp);
				
				
				FileInfo t(wrkdirtemp, file_data.cFileName, to_wstring(file_data.nFileSizeHigh) + to_wstring(file_data.nFileSizeLow));
				info.push_back(t);

			}
		}
	} while (FindNextFile(hFile, &file_data));

	FindClose(hFile);
}


bool compare_files(const std::wstring& filename1, const std::wstring& filename2)
{
	std::ifstream file1(filename1, std::ifstream::ate | std::ifstream::binary); //open file at the end
	std::ifstream file2(filename2, std::ifstream::ate | std::ifstream::binary); //open file at the end
	const std::ifstream::pos_type fileSize = file1.tellg();

	if (fileSize != file2.tellg()) {
		return false; //different file size
	}

	file1.seekg(0); //rewind
	file2.seekg(0); //rewind

	std::istreambuf_iterator<char> begin1(file1);
	std::istreambuf_iterator<char> begin2(file2);

	return std::equal(begin1, std::istreambuf_iterator<char>(), begin2); //Second argument is end-of-range iterator
}
bool CompareType(wstring s1,wstring s2)
{
	
	wstring t1 = L"";
	wstring t2 = L"";
	for(int i =s1.size()-1;i>=0;--i)
	{
		if (s1[i] == '.') break;
		t1 += s1[i];
	}
	for (int i = s2.size() - 1; i >= 0; --i)
	{
		if (s2[i] == '.') break;
		t2 += s2[i];
	}
	
	if (t1 == t2)	
		return true;
	return false;
}
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int pParts[5];
	HWND hwndSb;
	static HWND hwndList;
	int iBarWidths[] = { 120, 285, -1 };
	vector<wstring> files;

    switch (message)
    {

	case WM_CREATE:
	{
	
		RegisterHotKey(hWnd,HOT_CNTRL_S,MOD_CONTROL, 0x53);//s key
		RegisterHotKey(hWnd, HOT_CNTRL_O, MOD_CONTROL, 0x4F);//o key
		RegisterHotKey(hWnd, HOT_ABOUT, MOD_CONTROL, 0x49);// i 
		RegisterHotKey(hWnd, HOT_RUN, MOD_CONTROL, 0x52);//r
		RegisterHotKey(hWnd, HOT_EXIT, 0, VK_ESCAPE);
		RegisterHotKey(hWnd, HOT_REMOVE, 0, VK_DELETE);
		
		hwndList = CreateWindow(WC_LISTVIEW, L"",
			WS_VISIBLE | WS_BORDER | WS_CHILD | LVS_REPORT | LVS_EDITLABELS,
			0, 50, 496 -50, 100,
			hWnd, (HMENU)ID_LIST, hInst, 0);
		SendMessage(hwndList, LVM_SETEXTENDEDLISTVIEWSTYLE,
			0, LVS_EX_FULLROWSELECT);
		////////////////////////////
		
		LVCOLUMN LvCol;
		
		memset(&LvCol, 0, sizeof(LvCol));                  // Zero Members
		LvCol.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;    // Type of mask                              
		LvCol.pszText = L"Однакові файли";                            // First Header Text
		LvCol.cx = 450;
		
	                       
		SendMessage(hwndList, LVM_INSERTCOLUMN, 0, (LPARAM)&LvCol); 
		                     
		
		
	

		CreateSimpleToolbar(hWnd);
		//auto message = CreateTrackingToolTip(IDM_NEW, hWnd, hInst, L"Tooltip message");
		//auto tooltip_mess = CreateToolTip(IDM_NEW, hWnd, hInst, L"Tooltip message");
		//if (tooltip_mess)
	//	SendMessage(message, TTM_ACTIVATE, TRUE, 0);

		hStatusBar = CreateWindowEx(NULL, STATUSCLASSNAME, NULL, WS_CHILD | WS_VISIBLE, 0, 0, 0, 0,
			hWnd, (HMENU)IDD_STATUSBAR, hInst, NULL);
		SendMessage(hStatusBar, SB_SETPARTS, 3, (LPARAM)iBarWidths);
		SendMessage(hStatusBar, SB_SETTEXT, 0, (LPARAM) L"");
		SendMessage(hStatusBar, SB_SETTEXT, 2, (LPARAM) L"");


		/* Creating and place the  Progress Bar inside the StatusBar */
		hProgressBar = CreateWindowEx(NULL, L"msctls_progress32", NULL, WS_CHILD | WS_VISIBLE, 2, 2, 163, 18,
			hStatusBar, (HMENU)IDD_PROGRESS, hInst, NULL);
		
		SendMessage(hProgressBar, PBM_SETPOS, 0, 0);
		 
		

		AddMenus(hWnd);
		break;
	}
	case WM_SIZE:
	{
		static RECT lpRect;
		GetWindowRect(hWnd, &lpRect);
		int nWidth;
		int nWidths[3];
		nWidth = LOWORD(lParam);
		int nHigth = LOWORD(wParam);
		SetWindowPos(hWndToolbar, 0,0,0, nWidth, lpRect.bottom - lpRect.top, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
		//SendMessage(hwndList, LVM_SETCOLUMNWIDTH,300,300);
	
		ListView_SetColumnWidth(hwndList,0,nWidth);
		SetWindowPos(hwndList,0,0,0, nWidth , lpRect.bottom-lpRect.top-130, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
		nWidths[0] = 170;
		nWidths[1] = 170 + (nWidth -170)/2;
		nWidths[2] = -1;

		SendMessage(hStatusBar, SB_SETPARTS, 3, (LPARAM)nWidths);
		SendMessage(hStatusBar, WM_SIZE, 0, 0);
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	case WM_GETMINMAXINFO:
	{
		LPMINMAXINFO lpMMI = (LPMINMAXINFO)lParam;
		lpMMI->ptMinTrackSize.x = 512;
		lpMMI->ptMinTrackSize.y = 378;
		break;
	}
   
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
			case IDM_INFO_ABOUT: 
			case IDM_ABOUT: 
				{
					/*WCHAR AboutClass[100];
					HWND hwndAbout = CreateWindowW(AboutClass, L"Про програму", WS_OVERLAPPEDWINDOW,
						CW_USEDEFAULT, 0, 320, 280, nullptr, nullptr, hInst, nullptr);*/
					DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
					break;
				}
			case IDM_FILE_OPEN:
			case IDM_OPEN:
				{

					
					emptyFolders.clear();
					info.clear();
					Path = BrowseFolder("");
					SendMessage(hStatusBar, SB_SETTEXT, 2,(LPARAM)Path.c_str());
					break;
				}
			case IDM_SAVE:
			case IDM_FILE_SAVE:
				{
					save();
					ProgramLog = L"";
					break;
				}
			case IDM_NEW:
			case IDM_RUN_RUN:
				{
					wstring log = L"початок процесу ";
					
					time_t timev;
					time(&timev);
					struct tm * now = localtime(&timev);
					log += std::to_wstring(now->tm_mon + 1);
					log += L"/";
					log += std::to_wstring(now->tm_mday);
					log += L"/";
					log += std::to_wstring(now->tm_year+1900);
					log += L" ";
					log += std::to_wstring(now->tm_hour);
					log += L":";
					log += std::to_wstring(now->tm_min);
					log += L":";
					log += std::to_wstring(now->tm_sec);
					ProgramLog += log;
					ProgramLog += L"\n";
					SendMessage(hStatusBar, SB_SETTEXT, 1, (LPARAM)log.c_str());
					log = L"";
					SendMessage(hProgressBar, PBM_SETPOS, 0, 0);
					emptyFolders.clear();
					info.clear();
					SendMessage(hwndList, LVM_DELETEALLITEMS, 0, 0);
					if (Path != L"") {
						find_files(Path, files);
					} else
					{
						MessageBox(hWnd, L"Тека не вибрана",L"Error", MB_OK);
					}
					sort(info.begin(), info.end());
					vector<bool>used;
					used.resize(info.size() + 1);
					std::fill(used.begin(), used.end(), 0);
					vector <FileInfo> t;
					t.clear();
					for (int i =1;i<info.size();++i)
					{
						int step = i*100 / info.size();
						SendMessage(hProgressBar, PBM_SETPOS, step, 0);
						if (info[i].atributes == info[i-1].atributes && CompareType(info[i].name,info[i-1].name) && compare_files(info[i].namePath,info[i].namePath))
						{
							wstring log = L"Аналіз файлу: ";
							log += info[i].name;
							ProgramLog += log;
							ProgramLog += L"\n";
							SendMessage(hStatusBar, SB_SETTEXT, 1, (LPARAM)log.c_str());
							if (!used[i]) {
								t.push_back(info[i]);
								used[i] = true;
							}
							if (!used[i-1]) {
								t.push_back(info[i-1]);
								used[i-1] = true;
							}
						}
					}
					info.clear();
					info.swap(t);
				
					//MessageBox(hWnd, std::to_wstring(emptyFolders.size()).c_str(), L"", MB_OK);
					for (auto i:info)
					{
						insertItem(i.namePath, hwndList);
					}
					for (auto i : emptyFolders)
					{
						insertItem(i, hwndList);
					}
					SendMessage(hProgressBar, PBM_SETPOS, 0, 0);
					log = L"Кінець процесу: ";
					time(&timev);
					now = localtime(&timev);
					log += std::to_wstring(now->tm_mon + 1);
					log += L"/";
					log += std::to_wstring(now->tm_mday);
					log += L"/";
					log += std::to_wstring(now->tm_year + 1900);
					log += L" ";
					log += std::to_wstring(now->tm_hour);
					log += L":";
					log += std::to_wstring(now->tm_min);
					log += L":";
					log += std::to_wstring(now->tm_sec);
					::ProgramLog += log;
					ProgramLog += L"\n";
					SendMessage(hStatusBar, SB_SETTEXT, 1, (LPARAM)log.c_str());

					break;
				}
			case IDM_RUN_REMOVE:
			case IDM_REMOVE:
				{
					auto selitm = ListView_GetNextItem(hwndList, -1, LVNI_SELECTED);
					if (selitm != -1) {
						wchar_t text[255]=L"";
						LVITEM LvItem;
						
						memset(&LvItem, 0, sizeof(LvItem));
						LvItem.mask = LVIF_TEXT;
						LvItem.iSubItem = 0;
						LvItem.pszText = text;
						LvItem.cchTextMax = 256;
						LvItem.iItem = selitm;
						ListView_GetItem(hwndList,&LvItem);
						if (experimental::filesystem::remove(text) == false)
						{
							MessageBox(hWnd, L"Файл не вдалось видалити", L"", MB_OK);
						}
						else
						{
							MessageBox(hWnd, L"Файл видалено", L"", MB_OK);
						}
						////////
						ListView_DeleteItem(hwndList, selitm);
						
					}
					break;
				}
			case IDM_FILE_QUIT:
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
		
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
            EndPaint(hWnd, &ps);
        }
        break;
	case WM_HOTKEY:

		if (HOT_CNTRL_S == (int)wParam)
		{
			save();
			ProgramLog = L"";
			break;
		}
		if (HOT_ABOUT == (int)wParam)
		{
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		}
		if (HOT_CNTRL_O == (int)wParam)
		{
			emptyFolders.clear();
			info.clear();
			Path = BrowseFolder("");
			SendMessage(hStatusBar, SB_SETTEXT, 2, (LPARAM)Path.c_str());
			break;
		}
		if (HOT_RUN == (int)wParam)
		{
			wstring log = L"початок процесу ";

			time_t timev;
			time(&timev);
			struct tm * now = localtime(&timev);
			log += std::to_wstring(now->tm_mon + 1);
			log += L"/";
			log += std::to_wstring(now->tm_mday);
			log += L"/";
			log += std::to_wstring(now->tm_year + 1900);
			log += L" ";
			log += std::to_wstring(now->tm_hour);
			log += L":";
			log += std::to_wstring(now->tm_min);
			log += L":";
			log += std::to_wstring(now->tm_sec);
			ProgramLog += log;
			ProgramLog += L"\n";
			SendMessage(hStatusBar, SB_SETTEXT, 1, (LPARAM)log.c_str());
			log = L"";
			SendMessage(hProgressBar, PBM_SETPOS, 0, 0);
			emptyFolders.clear();
			info.clear();
			SendMessage(hwndList, LVM_DELETEALLITEMS, 0, 0);
			if (Path != L"") {
				find_files(Path, files);
			}
			else
			{
				MessageBox(hWnd, L"Тека не вибрана", L"Error", MB_OK);
			}
			sort(info.begin(), info.end());
			vector<bool>used;
			used.resize(info.size() + 1);
			std::fill(used.begin(), used.end(), 0);
			vector <FileInfo> t;
			t.clear();
			for (int i = 1; i<info.size(); ++i)
			{
				int step = i * 100 / info.size();
				SendMessage(hProgressBar, PBM_SETPOS, step, 0);
				if (info[i].atributes == info[i - 1].atributes && CompareType(info[i].name, info[i - 1].name) && compare_files(info[i].namePath, info[i].namePath))
				{
					wstring log = L"Аналіз файлу: ";
					log += info[i].name;
					ProgramLog += log;
					ProgramLog += L"\n";
					SendMessage(hStatusBar, SB_SETTEXT, 1, (LPARAM)log.c_str());
					if (!used[i]) {
						t.push_back(info[i]);
						used[i] = true;
					}
					if (!used[i - 1]) {
						t.push_back(info[i - 1]);
						used[i - 1] = true;
					}
				}
			}
			info.clear();
			info.swap(t);

			//MessageBox(hWnd, std::to_wstring(emptyFolders.size()).c_str(), L"", MB_OK);
			for (auto i : info)
			{
				insertItem(i.namePath, hwndList);
			}
			for (auto i : emptyFolders)
			{
				insertItem(i, hwndList);
			}
			SendMessage(hProgressBar, PBM_SETPOS, 0, 0);
			log = L"Кінець процесу: ";
			time(&timev);
			now = localtime(&timev);
			log += std::to_wstring(now->tm_mon + 1);
			log += L"/";
			log += std::to_wstring(now->tm_mday);
			log += L"/";
			log += std::to_wstring(now->tm_year + 1900);
			log += L" ";
			log += std::to_wstring(now->tm_hour);
			log += L":";
			log += std::to_wstring(now->tm_min);
			log += L":";
			log += std::to_wstring(now->tm_sec);
			::ProgramLog += log;
			ProgramLog += L"\n";
			SendMessage(hStatusBar, SB_SETTEXT, 1, (LPARAM)log.c_str());

			break;
		}
		if (HOT_REMOVE == (int)wParam)
		{
			auto selitm = ListView_GetNextItem(hwndList, -1, LVNI_SELECTED);
			if (selitm != -1) {
				wchar_t text[255] = L"";
				LVITEM LvItem;

				memset(&LvItem, 0, sizeof(LvItem));
				LvItem.mask = LVIF_TEXT;
				LvItem.iSubItem = 0;
				LvItem.pszText = text;
				LvItem.cchTextMax = 256;
				LvItem.iItem = selitm;
				ListView_GetItem(hwndList, &LvItem);
				if (experimental::filesystem::remove(text) == false)
				{
					MessageBox(hWnd, L"Файл не вдалось видалити", L"", MB_OK);
				}
				else
				{
					MessageBox(hWnd, L"Файл видалено", L"", MB_OK);
				}
				////////
				ListView_DeleteItem(hwndList, selitm);

			}
			break;
		}
		if (HOT_EXIT == (int)wParam)
		{
			DestroyWindow(hWnd);
			break;
		}
		
		break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
	
    case WM_INITDIALOG:
		{
			SetDlgItemText(hDlg, IDC_STATIC, L"Zver UCleaner 2017  Версія 01.01.01.8007 \n\nПрограма призначена для пошуку та видалення однакових файлів та пустих папок\n\n\nВосьмушко Олександр");
			
			return (INT_PTR)TRUE;
		}
    case WM_COMMAND:
		
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
