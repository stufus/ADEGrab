#include "adegrab.h"

void performADExplorerCapture(HWND hDlg) {


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
			}
			break;

		case WM_DESTROY:
			PostQuitMessage(NULL);
			return NULL;
			break;

		case WM_INITDIALOG:
			
			break;

		default:
			return DefWindowProc(hDlg, msg, wParam, lParam);
			break;
	}

	return NULL;
}

// Entrypoint
int APIENTRY WinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPTSTR    lpCmdLine,
	_In_ int       nCmdShow) {
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	HWND hDlg;
	UINT Msg;

	if (hDlg = CreateDialogParam(hInstance, MAKEINTRESOURCE(DLG_MAIN), NULL, &mainDlgProc, NULL)) {
		ShowWindow(hDlg, SW_SHOW);
		while (GetMessage(&Msg, NULL, 0, 0)) {
			if (!IsDialogMessage(hDlg, &Msg)) {
				TranslateMessage(&Msg);
				DispatchMessage(&Msg);
			}
		}
	}
	ExitProcess((UINT) NULL);
}