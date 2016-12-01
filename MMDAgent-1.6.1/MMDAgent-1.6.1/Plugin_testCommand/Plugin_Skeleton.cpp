/* ----------------------------------------------------------------- */
/* Window style changer plugin                                       */
/*                                                                   */
/*  Version: 0.0.0                                                   */
/*  Updated: 2012/02/19                                              */
/*  Author : Ru--en                                                  */
/*  License: BSD license                                             */
/*                                                                   */
/* ----------------------------------------------------------------- */
/*
/* ----------------------------------------------------------------- */
/*           The Toolkit for Building Voice Interaction Systems      */
/*           "MMDAgent" developed by MMDAgent Project Team           */
/*           http://www.mmdagent.jp/                                 */
/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2009-2011  Nagoya Institute of Technology          */
/*                           Department of Computer Science          */
/*  All rights reserved.                                             */
/*                                                                   */
/*  Copyright (c) 2012       Ru--en                                  */
/*  All rights reserved.                                             */
/*                                                                   */
/* Redistribution and use in source and binary forms, with or        */
/* without modification, are permitted provided that the following   */
/* conditions are met:                                               */
/*                                                                   */
/* - Redistributions of source code must retain the above copyright  */
/*   notice, this list of conditions and the following disclaimer.   */
/* - Redistributions in binary form must reproduce the above         */
/*   copyright notice, this list of conditions and the following     */
/*   disclaimer in the documentation and/or other materials provided */
/*   with the distribution.                                          */
/* - Neither the name of the MMDAgent project team nor the names of  */
/*   its contributors may be used to endorse or promote products     */
/*   derived from this software without specific prior written       */
/*   permission.                                                     */
/*                                                                   */
/* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND            */
/* CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,       */
/* INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF          */
/* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE          */
/* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS */
/* BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,          */
/* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED   */
/* TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,     */
/* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON */
/* ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,   */
/* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY    */
/* OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE           */
/* POSSIBILITY OF SUCH DAMAGE.                                       */
/* ----------------------------------------------------------------- */

/* definitions */

#ifdef _WIN32
#define EXPORT extern "C" __declspec(dllexport)
#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")
#else
#define EXPORT extern "C"
#endif /* _WIN32 */

#define PLUGINWINDOWSTYLE_NAME			"WindowStyle"
#define PLUGINWINDOWSTYLE_STYLE			"WINDOW_STYLE"
#define PLUGINWINDOWSTYLE_POSITION		"WINDOW_POSITION"
#define PLUGINWINDOWSTYLE_PLACEMENT		"WINDOW_PLACEMENT"
#define PLUGINWINDOWSTYLE_TRANSPARENCY	"WINDOW_TRANSPARENCY"

#define PLUGINWINDOWSTYLE_DEFAULT_CLASSNAME	"MMDAgent"
#define PLUGINWINDOWSTYLE_DEFAULT_TITLE		"MMDAgent - Toolkit for building voice interaction systems"
#define PLUGINWINDOWSTYLE_NULL				"<eps>"

/* headers */

#include "MMDAgent.h"

/* variables */

static bool enable;
static bool glassEnabled = false;

typedef struct _StyleSet {
	const char *name;
	const int  type;
	const long style;
} StyleSet;

const static StyleSet styles[] = {
	{ "APPWINDOW", GWL_EXSTYLE, WS_EX_APPWINDOW },
	{ "CLIENTEDGE", GWL_EXSTYLE, WS_EX_CLIENTEDGE },
	{ "LAYERED", GWL_EXSTYLE, WS_EX_LAYERED },
	{ "NOACTIVATE", GWL_EXSTYLE, WS_EX_NOACTIVATE },
	{ "PALETEWINDOW", GWL_EXSTYLE, WS_EX_PALETTEWINDOW },
	{ "STATICEDGE", GWL_EXSTYLE, WS_EX_STATICEDGE },
	{ "TOOLWINDOW", GWL_EXSTYLE, WS_EX_TOOLWINDOW },
	{ "TOPMOST", GWL_EXSTYLE, WS_EX_TOPMOST },
	{ "WINDOWEDGE", GWL_EXSTYLE, WS_EX_WINDOWEDGE },
	//{"EX_OVERLAPPEDWINDOW", 	GWL_EXSTYLE,WS_EX_OVERLAPPEDWINDOW},
	{ "OVERLAPPED", GWL_STYLE, WS_OVERLAPPED },
	{ "CAPTION", GWL_STYLE, WS_CAPTION },
	{ "SYSMENU", GWL_STYLE, WS_SYSMENU },
	{ "THICKFRAME", GWL_STYLE, WS_THICKFRAME },
	{ "MINIMIZEBOX", GWL_STYLE, WS_MINIMIZEBOX },
	{ "MAXIMIZEBOX", GWL_STYLE, WS_MAXIMIZEBOX },
	{ "OVERLAPPEDWINDOW", GWL_STYLE, WS_OVERLAPPEDWINDOW },
	{ NULL, 0, 0 }
};

