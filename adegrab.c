#include "adegrab.h"

NOTIFYICONDATA ico = { 0 };
HINSTANCE hInstance;
HMENU popup;

void performADExplorerCapture(HWND hDlg) {


}

void addLog(HWND hDlg, char *message) {


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
	POINT coords;

	switch (msg) {

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
			
				// Buttons
				case BTN_CAPTURE:
					performADExplorerCapture(hDlg);
					break;

				case BTN_TRAY:
					SendMessage(hDlg, WM_SYSCOMMAND, SC_MINIMIZE, NULL);
					break;

				case BTN_QUIT:
					SendMessage(hDlg, WM_DESTROY, NULL, NULL);
					break;

				// Menu options
				case ID_ADEGRAB_EXIT:
					SendMessage(hDlg, WM_DESTROY, NULL, NULL);
					break;

				case ID_ADEGRAB_CAPTURETOCLIPBOARD:
					break;

				case ID_ADEGRAB_CAPTURETOLOGFILE:
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
			trayIcon(hDlg);
			popup = LoadMenu(hInstance, MAKEINTRESOURCE(MNU_MAIN));
			ShowWindow(hDlg, SW_SHOWNORMAL);
			break;

		case CM_TRAY:
			if (wParam == 0) {
				switch (lParam) {
					case WM_LBUTTONDBLCLK:
						SendMessage(hDlg, WM_SYSCOMMAND, SC_RESTORE, NULL);
						break;

					case WM_RBUTTONUP:
						GetCursorPos(&coords);
						SetForegroundWindow(hDlg);
						TrackPopupMenuEx(popup, TPM_RIGHTALIGN, coords.x, coords.y, hDlg, NULL);
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