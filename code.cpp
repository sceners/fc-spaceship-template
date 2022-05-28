////////////////////////////////////////////////////////////
// Forumcrack Template contest
// Template by jB
// Apr. 22th, 2006
//
////////////////////////////////////////////////////////////

#define WIN32_LEAN_AND_MEAN

#include "template.h"

#define MAX_SERIAL 100

extern "C" void process_serial(char *serial);
extern "C" void loadmusic(void);
DWORD WINAPI GenererSerial(HWND hwnd);
LRESULT CALLBACK ButtWindProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

HINSTANCE hInst;
LONG prev_proc;

// Message handlers for wizard button 
BOOL bMouseOverGenerateButton = FALSE;
BOOL bMouseOverAboutButton = FALSE;
BOOL bMouseOverExitButton = FALSE;
RECT rExit, rAbout, rGenerate, rWindow;
HWND hExit, hAbout, hGenerate, hWindow;
FMUSIC_MODULE *mod=NULL;

BOOL CALLBACK DlgProc( HWND hwnd, UINT uMsg, WPARAM wParam,LPARAM lParam)
{ 
	static HBITMAP hbmAbout, hbmAboutHover, hbmAboutDown, 
		hbmGenerate, hbmGenerateHover, hbmGenerateDown, 
		hbmExit, hbmExitHover, hbmExitDown;
	LPDRAWITEMSTRUCT lpdis;

	HDC hdc, hdcMem, hdcSerial;
	static HWND hKeygen, hSerial;
	WORD yPos;
	static LOGFONT lf;
	HFONT hFont, hFontOld;
	TCHAR szProgname[] = PROGNAME " - Keygen";

   switch (uMsg)
   {
   		case WM_CLOSE:
   			EndDialog(hwnd,0); 
   			break;

		case WM_LBUTTONDOWN:
			yPos = GET_Y_LPARAM(lParam);
			PostMessage(hwnd, WM_NCLBUTTONDOWN, HTCAPTION, lParam);
			return FALSE;

		case WM_MOUSEMOVE:
			bMouseOverGenerateButton = FALSE;
			InvalidateRect(GetDlgItem(hwnd, IDC_GENERATE), NULL, FALSE);
			bMouseOverAboutButton = FALSE;
			InvalidateRect(GetDlgItem(hwnd, IDC_ABOUT), NULL, FALSE);
			bMouseOverExitButton = FALSE;
			InvalidateRect(GetDlgItem(hwnd, IDC_EXIT), NULL, FALSE);
			break;

		case WM_DRAWITEM:
			lpdis = (LPDRAWITEMSTRUCT) lParam; 
			hdcMem = CreateCompatibleDC(lpdis->hDC);
			switch(lpdis->CtlID)
			{
			case IDC_GENERATE:
				if(bMouseOverGenerateButton)
				{
					bMouseOverGenerateButton=FALSE;
					SelectObject(hdcMem, hbmGenerateHover);
				}
				else
				{
				if (lpdis->itemState & ODS_SELECTED)  // if selected 
					SelectObject(hdcMem, hbmGenerateDown); 
				else 
					SelectObject(hdcMem, hbmGenerate);
				}
				break;
			case IDC_ABOUT:
				if(bMouseOverAboutButton)
				{
					bMouseOverAboutButton=FALSE;
					SelectObject(hdcMem, hbmAboutHover);
				}
				else
				{
				if (lpdis->itemState & ODS_SELECTED)  // if selected 
					SelectObject(hdcMem, hbmAboutDown); 
				else 
					SelectObject(hdcMem, hbmAbout);
				}
				break;
			case IDC_EXIT:
				if(bMouseOverExitButton)
				{
					bMouseOverExitButton=FALSE;
					SelectObject(hdcMem, hbmExitHover);
				}
				else
				{
				if (lpdis->itemState & ODS_SELECTED)  // if selected 
					SelectObject(hdcMem, hbmExitDown); 
				else 
					SelectObject(hdcMem, hbmExit);
				}
				break;
			}			
			BitBlt( 
				lpdis->hDC,         // destination DC 
				lpdis->rcItem.left, // x upper left 
				lpdis->rcItem.top,  // y upper left 

				// The next two lines specify the width and 
				// height. 
				lpdis->rcItem.right - lpdis->rcItem.left, 
				lpdis->rcItem.bottom - lpdis->rcItem.top, 
				hdcMem,    // source device context 
				0, 0,      // x and y upper left 
				SRCCOPY);  // raster operation 

			DeleteDC(hdcMem); 
			return TRUE;

		case WM_INITDIALOG:
			loadmusic();
			mod = FMUSIC_LoadSong(NULL, NULL);
			if(mod) FMUSIC_PlaySong(mod);

			AnimateWindow(hwnd, 1000, AW_BLEND);

			hGenerate=GetDlgItem(hwnd, IDC_GENERATE);
			hAbout=GetDlgItem(hwnd, IDC_ABOUT);
			hExit=GetDlgItem(hwnd, IDC_EXIT);

			GetWindowRect(hwnd, &rWindow);
			GetWindowRect(hGenerate, &rGenerate);
			GetWindowRect(hAbout, &rAbout);
			GetWindowRect(hExit, &rExit);

			hbmGenerate = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_GENERATE));
			hbmGenerateHover = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_GENERATE_HOVER));
			hbmGenerateDown = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_GENERATE_DOWN));
			hbmExit = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_EXIT));
			hbmExitHover = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_EXIT_HOVER));
			hbmExitDown = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_EXIT_DOWN));
			hbmAbout = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_ABOUT));
			hbmAboutHover = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_ABOUT_HOVER));
			hbmAboutDown = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_ABOUT_DOWN));

			ZeroMemory(&lf, sizeof(LOGFONT));
			strcpy(lf.lfFaceName,"Tahoma");
			lf.lfHeight = 17;
			lf.lfWeight = FW_BOLD;
			lf.lfQuality = ANTIALIASED_QUALITY;

			hWindow = hwnd;
			hKeygen = GetDlgItem(hwnd, IDC_KEYGEN);
			hSerial = GetDlgItem(hwnd, IDC_SERIAL);

			srand(GetTickCount());
			SetWindowText(hwnd,PROGNAME " - Keygen");
			GenererSerial(hwnd);
			SetFocus(GetDlgItem(hwnd, IDC_SERIAL));
			InvalidateRect(hwnd, NULL, FALSE);
			prev_proc = SetWindowLong(hGenerate, GWLP_WNDPROC, (LONG)ButtWindProc);
			SetWindowLong(hAbout, GWLP_WNDPROC, (LONG)ButtWindProc);
			SetWindowLong(hExit, GWLP_WNDPROC, (LONG)ButtWindProc);
			return FALSE;

		case WM_CTLCOLORSTATIC:
		case WM_CTLCOLOREDIT:
			SetBkMode((HDC)wParam, TRANSPARENT);
			SetTextColor((HDC)wParam, RGB(150, 150, 150));
			return (BOOL)GetStockObject(BLACK_BRUSH);

		case WM_PAINT:
			UpdateWindow(hwnd);
			hdc = GetDC(hKeygen);
			hFont = CreateFontIndirect(&lf);
			hFontOld = (HFONT)SelectObject(hdc, hFont);
			SetTextColor(hdc, RGB(150, 150, 150));
			SetBkMode(hdc, TRANSPARENT);
			TextOut(hdc, 10, 3, szProgname, sizeof(szProgname)-1);
			SelectObject(hdc, hFontOld);
			DeleteObject(hFont);
			ReleaseDC(hKeygen, hdc);
			
			hdcSerial = GetDC(hSerial);
			SetBkMode(hdcSerial, TRANSPARENT);
			SetBkColor(hdcSerial, RGB(0,0,0));			
			SetTextColor(hdcSerial, RGB(150, 150, 150));
			ReleaseDC(hSerial, hdcSerial);
			InvalidateRect(hSerial, NULL, FALSE);
			return FALSE;
			
 		case WM_COMMAND:
   			switch(LOWORD(wParam))
   			{
   			case IDC_GENERATE:
				GenererSerial(hwnd);
				break;
			case IDC_EXIT:
				EndDialog(hwnd,0);
				break;
			case IDC_ABOUT:
				DialogBoxParam(hInst,MAKEINTRESOURCE(IDD_ABOUT),hwnd,DlgProc2,(LPARAM)NULL);
				break;
   			}
 		default:
   			return FALSE;
   }   
   return TRUE;
}

