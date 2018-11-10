/*
*
* WhatsappTray
* Copyright (C) 1998-2017  Sebastian Amann, Nikolay Redko, J.D. Purcell
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*
*/

#include <windows.h>
#include "WhatsappTray.h"
#include <iostream>
#include <fstream>
#include <sstream>

#undef MODULE_NAME
#define MODULE_NAME "[WhatsappTrayHook] "

static HHOOK _hWndProc = NULL;
static HHOOK _mouseProc = NULL;
static HHOOK _cbtProc = NULL;

// Only works for 32-bit apps or 64-bit apps depending on whether this is complied as 32-bit or 64-bit (Whatsapp is a 64Bit-App)
LRESULT CALLBACK CallWndRetProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode >= 0)
	{
		CWPSTRUCT *msg = (CWPSTRUCT*)lParam;

		wchar_t buffer[2000];
		//swprintf_s(buffer, sizeof(buffer), MODULE_NAME L"CallWndRetProc message:%X", msg->message);
		//OutputDebugString(buffer);

		if (msg->message == WM_SYSCOMMAND)
		{
			// Description for WM_SYSCOMMAND: https://msdn.microsoft.com/de-de/library/windows/desktop/ms646360(v=vs.85).aspx
			if (msg->wParam == SC_MINIMIZE)
			{
				OutputDebugString(MODULE_NAME L"SC_MINIMIZE");

				// Ich pr�fe hier noch ob der Fenstertitel �bereinstimmt. Vorher hatte ich das Problem das sich Chrome auch minimiert hat.
				// Ich k�nnte hier auch noch die klasse checken, das hat dann den vorteil, das es noch genauer ist.
				// Sollte die Klasse von Whatsapp aus aber umbenannt werden muss ich hier wieder nachbesser.
				// -> Daher lass ichs erstmal so...
				if (msg->hwnd == FindWindow(NULL, WHATSAPP_CLIENT_NAME))
				{
					PostMessage(FindWindow(NAME, NAME), WM_ADDTRAY, 0, (LPARAM)msg->hwnd);
				}
			}
		}
		else if (msg->message == WM_NCDESTROY)
		{
			uintptr_t handle1 = reinterpret_cast<uintptr_t>(msg->hwnd);
			uintptr_t handle2 = reinterpret_cast<uintptr_t>(FindWindow(NAME, NAME));
			swprintf_s(buffer, sizeof(buffer), MODULE_NAME L"WM_NCDESTROY hwnd:%llX findwindow:%llX", handle1, handle2);
			OutputDebugString(buffer);

			// Eigentlich sollte ich hier die gleichen probleme haben wie bei sc_minimize,
			// Ich glaub aber das des zu einer zeit war wo noch ein globaler hook gemacht wurde...
			// Deshalb denk ich es ist ok wenn ich hier den namen nicht pr�fe.
			// NOTE: Ich mach es deshalb nicht weil das Fenster zu den zeitpunkt nicht auffindbar ist, was warscheinlich daran liegt das es bereits geschlossen wurde.

			// Ich machs jetz so dass ich wenn ich das Whatsapp-Fenster nicht finde auch schlie�e, wegen den oben genannten gruenden.
			if (msg->hwnd == FindWindow(NULL, WHATSAPP_CLIENT_NAME) || FindWindow(NULL, WHATSAPP_CLIENT_NAME) == NULL)
			{
				bool successfulSent = PostMessage(FindWindow(NAME, NAME), WM_WHAHTSAPP_CLOSING, 0, (LPARAM)msg->hwnd);
				if (successfulSent) {
					OutputDebugString(MODULE_NAME L"WM_WHAHTSAPP_CLOSING successful sent.");
				}
			}
		}

	}
	
	return CallNextHookEx(_hWndProc, nCode, wParam, lParam);
}