enum ESwitch {
	SWITCH_NONE = 0,
	SWITCH_ON = 1,
	SWITCH_OFF = 2,
	SWITCH_INVERT = 3
};

typedef struct _PlacementSet {
	const char *name;
	const unsigned int command;
} PlacementSet;

const static PlacementSet placements[] = {
	{ "HIDE", SW_HIDE },
	{ "MINIMIZE", SW_MINIMIZE },
	{ "RESTORE", SW_RESTORE },
	{ "SHOW", SW_SHOW },
	{ "MAXIMIZE", SW_SHOWMAXIMIZED },
	{ "ACTIVATE", SW_SHOW },
	{ "NORMAL", SW_SHOWNORMAL },
	{ NULL, 0 }
};

#ifdef _WIN32
/* get window handle */
HWND getWindowHandle(const char* classname, const char* title)
{
	HWND hWnd;
	if (classname == NULL && title == NULL) {
		/* get MMDAgent default window */
		hWnd = FindWindow(
			PLUGINWINDOWSTYLE_DEFAULT_CLASSNAME,
			NULL	//PLUGINWINDOWSTYLE_DEFAULT_TITLE
			);
	}
	else {
		hWnd = FindWindow(classname, title);
	}
	return hWnd;
}
#endif

int getIntFromString(const char* str, const int default)
{
	if (str == NULL) {
		return default;
	}
	else if (MMDAgent_strequal(str, PLUGINWINDOWSTYLE_NULL)) {
		return default;
	}
	else {
		return MMDAgent_str2int(str);
	}
}

/* setWindowStyle: Set window style */
static void setWindowStyle(const char *args)
{
#ifdef _WIN32
	char *buff, *param1, *param2, *param3, *flag, *save;
	HWND hWnd;
	LONG dwStyle, dwExStyle;

	buff = MMDAgent_strdup(args);
	param1 = MMDAgent_strtok(buff, "|", &save); /* window class name */
	//param2 = MMDAgent_strtok(NULL, "|", &save); /* window title */
	param2 = NULL;	/* window title */
	param3 = MMDAgent_strtok(NULL, "|", &save); /* styles */

	/* check */
	if (buff == NULL) {
		free(buff);
		return;
	}

	/* apply default window class and title */
	if (MMDAgent_strequal(param1, PLUGINWINDOWSTYLE_NULL)) {
		param1 = NULL;
	}
	//if (MMDAgent_strequal(param2, PLUGINWINDOWSTYLE_NULL)) {
	//	param2 = NULL;
	//}

	/* get window handle */
	hWnd = getWindowHandle(param1, param2);
	if (hWnd == 0) {
		free(buff);
		return;
	}

	/* get window styles */
	dwStyle = GetWindowLong(hWnd, GWL_STYLE);
	dwExStyle = GetWindowLong(hWnd, GWL_EXSTYLE);

	/* parse styles */
	flag = MMDAgent_strtok(param3, ",", &save);
	while (flag != NULL) {
		ESwitch sw = SWITCH_NONE;		// 0:don't change, 1:enable, 2:invert, -1:disable

		/* disable flag if the first character of flag-name */
		switch (flag[0]) {
		case '-':
			sw = SWITCH_OFF;
			flag++;
			break;
		case '+':
			sw = SWITCH_ON;
			flag++;
			break;
		case '!':
			sw = SWITCH_INVERT;
			flag++;
			break;
		default:	// 
			sw = SWITCH_ON;
			break;
		}

		/* apply flag */
		for (int i = 0; styles[i].name != NULL; i++) {
			if (MMDAgent_strequal(flag, styles[i].name)) {
				LONG style;

				if (styles[i].type == GWL_STYLE) {
					style = dwStyle;
				}
				else if (styles[i].type == GWL_EXSTYLE) {
					style = dwExStyle;
				}

				switch (sw) {
				case SWITCH_ON:
					style = style | styles[i].style;
					break;
				case SWITCH_OFF:
					style = style & ~(styles[i].style);
					break;
				case SWITCH_INVERT:
					style = style ^ styles[i].style;
					break;
				}

				if (styles[i].type == GWL_STYLE) {
					dwStyle = style;
				}
				else if (styles[i].type == GWL_EXSTYLE) {
					dwExStyle = style;
				}

				break;
			}
		}

		flag = MMDAgent_strtok(NULL, ",", &save);
	}

	SetWindowLong(hWnd, GWL_STYLE, dwStyle);
	SetWindowLong(hWnd, GWL_EXSTYLE, dwExStyle);

	if (dwExStyle & WS_EX_TOPMOST) {
		SetWindowPos(
			hWnd, HWND_TOPMOST, NULL, NULL, NULL, NULL,
			SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE
			);
	}
	else {
		SetWindowPos(
			hWnd, HWND_NOTOPMOST, NULL, NULL, NULL, NULL,
			SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE
			);
	}
#endif
}