LRESULT CALLBACK ButtWindProc(
								 HWND hWnd,
								 UINT message,
								 WPARAM wParam,
								 LPARAM lParam)
{
	long xPos, yPos;
	POINT p;
	switch (message)
	{
	case WM_MOUSEMOVE:	
		GetCursorPos(&p);
		xPos=p.x;
		yPos=p.y;
		bMouseOverGenerateButton=FALSE;
		bMouseOverAboutButton=FALSE;
		bMouseOverExitButton=FALSE;

		GetWindowRect(hWindow, &rWindow);
		GetWindowRect(hGenerate, &rGenerate);
		GetWindowRect(hAbout, &rAbout);
		GetWindowRect(hExit, &rExit);

		if(yPos > rGenerate.top && yPos < rGenerate.bottom)
			if(xPos > rGenerate.left && xPos < rGenerate.right)
				bMouseOverGenerateButton = TRUE;
		if(yPos > rAbout.top && yPos < rAbout.bottom)
			if(xPos > rAbout.left && xPos < rAbout.right)
				bMouseOverAboutButton = TRUE;
		if(yPos > rExit.top-rWindow.top && yPos < rExit.bottom)
			if(xPos > rExit.left && xPos < rExit.right)
				bMouseOverExitButton = TRUE;
		InvalidateRect(hGenerate, NULL, FALSE);
		InvalidateRect(hExit, NULL, FALSE);
		InvalidateRect(hAbout, NULL, FALSE);
		break;
	}

	// Any messages we don't process must be passed onto the original window function
	return CallWindowProc((WNDPROC)prev_proc, hWnd, message, wParam, lParam);
}

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow)
{ 
	hInstance = GetModuleHandle(NULL);
	hInst = hInstance;
	DialogBoxParam(hInstance,MAKEINTRESOURCE(IDD_KEYGEN),NULL,DlgProc,(LPARAM)NULL);
 	return 0;
}

DWORD WINAPI GenererSerial(HWND hwnd){
	char serial[MAX_SERIAL];
	memset(serial,0,MAX_SERIAL);
	process_serial(serial);
	SetDlgItemText(hwnd, IDC_SERIAL,serial);
    return 0;
}