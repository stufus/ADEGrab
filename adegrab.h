#include <Windows.h>
#include <WinUser.h>
#include <Strsafe.h>
#include <Commctrl.h>

#include "resource.h"

#define CM_TRAY	WM_USER+3
#define ADEGRAB_IDENTIFIER TEXT("ADEGrab v1.0")
#define MAX_BUFFER_SIZE 250

typedef struct _ADMemInject {
	LVITEM li;
	TCHAR buffer[MAX_BUFFER_SIZE + 1];
} ADMemInject;

#define NTDDI_VERSION NTDDI_WIN2K
#define NTDDI_VERSION NTDDI_WINXP
#define NTDDI_VERSION NTDDI_VISTA