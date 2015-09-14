#include "adegrab.h"

NOTIFYICONDATA ico = { 0 };
HINSTANCE hInstance;
HMENU popup, popup_sub;

void addLog(HWND hDlg, TCHAR *message) {
	SYSTEMTIME lt;
	TCHAR buf[400] = { 0 };

	GetLocalTime(&lt);
	StringCbPrintf(&buf, 400, TEXT("[%d/%d/%d %d:%d:%d] %s"), lt.wDay, lt.wMonth, lt.wYear, lt.wHour, lt.wMinute, lt.wSecond, message);
	SendDlgItemMessage(hDlg, LIST_LOG, LB_ADDSTRING, 0, (LPARAM)&buf);
}

void performADExplorerCapture(HWND hDlg, HMENU menu) {
	MENUITEMINFO mi = { 0 };
	HANDLE hFile;
	TCHAR log[300];
	DWORD dwWritten;

	HWND ADExplorer;
	HWND ADExplorerListView;
	int counter;
	int itemCount;
	int maxchars;
	ADMemInject LVQuery;
	DWORD processId;
	DWORD threadId;
	HGLOBAL hClipboardMem;

	HANDLE remoteProcess;
	DWORD remoteMemory;
	DWORD err;
	
	TCHAR *completebuffer_unicode = NULL;
	int cbansi_size = NULL;
	char *completebuffer_ansi = NULL;

	mi.cbSize = sizeof(MENUITEMINFO);
	mi.fMask = MIIM_STATE;

	// Retrieve the encompassing window
	if (ADExplorer = FindWindowExW(NULL, NULL, TEXT("#32770"), TEXT("Search Container"))) {

		// Now retrieve the handle of the specific listview
		if (ADExplorerListView = FindWindowExW(ADExplorer, NULL, TEXT("SysListView32"), TEXT("List1"))) {

			// Update log
			StringCbPrintf(&log, 300, TEXT("Found AD Explorer listbox (Handle: %d)."), ADExplorerListView);
			addLog(hDlg, &log);

			// Now get the process Id and thread ID of the AD Explorer process
			if (threadId = GetWindowThreadProcessId(ADExplorerListView, &processId)) {

				// Obtain privileges that we will need to write to its memory
				if (remoteProcess = OpenProcess(PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE, FALSE, processId)) {

					StringCbPrintf(&log, 300, TEXT("Opened AD Explorer process (Handle: %d)."), remoteProcess);
					addLog(hDlg, &log);

					// Make sure there are some items in the listview
					if (itemCount = ListView_GetItemCount(ADExplorerListView)) {

						StringCbPrintf(&log, 300, TEXT("Found %d item(s)."), itemCount);
						addLog(hDlg, &log);

						// Allocate the main buffer which is (num_items * max size) + \r\n
						maxchars = 2 + (itemCount*MAX_BUFFER_SIZE);
						completebuffer_unicode = calloc(sizeof(TCHAR), maxchars);

						StringCbPrintf(&log, 300, TEXT("Temporary buffer is %d chars (unicode = 2x)."), maxchars);
						addLog(hDlg, &log);

						// Allocate memory in remote process in the format [LVITEM][TCHAR String][NULL]
						remoteMemory = (DWORD)VirtualAllocEx(remoteProcess, NULL, sizeof(LVQuery), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

						for (counter = 0; counter < itemCount; counter++) {

							// Loop through the items retrieving each one. Note that we explicitly clear the memory between
							// each iteration because LVM_GETITEM updates the struct and it is cleaner to start with a "blank" one each time.
							// Fill in the local listitem struct to perform the query
							SecureZeroMemory((PVOID)&LVQuery, sizeof(LVQuery));
							LVQuery.li.cchTextMax = MAX_BUFFER_SIZE;
							LVQuery.li.pszText = (LPWSTR)remoteMemory + sizeof(LVQuery.li);
							LVQuery.li.mask = LVIF_TEXT | LVIF_COLUMNS;
							LVQuery.li.iSubItem = 0;
							LVQuery.li.iItem = counter;

							// Write the listitem struct to AD Explorer's memory
							WriteProcessMemory(remoteProcess, (LPVOID)remoteMemory, &LVQuery, sizeof(LVQuery), NULL);

							// Request the text from the ListView control
							SendMessage(ADExplorerListView, LVM_GETITEM, NULL, remoteMemory);

							// Read from the process - retrieve the initial LVITEM and then retrieve the buffer
							// It is necessary to do this twice because there is no guarantee that the result is written to the buffer that we offered
							ReadProcessMemory(remoteProcess, (LPVOID)remoteMemory, &(LVQuery.li), sizeof(LVQuery.li), NULL);
							ReadProcessMemory(remoteProcess, (LPVOID)LVQuery.li.pszText, &(LVQuery.buffer), sizeof(LVQuery.buffer), NULL);

							// Add the string to the main buffer
							if (strlen(LVQuery.buffer)) {
								StringCbCat(completebuffer_unicode, maxchars, LVQuery.buffer);
								StringCbCat(completebuffer_unicode, maxchars, TEXT("\r\n"));
							}

						}

						SetDlgItemText(hDlg, EDIT_CAPTURED, completebuffer_unicode);

						cbansi_size = WideCharToMultiByte(CP_UTF8, NULL, completebuffer_unicode, -1, completebuffer_ansi, 0, NULL, NULL);
						completebuffer_ansi = calloc(sizeof(CHAR), cbansi_size);
						if (cbansi_size && WideCharToMultiByte(CP_UTF8, NULL, completebuffer_unicode, -1, completebuffer_ansi, cbansi_size, NULL, NULL)) {
							StringCbPrintf(&log, 300, TEXT("Converted %d characters to ANSI (multibyte) from Unicode."), cbansi_size);
							addLog(hDlg, &log);
						}

						// Write to clipboard?
						GetMenuItemInfo(menu, MAKEINTRESOURCE(ID_ADEGRAB_CAPTURETOCLIPBOARD), FALSE, &mi);
						if (mi.fState & MFS_CHECKED) {
							hClipboardMem = GlobalAlloc(GMEM_MOVEABLE, cbansi_size);
							memcpy(GlobalLock(hClipboardMem), completebuffer_ansi, cbansi_size);
							GlobalUnlock(hClipboardMem);
							OpenClipboard(hDlg);
							EmptyClipboard();
							if (SetClipboardData(CF_TEXT, hClipboardMem)) {
								StringCbPrintf(&log, 300, TEXT("Written %d characters to clipboard."), cbansi_size);
								addLog(hDlg, &log);
							}
							CloseClipboard();
						}

						// Save to file?
						GetMenuItemInfo(menu, MAKEINTRESOURCE(ID_ADEGRAB_CAPTURETOLOGFILE), FALSE, &mi);
						if (mi.fState & MFS_CHECKED) {
							if (hFile = CreateFile(TEXT("adegrab.log"), FILE_GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0)) {
								SetFilePointer(hFile, 0, NULL, FILE_END);
								WriteFile(hFile, completebuffer_ansi, cbansi_size-1, &dwWritten, NULL);
								StringCbPrintf(&log, 300, TEXT("Written %d byte(s) to output file."), dwWritten);
								CloseHandle(hFile);
							}
							else {
								StringCbPrintf(&log, 300, TEXT("Unable to open output file (Error %d)."), GetLastError());
							}
							addLog(hDlg, &log);
						}

						free(completebuffer_ansi);
						free(completebuffer_unicode);
						// Clean up
						VirtualFreeEx(remoteProcess, (LPVOID)remoteMemory, NULL, MEM_RELEASE);
						CloseHandle(remoteProcess);
					}

				}
			}
		} else {
			addLog(hDlg, TEXT("Unable to find ListView handle inside Search Container window."));
		}
	} else {
		addLog(hDlg, TEXT("Unable to find Search Container window."));
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
				case ID_ADEGRAB_RESTORE:
					SendMessage(hDlg, WM_SYSCOMMAND, SC_RESTORE, NULL);
					break;

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
					case WM_LBUTTONDBLCLK:
						performADExplorerCapture(hDlg, popup);
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