/* setWindowTransparency: Set window transparency */
static void setWindowTransparency(const char *args)
{
#ifdef _WIN32
	char *buff, *param1, *param2, *param3, *param4, *save;
	HWND hWnd;
	HRESULT hr;
	MARGINS margins;

	buff = MMDAgent_strdup(args);
	param1 = MMDAgent_strtok(buff, "|", &save); /* window class name */
	//param2 = MMDAgent_strtok(NULL, "|", &save); /* window title */
	param2 = NULL;	/* window title */
	param3 = MMDAgent_strtok(NULL, "|", &save); /* styles */
	param4 = MMDAgent_strtok(NULL, "|", &save); /* styles */

	/* check */
	if (buff == NULL) {
		free(buff);
		return;
	}

	/* apply default window class and title */
	if (MMDAgent_strequal(param1, PLUGINWINDOWSTYLE_NULL)) {
		param1 = NULL;
	}
	//if (MMDAgent_strequal(param2, PLUGINWINDOWSTYLE_NULL)) {
	//	param2 = NULL;
	//}

	/* get window handle */
	hWnd = getWindowHandle(param1, param2);
	if (hWnd == 0) {
		free(buff);
		return;
	}

	/* parse styles */
	if (MMDAgent_strequal(param3, "OPACITY")) {
		int alpha = getIntFromString(param4, -1);

		if (alpha >= 0) {
			LONG dwExStyle = GetWindowLong(hWnd, GWL_EXSTYLE);
			dwExStyle = dwExStyle | WS_EX_LAYERED;
			SetWindowLong(hWnd, GWL_EXSTYLE, dwExStyle);
			SetLayeredWindowAttributes(hWnd, RGB(0, 0, 0), alpha * 0xFF / 100, LWA_ALPHA);
		}
	}
	else if (MMDAgent_strequal(param3, "GLASS")) {
		/* apply Aero glass effect */
		if (MMDAgent_strequal(param4, "OFF")) {
			margins.cxLeftWidth = margins.cxRightWidth = margins.cyTopHeight = margins.cyBottomHeight = 0;
			hr = DwmExtendFrameIntoClientArea(hWnd, &margins);
		}
		else {
			margins.cxLeftWidth = margins.cxRightWidth = margins.cyTopHeight = margins.cyBottomHeight = -1;
			hr = DwmExtendFrameIntoClientArea(hWnd, &margins);
		}
	}
	else if (MMDAgent_strequal(param3, "LAYERED")) {
		// not implemented.
	}
	else if (MMDAgent_strequal(param3, "RESTORE")) {
		SetLayeredWindowAttributes(hWnd, RGB(0, 0, 0), 0xFF, LWA_ALPHA);
		margins.cxLeftWidth = margins.cxRightWidth = margins.cyTopHeight = margins.cyBottomHeight = 0;
		hr = DwmExtendFrameIntoClientArea(hWnd, &margins);
	}


#endif
}


