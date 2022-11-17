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
#pragma once

#include <windows.h>
#include <NTSecAPI.h>

#define LDR_DLL_NOTIFICATION_REASON_LOADED 1
#define LDR_DLL_NOTIFICATION_REASON_UNLOADED 2

namespace d2dx 
{
	class DllNotify
	{
	public:
		DllNotify();
		virtual ~DllNotify() noexcept {}
		
		typedef const UNICODE_STRING* PCUNICODE_STRING;

		typedef struct _LDR_DLL_LOADED_NOTIFICATION_DATA {
			ULONG Flags;                    //Reserved.
			PCUNICODE_STRING FullDllName;   //The full path name of the DLL module.
			PCUNICODE_STRING BaseDllName;   //The base file name of the DLL module.
			PVOID DllBase;                  //A pointer to the base address for the DLL in memory.
			ULONG SizeOfImage;              //The size of the DLL image, in bytes.
		} LDR_DLL_LOADED_NOTIFICATION_DATA, * PLDR_DLL_LOADED_NOTIFICATION_DATA;

		typedef struct _LDR_DLL_UNLOADED_NOTIFICATION_DATA {
			ULONG Flags;                    //Reserved.
			PCUNICODE_STRING FullDllName;   //The full path name of the DLL module.
			PCUNICODE_STRING BaseDllName;   //The base file name of the DLL module.
			PVOID DllBase;                  //A pointer to the base address for the DLL in memory.
			ULONG SizeOfImage;              //The size of the DLL image, in bytes.
		} LDR_DLL_UNLOADED_NOTIFICATION_DATA, * PLDR_DLL_UNLOADED_NOTIFICATION_DATA;

		typedef union _LDR_DLL_NOTIFICATION_DATA {
			LDR_DLL_LOADED_NOTIFICATION_DATA Loaded;
			LDR_DLL_UNLOADED_NOTIFICATION_DATA Unloaded;
		} LDR_DLL_NOTIFICATION_DATA, * PLDR_DLL_NOTIFICATION_DATA;

		typedef const _LDR_DLL_NOTIFICATION_DATA* PCLDR_DLL_NOTIFICATION_DATA;

		typedef VOID(CALLBACK* PLDR_DLL_NOTIFICATION_FUNCTION)(
			_In_      ULONG NotificationReason,
			_In_      PCLDR_DLL_NOTIFICATION_DATA NotificationData,
			_In_opt_  PVOID Context
			);

		typedef NTSTATUS(NTAPI* PLDR_REGISTER_DLL_NOTIFICATION)(
			_In_      ULONG Flags,
			_In_      PLDR_DLL_NOTIFICATION_FUNCTION NotificationFunction,
			_In_opt_  PVOID Context,
			_Out_     PVOID* Cookie
			);


		typedef NTSTATUS(NTAPI* PLDR_UNREGISTER_DLL_NOTIFICATION)(
			_In_  PVOID Cookie
			);

		static BOOL Init_Dllnotify();
		static BOOL Uninit_Dllnotify();

	private:
		static VOID CALLBACK LdrDllNotification(
			_In_      ULONG NotificationReason,
			_In_      PCLDR_DLL_NOTIFICATION_DATA NotificationData,
			_In_opt_  PVOID Context
		);

		static void PatchUInt32(HANDLE hModule, uint32_t offset, uint32_t value);
		static void PatchUInt8(HANDLE hModule, uint32_t offset, uint8_t value);
	};
}
