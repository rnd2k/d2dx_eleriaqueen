/*
	This file is part of D2DX.

	Copyright (C) 2021  Bolrog

	D2DX is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	D2DX is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with D2DX.  If not, see <https://www.gnu.org/licenses/>.
*/
#include "pch.h"
#include "DllNotify.h"

using namespace d2dx;


DllNotify::DllNotify() 
{
}


VOID CALLBACK DllNotify::LdrDllNotification(
	_In_      ULONG NotificationReason,
	_In_      PCLDR_DLL_NOTIFICATION_DATA NotificationData,
	_In_opt_  PVOID Context
)
{
	if (NotificationReason == LDR_DLL_NOTIFICATION_REASON_LOADED)
	{
		// D2Expres.dll want load?
		if (lstrcmpiW(NotificationData->Loaded.BaseDllName->Buffer, L"d2expres.dll") == 0) {
			void* D2ExpresBase = NotificationData->Loaded.DllBase;

			// remove sven's glide3x.dll patch, to prevent crash
			PatchUInt32(D2ExpresBase, 0x20C9 + 0, 0x90903BEB);
			PatchUInt32(D2ExpresBase, 0x20C9 + 4, 0xEB909090);
		}
	}
}


static PVOID cookie;
static DllNotify::PLDR_REGISTER_DLL_NOTIFICATION    p_LdrRegisterDllNotification;
static DllNotify::PLDR_UNREGISTER_DLL_NOTIFICATION  p_LdrUnRegisterDllNotification;

BOOL DllNotify::Init_Dllnotify()
{
	NTSTATUS status = 1;
	HMODULE ntdll = GetModuleHandle(L"ntdll.dll");

	p_LdrRegisterDllNotification = (PLDR_REGISTER_DLL_NOTIFICATION)GetProcAddress(ntdll, "LdrRegisterDllNotification");
	p_LdrUnRegisterDllNotification = (PLDR_UNREGISTER_DLL_NOTIFICATION)GetProcAddress(ntdll, "LdrUnRegisterDllNotification");

	if (p_LdrRegisterDllNotification)
	{
		status = p_LdrRegisterDllNotification(
			0, // must be zero
			LdrDllNotification,
			0, // context,
			&cookie
		);
	}

	return status == 0;
}


BOOL DllNotify::Uninit_Dllnotify()
{
	NTSTATUS status = 1;

	if (p_LdrUnRegisterDllNotification)
	{
		status = p_LdrUnRegisterDllNotification(cookie);
		cookie = 0;
	}

	return status == 0;
}


void DllNotify::PatchUInt32(HANDLE hModule, uint32_t offset, uint32_t value)
{
	uint32_t* patchLocation = (uint32_t*)((uint32_t)hModule + offset);
	DWORD dwOldPage;
	VirtualProtect(patchLocation, 4, PAGE_EXECUTE_READWRITE, &dwOldPage);
	*patchLocation = value;
	VirtualProtect(patchLocation, 4, dwOldPage, &dwOldPage);
}


void DllNotify::PatchUInt8(HANDLE hModule, uint32_t offset, uint8_t value)
{
	uint8_t* patchLocation = (uint8_t*)((uint32_t)hModule + offset);
	DWORD dwOldPage;
	VirtualProtect(patchLocation, 1, PAGE_EXECUTE_READWRITE, &dwOldPage);
	*patchLocation = value;
	VirtualProtect(patchLocation, 1, dwOldPage, &dwOldPage);
}