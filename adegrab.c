#include "adegrab.h"

NOTIFYICONDATA ico = { 0 };
HINSTANCE hInstance;

void performADExplorerCapture(HWND hDlg) {


}

void trayIcon(HWND hDlg) {
	ico.cbSize = sizeof(NOTIFYICONDATA);
	ico.uID = 1;
	ico.uFlags = NIF_ICON | NIF_MESSAGE;
	ico.uCallbackMessage = CM_TRAY;
	ico.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(ICO_MAINICON));
	Shell_NotifyIcon(NIM_ADD, &ico);
}

// Message loop for main box
int CALLBACK mainDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam) {

	switch (msg) {

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
			
				case BTN_CAPTURE:
					performADExplorerCapture(hDlg);
					break;

				case BTN_TRAY:
					SendMessage(hDlg, WM_SIZE, SIZE_MINIMIZED, NULL);
					break;

				case BTN_QUIT:
					DestroyWindow(hDlg);
					break;

			}
			break;

		case WM_CLOSE:
			SendMessage(hDlg, WM_SIZE, SIZE_MINIMIZED, NULL);
			break;

		case WM_SIZE:
			if (wParam == SIZE_MINIMIZED) {
				ShowWindow(hDlg, SW_HIDE);
			} else if((wParam == SIZE_RESTORED)||(wParam==SIZE_MAXIMIZED)) {
				ShowWindow(hDlg, SW_SHOW);
				SetForegroundWindow(hDlg);
			}
			break;

		case WM_DESTROY:
			Shell_NotifyIcon(NIM_DELETE, &ico);
			PostQuitMessage(NULL);
			return NULL;
			break;

		case WM_INITDIALOG:
			trayIcon(hDlg);
			break;

		case CM_TRAY:
			if (wParam == 0) {
				switch (lParam) {
					case WM_LBUTTONDBLCLK:
						SendMessage(hDlg, WM_SIZE, SIZE_RESTORED, NULL);
						break;

					case WM_RBUTTONUP:
						MessageBox(hDlg, L"Context Menu", NULL, NULL);
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

	HWND hDlg;
	WNDCLASSEX wc = { 0 };
	UINT Msg;

	hInstance = hInst;

	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = &mainDlgProc;
	wc.cbWndExtra = DLGWINDOWEXTRA;
	wc.hInstance = hInst;
	wc.lpszClassName = L"ADEGrab";
	wc.lpszMenuName = MAKEINTRESOURCE(MNU_MAIN);
	wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(ICO_MAINICON));
	wc.hIconSm = wc.hIcon;
	wc.hCursor = LoadCursor(hInstance, IDC_ARROW);
	//RegisterClassEx(&wc);

	if (hDlg = CreateDialogParam(hInstance, MAKEINTRESOURCE(DLG_MAIN), NULL, &mainDlgProc, NULL)) {
		ShowWindow(hDlg, SW_SHOW);
		UpdateWindow(hDlg);
		while (GetMessage(&Msg, NULL, 0, 0)) {
			if (!IsDialogMessage(hDlg, &Msg)) {
				TranslateMessage(&Msg);
				DispatchMessage(&Msg);
			}
		}
	}
	ExitProcess((UINT) NULL);
}