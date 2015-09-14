#include <Windows.h>
#include <WinUser.h>
#include <Strsafe.h>
#include <Commctrl.h>

#include "resource.h"

#define CM_TRAY	WM_USER+3
#define ADEGRAB_IDENTIFIER "ADEGrab v1.0"
#define MAX_BUFFER_SIZE 100

typedef struct _ADMemInject {
	LVITEM li;
	TCHAR buffer[MAX_BUFFER_SIZE + 1];
} ADMemInject;
