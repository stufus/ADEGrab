#include "adegrab.h"

NOTIFYICONDATA ico = { 0 };
HINSTANCE hInstance;
HMENU popup, popup_sub;

void addLog(HWND hDlg, TCHAR *message) {
	SYSTEMTIME lt;
	TCHAR buf[100] = { 0 };

	GetLocalTime(&lt);
	StringCbPrintf(&buf, 100, TEXT("[%d/%d/%d %d:%d:%d] %s"), lt.wDay, lt.wMonth, lt.wYear, lt.wHour, lt.wMinute, lt.wSecond, message);
	SendDlgItemMessage(hDlg, LIST_LOG, LB_ADDSTRING, 0, (LPARAM)&buf);
}

void performADExplorerCapture(HWND hDlg, HMENU menu) {
	MENUITEMINFO mi = { 0 };
	HANDLE hFile;
	TCHAR log[100];
	DWORD dwWritten;

	mi.cbSize = sizeof(MENUITEMINFO);
	mi.fMask = MIIM_STATE;
	
	GetMenuItemInfo(menu, MAKEINTRESOURCE(ID_ADEGRAB_CAPTURETOCLIPBOARD), FALSE, &mi);
	if (mi.fState & MFS_CHECKED) {
		// Copy to clipboard
	}

	GetMenuItemInfo(menu, MAKEINTRESOURCE(ID_ADEGRAB_CAPTURETOLOGFILE), FALSE, &mi);
	if (mi.fState & MFS_CHECKED) {
		if (hFile = CreateFile(TEXT("adegrab.log"), FILE_GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0)) {
			SetFilePointer(hFile, 0, NULL, FILE_END);
			//WriteFile(hFile, "STUFUS", 6, &dwWritten, NULL);
			StringCbPrintf(&log, 100, TEXT("Written %d byte(s) to output file."), dwWritten);
			CloseHandle(hFile);
		} else {
			StringCbPrintf(&log, 100, TEXT("Unable to open output file (Error %d)."), GetLastError());
		}
		addLog(hDlg, &log);
	}

}

void trayIcon(HWND hDlg) {
	ico.cbSize = sizeof(NOTIFYICONDATA);
	ico.uID = 0;
	ico.uFlags = NIF_ICON | NIF_MESSAGE;
	ico.hWnd = hDlg;
	ico.uCallbackMessage = CM_TRAY;
	ico.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(ICO_MAINICON));
	Shell_NotifyIcon(NIM_ADD, &ico);
}

// Message loop for main box
int CALLBACK mainDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
	POINT coords = { 0 };
	MENUITEMINFO mi = { 0 };

	switch (msg) {

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
			
				// Buttons
				case BTN_CAPTURE:
					performADExplorerCapture(hDlg, popup);
					break;

				case BTN_QUIT:
					SendMessage(hDlg, WM_DESTROY, NULL, NULL);
					break;

				// Menu options
				case ID_ADEGRAB_EXIT:
					SendMessage(hDlg, WM_DESTROY, NULL, NULL);
					break;

				case ID_ADEGRAB_CAPTURE:
					performADExplorerCapture(hDlg, popup);
					break;

				case ID_ADEGRAB_CAPTURETOCLIPBOARD:
				case ID_ADEGRAB_CAPTURETOLOGFILE:
					mi.cbSize = sizeof(MENUITEMINFO);
					mi.fMask = MIIM_STATE;
					GetMenuItemInfo(popup, LOWORD(wParam), FALSE, &mi);
					if (mi.fState & MFS_CHECKED) {
						mi.fState &= ~MFS_CHECKED;
					} else if (!(mi.fState & MFS_CHECKED)) {
						mi.fState |= MFS_CHECKED;
					}
					SetMenuItemInfo(popup, LOWORD(wParam), FALSE, &mi);
					break;

				default:
					return FALSE;
			}
			break;

		case WM_SYSCOMMAND:
			switch (wParam) {
				case SC_MINIMIZE:
					ShowWindow(hDlg, SW_HIDE);
					break;
				case SC_RESTORE:
					ShowWindow(hDlg, SW_SHOW);
					break;
				default:
					return FALSE;
			}
			break;

		case WM_CLOSE:
			SendMessage(hDlg, WM_DESTROY, NULL, NULL);
			break;

		case WM_DESTROY:
			Shell_NotifyIcon(NIM_DELETE, &ico);
			DestroyMenu(popup);
			EndDialog(hDlg, NULL);
			break;

		case WM_INITDIALOG:
			popup = LoadMenu(hInstance, MAKEINTRESOURCE(MNU_MAIN));
			popup_sub = GetSubMenu(popup, 0);
			trayIcon(hDlg);
			SetMenu(hDlg, popup);
			ShowWindow(hDlg, SW_SHOWNORMAL);
			addLog(hDlg, TEXT(ADEGRAB_IDENTIFIER));
			break;

		case CM_TRAY:
			if (wParam == 0) {
				switch (lParam) {
					case WM_LBUTTONUP:
						SendMessage(hDlg, WM_SYSCOMMAND, SC_RESTORE, NULL);
						break;

					case WM_RBUTTONUP:
						GetCursorPos(&coords);
						SetForegroundWindow(hDlg);
						TrackPopupMenuEx(popup_sub, TPM_RIGHTALIGN, coords.x, coords.y, hDlg, NULL);
						PostMessage(hDlg, WM_NULL, NULL, NULL);
						break;
				}
			}
			break;

		default:
			return FALSE;
			break;
	}

	return TRUE;
}

// Entrypoint
int APIENTRY WinMain(_In_ HINSTANCE hInst,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPTSTR    lpCmdLine,
	_In_ int       nCmdShow) {
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	hInstance = hInst;

	DialogBoxParam(hInst, MAKEINTRESOURCE(DLG_MAIN), NULL, &mainDlgProc, NULL);
	ExitProcess((UINT) NULL);
}