LRESULT CALLBACK CallWndRetProcDebug(int nCode, WPARAM wParam, LPARAM lParam) {
	//if (nCode >= 0)
	{
		CWPSTRUCT *msg = (CWPSTRUCT*)lParam;

		//if (msg->hwnd == (HWND)0x00120A42)
		if (msg->hwnd == FindWindow(NULL, WHATSAPP_CLIENT_NAME))
		{
			//
			if (msg->message == 0x2 || msg->message == 528 || msg->message == 70 || msg->message == 71 || msg->message == 28 || msg->message == 134 || msg->message == 6 || msg->message == 641 || msg->message == 642 || msg->message == 7 || msg->message == 533 || msg->message == 144 || msg->message == 70 || msg->message == 71 || msg->message == 134 || msg->message == 6 || msg->message == 28 || msg->message == 8 || msg->message == 641 || msg->message == 642 || msg->message == 626 || msg->message == 2)
			{
				// WM_DESTROY

				//std::ostringstream filename2;
				//filename2 << "C:/hooktest/HWND_" << msg->hwnd << ".txt";

				//std::ofstream myfile;
				//myfile.open(filename2.str().c_str(), std::ios::app);

				////LONG wndproc = GetWindowLong(msg->hwnd, -4);

				//myfile << "\nblocked (" << msg->message << ")" << msg->wParam;



				//MSG msgr;
				//PeekMessage(&msgr, msg->hwnd, 0, 0, PM_REMOVE);

				return 0;
			}

			//if (msg->message == WM_SYSCOMMAND)
			{
				std::ostringstream filename2;
				filename2 << "C:/hooktest/HWND_" << msg->hwnd << ".txt";

				std::ofstream myfile;
				myfile.open(filename2.str().c_str(), std::ios::app);

				myfile << "\nWM_SYSCOMMAND (" << msg->message << ")" << msg->wParam;

				if (msg->wParam == 61472)
				{
					myfile << "\nMinimize";
					myfile << "\nHWND to Hookwindow:" << FindWindow(NAME, NAME);

					PostMessage(FindWindow(NAME, NAME), WM_ADDTRAY, 0, (LPARAM)msg->hwnd);
					//PostMessage(FindWindow(NAME, NAME), WM_REFRTRAY, 0, (LPARAM)msg->hwnd);
				}
				myfile.close();
			}

		}

	}

	return CallNextHookEx(_hWndProc, nCode, wParam, lParam);
}

LRESULT CALLBACK CBTProc(
	_In_ int    nCode,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam
)
{

	if (nCode == HCBT_DESTROYWND)
	{
		std::ostringstream filename2;
		filename2 << "C:/hooktest/HWND_" << (HWND)wParam << ".txt";

		std::ofstream myfile;
		myfile.open(filename2.str().c_str(), std::ios::app);

		myfile << "\nblocked (" ;

		
		return 1;
	}
	return CallNextHookEx(_cbtProc, nCode, wParam, lParam);
}

LRESULT CALLBACK MouseProc(
	_In_ int    nCode,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam
)
{
	if (nCode >= 0)
	{
		if (wParam == WM_LBUTTONDOWN)
		{
			//std::ostringstream filename2;
			//filename2 << "C:/hooktest/HWND_" << (HWND)wParam << ".txt";

			//std::ofstream myfile;
			//myfile.open(filename2.str().c_str(), std::ios::app);

			MOUSEHOOKSTRUCT *mhs = (MOUSEHOOKSTRUCT*)lParam;

			RECT rect;
			GetWindowRect(mhs->hwnd, &rect);

			// Modify rect to cover the X(close) button.
			rect.left = rect.right - 46;
			rect.bottom = rect.top + 35;

			bool mouseOnClosebutton = PtInRect(&rect, mhs->pt);

			if (mouseOnClosebutton)
			{
				//OutputDebugString(MODULE_NAME L"Closebutton mousedown");
				//myfile << "\nMinimize";
				//myfile << "\nHWND to Hookwindow:" << FindWindow(NAME, NAME);

				PostMessage(FindWindow(NAME, NAME), WM_ADDTRAY, 0, (LPARAM)mhs->hwnd);

				// Returning nozero blocks the message frome being sent to the application.
				return 1;
			}
		}
	}

	return CallNextHookEx(_cbtProc, nCode, wParam, lParam);
} 

// Wenn ich als threadId 0 �bergeben, ist es ein Globaler Hook.
BOOL DLLIMPORT RegisterHook(HMODULE hLib, DWORD threadId, bool closeToTray)
{
	_hWndProc = SetWindowsHookEx(WH_CALLWNDPROC, (HOOKPROC)CallWndRetProc, hLib, threadId);
	if (_hWndProc == NULL)
	{
		OutputDebugString(MODULE_NAME L"RegisterHook() - Error Creation Hook _hWndProc\n");
		UnRegisterHook();
		return FALSE;
	}

	if (closeToTray)
	{
		_mouseProc = SetWindowsHookEx(WH_MOUSE, (HOOKPROC)MouseProc, hLib, threadId);
		if (_mouseProc == NULL)
		{
			OutputDebugString(MODULE_NAME L"RegisterHook() - Error Creation Hook _hWndProc\n");
			UnRegisterHook();
			return FALSE;
		}
	}
	
	//_cbtProc = SetWindowsHookEx(WH_CBT, (HOOKPROC)CBTProc, hLib, threadId);
	
	return TRUE;
}

void DLLIMPORT UnRegisterHook()
{
	if (_hWndProc)
	{
		UnhookWindowsHookEx(_hWndProc);
		_hWndProc = NULL;
	}
	if (_mouseProc)
	{
		UnhookWindowsHookEx(_mouseProc);
		_mouseProc = NULL;
	}
}