/* setWindowPosition: Set window position and z-order */
static void setWindowPosition(const char *args)
{
#ifdef _WIN32
	char *buff, *param1, *param2, *param3, *param4, *save, *tmp;
	int x, y, w, h;
	bool positionChanged = false;
	bool sizeChanged = false;
	RECT rect;
	HWND hWnd;

	buff = MMDAgent_strdup(args);
	param1 = MMDAgent_strtok(buff, "|", &save); /* window class name */
	//param2 = MMDAgent_strtok(NULL, "|", &save); /* window title */
	param2 = NULL;	/* window title */
	param3 = MMDAgent_strtok(NULL, "|", &save); /* position (x,y)*/
	param4 = MMDAgent_strtok(NULL, "|", &save); /* size (width,height)*/

	/* check */
	if (buff == NULL) {
		free(buff);
		return;
	}

	/* get window handle */
	hWnd = getWindowHandle(param1, param2);
	if (hWnd == 0) {
		free(buff);
		return;
	}

	GetWindowRect(hWnd, (LPRECT)&rect);

	/* parse parameters */
	x = rect.left;
	y = rect.top;
	w = rect.right - rect.left + 1;
	h = rect.bottom - rect.top + 1;
	if (!MMDAgent_strequal(param3, PLUGINWINDOWSTYLE_NULL)) {
		tmp = MMDAgent_strtok(param3, ",", &save);
		x = getIntFromString(tmp, x);

		tmp = MMDAgent_strtok(NULL, ",", &save);
		y = getIntFromString(tmp, y);
		positionChanged = true;
	}

	if (!MMDAgent_strequal(param4, PLUGINWINDOWSTYLE_NULL)) {
		tmp = MMDAgent_strtok(param4, ",", &save);
		w = getIntFromString(tmp, w);

		tmp = MMDAgent_strtok(NULL, ",", &save);
		h = getIntFromString(tmp, h);
		sizeChanged = true;
	}
	SetWindowPos(
		hWnd, NULL,
		x, y, w, h,
		SWP_NOACTIVATE | SWP_NOZORDER
		);
#endif
}

/* setWindowPlacement: Set window placement (normal / maximize / minimize) */
static void setWindowPlacement(const char* args)
{
#ifdef _WIN32
	char *buff, *param1, *param2, *param3, *save;
	WINDOWPLACEMENT placement;
	HWND hWnd;

	buff = MMDAgent_strdup(args);
	param1 = MMDAgent_strtok(buff, "|", &save); /* window class name */
	//param2 = MMDAgent_strtok(NULL, "|", &save); /* window title */
	param2 = NULL;	/* window title */
	param3 = MMDAgent_strtok(NULL, "|", &save); /* placement */

	/* check */
	if (buff == NULL) {
		free(buff);
		return;
	}

	/* get window handle */
	hWnd = getWindowHandle(param1, param2);
	if (hWnd == 0) {
		free(buff);
		return;
	}

	/* get current placement */
	GetWindowPlacement(hWnd, (LPWINDOWPLACEMENT)&placement);

	/* parse command */
	for (int i = 0; placements[i].name != NULL; i++) {
		if (MMDAgent_strequal(param3, placements[i].name)) {
			placement.showCmd = placements[i].command;
			break;
		}
	}

	/* set window placement */
	placement.length = sizeof(WINDOWPLACEMENT);
	SetWindowPlacement(hWnd, &placement);

	if (placement.showCmd == SW_SHOW) {
		SetForegroundWindow(hWnd);
	}

#endif
}

/* extAppStart: initialize controller */
EXPORT void extAppStart(MMDAgent *mmdagent)
{
	enable = true;
	mmdagent->sendMessage(MMDAGENT_EVENT_PLUGINDISABLE, "%s", PLUGINWINDOWSTYLE_NAME);
}

/* extProcCommand: process command message */
EXPORT void extProcCommand(MMDAgent *mmdagent, const char *type, const char *args)
{
	if (enable == true) {
		if (MMDAgent_strequal(type, MMDAGENT_COMMAND_PLUGINDISABLE) == true) {
			if (MMDAgent_strequal(args, PLUGINWINDOWSTYLE_NAME)) {
				enable = false;
				mmdagent->sendMessage(MMDAGENT_EVENT_PLUGINDISABLE, "%s", PLUGINWINDOWSTYLE_NAME);
			}
		}
		else if (MMDAgent_strequal(type, PLUGINWINDOWSTYLE_STYLE) == true) {
			setWindowStyle(args);
		}
		else if (MMDAgent_strequal(type, PLUGINWINDOWSTYLE_POSITION) == true) {
			setWindowPosition(args);
		}
		else if (MMDAgent_strequal(type, PLUGINWINDOWSTYLE_PLACEMENT) == true) {
			setWindowPlacement(args);
		}
		else if (MMDAgent_strequal(type, PLUGINWINDOWSTYLE_TRANSPARENCY) == true) {
			setWindowTransparency(args);
		}
	}
	else {
		if (MMDAgent_strequal(type, MMDAGENT_COMMAND_PLUGINENABLE) == true) {
			if (MMDAgent_strequal(args, PLUGINWINDOWSTYLE_NAME) == true) {
				enable = true;
				mmdagent->sendMessage(MMDAGENT_EVENT_PLUGINDISABLE, "%s", PLUGINWINDOWSTYLE_NAME);
			}
		}
	}
}

/* extAppEnd: stop controller */
EXPORT void extAppEnd(MMDAgent *mmdagent)
{
	enable = false;
}
