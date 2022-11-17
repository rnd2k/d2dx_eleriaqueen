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
#include "GameHelper.h"
#include "Buffer.h"
#include "Utils.h"

using namespace d2dx;

GameHelper::GameHelper() :
	_version(GetGameVersion()),
	_hProcess(GetCurrentProcess()),
	_hGameExe(GetModuleHandleA("game.exe")),
	_hD2ClientDll(LoadLibraryA("D2Client.dll")),
	_hD2CommonDll(LoadLibraryA("D2Common.dll")),
	_hD2GfxDll(LoadLibraryA("D2Gfx.dll")),
	_hD2WinDll(LoadLibraryA("D2Win.dll")),
	_isProjectDiablo2(GetModuleHandleA("PD2_EXT.dll") != nullptr)
{
	InitializeTextureHashPrefixTable();

	if (_isProjectDiablo2)
	{
		D2DX_LOG("Detected Project Diablo 2.");
	}
}

const HMODULE GameHelper::GetModule(LPCWSTR szModule) const
{
	HMODULE hModule = 0;
	wchar_t wcCurrentDir[MAX_PATH] = { 0 };
	wchar_t buffer[MAX_PATH] = { 0 };

	GetCurrentDirectoryW(MAX_PATH, wcCurrentDir);
	swprintf(buffer, L"%s\\%s", wcCurrentDir, szModule);

	hModule = GetModuleHandleW(szModule);
	if (hModule == 0) {
		hModule = LoadLibraryW(buffer);
	}
	return hModule;
}

GameVersion GameHelper::GetVersion() const
{
	return _version;
}

_Use_decl_annotations_
const char* GameHelper::GetVersionString() const
{
	switch (_version)
	{
	case GameVersion::Lod109d:
		return "Lod109d";
	case GameVersion::Lod110f:
		return "Lod110";
	case GameVersion::Lod112:
		return "Lod112";
	case GameVersion::Lod113c:
		return "Lod113c";
	case GameVersion::Lod113d:
		return "Lod113d";
	case GameVersion::Lod114d:
		return "Lod114d";
	default:
		return "Unhandled";
	}
}

uint32_t GameHelper::ScreenOpenMode() const
{
	switch (_version)
	{
	case GameVersion::Lod109d:
		return *(const uint32_t*)((uint32_t)_hD2ClientDll + 0x115C10);
	case GameVersion::Lod110f:
		return *(const uint32_t*)((uint32_t)_hD2ClientDll + 0x10B9C4);
	case GameVersion::Lod112:
		return *(const uint32_t*)((uint32_t)_hD2ClientDll + 0x11C1D0);
	case GameVersion::Lod113c:
		return *(const uint32_t*)((uint32_t)_hD2ClientDll + 0x11C414);
	case GameVersion::Lod113d:
		return *(const uint32_t*)((uint32_t)_hD2ClientDll + 0x11D070);
	case GameVersion::Lod114d:
		return *(const uint32_t*)((uint32_t)_hGameExe + 0x3A5210);
	default:
		return 0;
	}
}

Size GameHelper::GetConfiguredGameSize() const
{
	HKEY hKey;
	LPCTSTR diablo2Key = TEXT("SOFTWARE\\Blizzard Entertainment\\Diablo II");
	LONG openRes = RegOpenKeyEx(HKEY_CURRENT_USER, diablo2Key, 0, KEY_READ, &hKey);
	if (openRes != ERROR_SUCCESS)
	{
		return { 800, 600 };
	}

	DWORD type = REG_DWORD;
	DWORD size = 4;
	DWORD value = 0;
	auto queryRes = RegQueryValueExA(hKey, "Resolution", NULL, &type, (LPBYTE)&value, &size);
	assert(queryRes == ERROR_SUCCESS || queryRes == ERROR_MORE_DATA);

	RegCloseKey(hKey);

	if (value == 0)
	{
		return { 640, 480 };
	}
	else
	{
		return { 800, 600 };
	}
}

static const uint32_t gameAddresses_109d[] =
{
	0xFFFFFFFF,
	0x6f818468, /* DrawWall1 */
	0x6f818476, /* DrawWall2 */
	0x6f813e2f, /* DrawFloor */
	0, /* DrawShadow */
	0x6f815efb,  /* DrawDynamic */
	0, /* DrawSomething1 */
	0, /* DrawSomething2 */
};

static const uint32_t gameAddresses_110[] =
{
	0xFFFFFFFF,
	0x6f81840c, /* DrawWall1 */
	0x6f81841a, /* DrawWall2 */
	0x6f813e24, /* DrawFloor */
	0, /* DrawShadow */
	0x6f815ec9,  /* DrawDynamic */
	0, /* DrawSomething1 */
	0, /* DrawSomething2 */
};

static const uint32_t gameAddresses_112[] =
{
	0xFFFFFFFF,
	0x6f85a2f9, /* DrawWall1 */
	0x6f85a2eb, /* DrawWall2 */
	0x6f856d6c, /* DrawFloor */
	0, /* DrawShadow */
	0x6f8587a4,  /* DrawDynamic */
	0, /* DrawSomething1 */
	0, /* DrawSomething2 */
};

static const uint32_t gameAddresses_113c[] =
{
	0xFFFFFFFF,
	0x6f8567ab, /* DrawWall1 */
	0x6f8567b9, /* DrawWall2 */
	0x6f85befc, /* DrawFloor */
	0x50a995, /* DrawShadow */
	0x6f85a344,  /* DrawDynamic */
	0x0050c38d, /* DrawSomething1 */
	0x0050c0de, /* DrawSomething2 */
};

static const uint32_t gameAddresses_113d[] =
{
	0xFFFFFFFF,
	0x6f857199, /* DrawWall1 */
	0x6f85718b, /* DrawWall2 */
	0x6f85c17c, /* DrawFloor */
	0x6f859ef5, /* DrawShadow */
	0x6f859ce4,  /* DrawDynamic */
	0x0050c38d, /* DrawSomething1 */
	0x0050c0de, /* DrawSomething2 */
};

static const uint32_t gameAddresses_114d[] =
{
	0xFFFFFFFF,
	0x50d39f, /* DrawWall1 */
	0x50d3ae, /* DrawWall2 */
	0x50db03, /* DrawFloor */
	0x50a995, /* DrawShadow */
	0x50abdc,  /* DrawDynamic */
	0x50c38d, /* DrawSomething1 */
	0x50c0de, /* DrawSomething2 */
};

_Use_decl_annotations_
GameAddress GameHelper::IdentifyGameAddress(
	uint32_t returnAddress) const
{
	const uint32_t* gameAddresses = nullptr;
	uint32_t gameAddressCount = 0;

	switch (_version)
	{
	case GameVersion::Lod109d:
		gameAddresses = gameAddresses_109d;
		gameAddressCount = ARRAYSIZE(gameAddresses_109d);
		break;
	case GameVersion::Lod110f:
		gameAddresses = gameAddresses_110;
		gameAddressCount = ARRAYSIZE(gameAddresses_110);
		break;
	case GameVersion::Lod112:
		gameAddresses = gameAddresses_112;
		gameAddressCount = ARRAYSIZE(gameAddresses_112);
		break;
	case GameVersion::Lod113c:
		gameAddresses = gameAddresses_113c;
		gameAddressCount = ARRAYSIZE(gameAddresses_113c);
		break;
	case GameVersion::Lod113d:
		gameAddresses = gameAddresses_113d;
		gameAddressCount = ARRAYSIZE(gameAddresses_113d);
		break;
	case GameVersion::Lod114d:
		gameAddresses = gameAddresses_114d;
		gameAddressCount = ARRAYSIZE(gameAddresses_114d);
		break;
	}

	for (uint32_t i = 0; i < gameAddressCount; ++i)
	{
		if (gameAddresses[i] == returnAddress)
		{
			return (GameAddress)i;
		}
	}

	return GameAddress::Unknown;
}

static const uint32_t titleScreenHashes[] = {
	0x0836bff0,	0x0d609152,	0x1df19dd6,	0x2c779942,	0x3a174cb2,	0x3d35f3c5,	0x3d4c8c14,	0x605f521f,	0x6b69636d,
	0x73059f7c,	0x8766b77a,	0x8af2178a,	0x90bdd994,	0x94e77d2d,	0xa66ac09c,	0xbe1a20c3,	0xc158e602,	0xc2625261,
	0xccf7cc94,	0xcee4c170,	0xd38a63df,	0xd4579523,	0xda6e064e,	0xe22a8bc4,	0xe2e6b0c7,	0xe9263199, 0xe1e211f9,
	0x2ac72136, 0x2f15f9de, 0x2dba4381, 0x5bbe76ab, 0x5fa60772, 0x7bb42e90, 0x08b64561, 0x8a09de96, 0x8b255624,
	0x8bceb8c5, 0x8be41271, 0x8c93dc24, 0x38ac989c, 0x42f99404, 0x49d1f478, 0x49f4099d, 0x57ff0b65, 0x87c0b98d,
	0x89aaf047, 0x128bd717, 0x169c4b8e, 0x234cae2e, 0x264fc41c, 0x282fe954, 0x467c7521, 0x614d3948, 0x874fb06e,
	0x968db1ce, 0x969ed6e4, 0x3034acfa, 0x18793782, 0x32561192, 0x23206047, 0xa6a88d0e, 0xa8b86316, 0xa8ba2a4f,
	0xa13c32b5, 0xafaa7b74, 0xb1450cb1, 0xbc7f5ddb, 0xbcf633e8, 0xbe9c50d5, 0xbec03b1c, 0xc0821e4c, 0xc5638e07,
	0xcae3f8e8, 0xd113d34d, 0xd4032a7f, 0xd5206c21, 0xd1149259, 0xe15c8e53, 0xe9174a70, 0xedf6f578, 0xf13dd4fb,
	0xf045cd36, 0xf169106c, 0xc9d4e158
};

static const uint32_t loadingScreenHashes[] = {
	0x0aa1834d, 0x1a7964a9, 0x2f5b86a7, 0x70a8cb14, 0x32965ce1, 0x897794ce, 0x3136b0ee, 0x32965ce1, 0xc2cc7e28,
	0x2a683b29, 0x01c37ff8
};

static const uint32_t mousePointerHashes[] = {
	0xfe34f8b7,	0x5cac0e94,	0x4b661cd1,	3432412206,	2611936918,	2932294163,	1166565234,	77145516, 1264983249,
	4264884407
};

static const uint32_t uiHashes[] = {
	0x2ff1fd61, 0x54cc8b72,	0xfc253c88, 0xabe12614, 0xa22f5459, 0xa0d8fb2a, 0x20526487, 0x8a3b7d58, 0x2ff1fd61,
	0x54cc8b72, 0x76aa9aac, 0xef8d8978, 0x45e0af79, 0x9a008b35, 0x2a53bd89, 0x13d2c082, 0xab6ab811, 0xee7d31ba,
	0x6d1e37cf, 0xa4e86125, 0xa769824b, 0xb4119f58, 0xc2da4379, 0xdfbf045f, 0x88021112, 0x726eeaa0, 0x49e4e24e,
	0x3b50f3b6, 0x1e623206, 0xae502740, 0xd16d7f9a, 0xf6ec6116, 0x56acd7e4, 0x7656c190, 0xb0d15023, 0xb2c6e5fb,
	0x27d5991a, 0x21d8d615, 0x2bbf74be, 0x9ab19e53, 0x9ba9eeb2, 0x109348c9, 0x0f37086a, 0x10ac28d0, 0x5c121175,
	0x5c4d1125, 0xa1990293, 0xae25bff7, 0xb5855728, 0xc8f9d3f1, 0x2172d939, 0x0bd8d550, 0x62cfb0b8, 0x93e92b00,
	0x815a6925, 0x135190af, 0x3408446d, 0xaa265b2e, 0x316149fe, 0x63556155, 0xa9ba1eb0, 0xa9e34142, 0xa0564010,
	0xb0a058c2, 0xb037844a, 0xbbfee318, 0xc95d3136, 0xceadb1cd, 0xcef62ab8, 0xcfd7f4dd, 0xd8a1f81b, 0xd8df8f4b,
	0xd9dc1bdd, 0xdfe365f3, 0xee4f10d9, 0x4c389b09, 0x4c049e57, 0x4c8bda35, 0x4d234ffb, 0x4b2e9d5b, 0x1ffb1615,
	0x0a90d031, 0x5ed2fc41, 0x6b7e62ef, 0x6d05de67, 0x7acfc435, 0x7a742b36, 0x8d3366ec, 0x9ab19e5e, 0x933ba45c,
	0x977c13be, 0x7820ea79, 0x9643d531, 0x7111312a, 0x25534537, 0xc723c18e, 0xfa170b3f, 0x97c7e7f4, 0x8ce7ef63,
	0x45c78147, 0x5ca62551, 0xf8d429fb, 0xfee40e62,
};

struct Hashes
{
	int32_t count;
	const uint32_t* hashes;
};

static const Hashes hashesPerCategory[(int)TextureCategory::Count] =
{
	{ 0, nullptr },
	{ ARRAYSIZE(mousePointerHashes), mousePointerHashes },
	{ 0, nullptr },
	{ ARRAYSIZE(loadingScreenHashes), loadingScreenHashes },
	{ /* Floor: don't bother keeping a list of hashes */ 0, nullptr },
	{ ARRAYSIZE(titleScreenHashes), titleScreenHashes },
	{ /* Wall: don't bother keeping a list of hashes */ 0, nullptr },
	{ ARRAYSIZE(uiHashes), uiHashes },
};

static bool isInitialized = false;
static Buffer<uint32_t> prefixTable[256];
static Buffer<uint32_t> prefixCounts(256, true);

void GameHelper::InitializeTextureHashPrefixTable()
{
	for (int32_t category = 0; category < ARRAYSIZE(hashesPerCategory); ++category)
	{
		const Hashes& hashes = hashesPerCategory[category];

		for (int32_t hashIndex = 0; hashIndex < hashes.count; ++hashIndex)
		{
			++prefixCounts.items[(hashes.hashes[hashIndex] >> 24) & 0xFF];
		}
	}

	for (int32_t prefix = 0; prefix < 256; ++prefix)
	{
		const uint32_t count = prefixCounts.items[prefix];

		if (count > 0)
		{
			prefixTable[prefix] = Buffer<uint32_t>(count, true);
		}
		else
		{
			prefixTable[prefix] = Buffer<uint32_t>();
		}
	}

	for (int32_t category = 0; category < ARRAYSIZE(hashesPerCategory); ++category)
	{
		const Hashes& hashes = hashesPerCategory[category];
		for (int32_t j = 0; j < hashes.count; ++j)
		{
			const uint32_t hash = hashes.hashes[j];
			const uint32_t prefix = hash >> 24;
			const uint32_t position = --prefixCounts.items[prefix];
			prefixTable[prefix].items[position] = (((uint32_t)category & 0xFF) << 24) | (hash & 0x00FFFFFF);
		}
	}
}

_Use_decl_annotations_
TextureCategory GameHelper::GetTextureCategoryFromHash(
	uint32_t textureHash) const
{
	Buffer<uint32_t>& table = prefixTable[textureHash >> 24];

	if (table.capacity == 0)
	{
		return TextureCategory::Unknown;
	}

	for (int32_t i = 0; i < (int32_t)table.capacity; ++i)
	{
		uint32_t entry = table.items[i];

		if ((entry & 0x00FFFFFF) == (textureHash & 0x00FFFFFF))
			return (TextureCategory)(entry >> 24);
	}

	return TextureCategory::Unknown;
}

_Use_decl_annotations_
TextureCategory GameHelper::RefineTextureCategoryFromGameAddress(
	TextureCategory previousCategory,
	GameAddress gameAddress) const
{
	if (previousCategory != TextureCategory::Unknown)
	{
		return previousCategory;
	}

	switch (gameAddress)
	{
	case GameAddress::DrawFloor:
		return TextureCategory::Floor;
		break;
	case GameAddress::DrawWall1:
	case GameAddress::DrawWall2:
		return TextureCategory::Wall;
		break;
	default:
		return previousCategory;
	}
}

//GameVersion GameHelper::GetGameVersion()
//{
//	GameVersion version = GameVersion::Unsupported;
//
//	auto versionSize = GetFileVersionInfoSizeA("game.exe", nullptr);
//	Buffer<uint8_t> verData(versionSize);
//
//	if (!GetFileVersionInfoA("game.exe", NULL, verData.capacity, verData.items))
//	{
//		D2DX_LOG("Failed to get file version for game.exe.");
//		return GameVersion::Unsupported;
//	}
//
//	uint32_t size = 0;
//	const uint8_t* lpBuffer = nullptr;
//	bool success = VerQueryValueA(verData.items, "\\", (VOID FAR * FAR*) & lpBuffer, &size);
//
//	if (!(success && size > 0))
//	{
//		D2DX_LOG("Failed to query version info for game.exe.");
//		return GameVersion::Unsupported;
//	}
//
//	VS_FIXEDFILEINFO* vsFixedFileInfo = (VS_FIXEDFILEINFO*)lpBuffer;
//	if (vsFixedFileInfo->dwSignature != 0xfeef04bd)
//	{
//		D2DX_LOG("Unexpected signature in version info for game.exe.");
//		return GameVersion::Unsupported;
//	}
//
//	const int32_t a = vsFixedFileInfo->dwFileVersionMS >> 16;
//	const int32_t b = vsFixedFileInfo->dwFileVersionMS & 0xffff;
//	const int32_t c = vsFixedFileInfo->dwFileVersionLS >> 16;
//	const int32_t d = vsFixedFileInfo->dwFileVersionLS & 0xffff;
//
//	if (a == 1 && b == 0 && c == 9 && d == 22)
//	{
//		version = GameVersion::Lod109d;
//	}
//	else if (a == 1 && b == 0 && c == 10 && d == 39)
//	{
//		version = GameVersion::Lod110f;
//	}
//	else if (a == 1 && b == 0 && c == 12 && d == 49)
//	{
//		version = GameVersion::Lod112;
//	}
//	else if (a == 1 && b == 0 && c == 13 && d == 60)
//	{
//		version = GameVersion::Lod113c;
//	}
//	else if (a == 1 && b == 0 && c == 13 && d == 64)
//	{
//		version = GameVersion::Lod113d;
//	}
//	else if (a == 1 && b == 14 && c == 3 && d == 68)
//	{
//		D2DX_FATAL_ERROR("This version (1.14b) of Diablo II will not work with D2DX. Please upgrade to version 1.14d.");
//	}
//	else if (a == 1 && b == 14 && c == 3 && d == 71)
//	{
//		version = GameVersion::Lod114d;
//	}
//
//	if (version == GameVersion::Unsupported)
//	{
//		MessageBoxA(NULL, "This version of Diablo II is not supported by D2DX. Please upgrade or downgrade to a supported version.", "D2DX", MB_OK);
//	}
//
//	D2DX_LOG("Game version: %d.%d.%d.%d (%s)\n", a, b, c, d, version == GameVersion::Unsupported ? "unsupported" : "supported");
//
//	return version;
//}


IMAGE_NT_HEADERS* GameHelper::GetHeader(LPBYTE pBase)
{
	if (pBase == NULL)
		return NULL;

	IMAGE_DOS_HEADER* pDosHeader = (IMAGE_DOS_HEADER*)pBase;

	if (IsBadReadPtr(pDosHeader, sizeof(IMAGE_DOS_HEADER)))
		return NULL;

	if (pDosHeader->e_magic != IMAGE_DOS_SIGNATURE)
		return NULL;

	IMAGE_NT_HEADERS* pHeader = (IMAGE_NT_HEADERS*)(pBase + pDosHeader->e_lfanew);
	if (IsBadReadPtr(pHeader, sizeof(IMAGE_NT_HEADERS)))
		return NULL;

	if (pHeader->Signature != IMAGE_NT_SIGNATURE)
		return NULL;

	return pHeader;
}

GameVersion GameHelper::GetGameVersion() {
	GameVersion version = GameVersion::Unsupported;

	DWORD offset_Game = 0;
	HMODULE offset_D2Glide = 0;
	HMODULE offset_D2Client = 0;
	HMODULE offset_D2CMP = 0;
	HMODULE offset_D2Common = 0;
	HMODULE offset_D2Game = 0;
	HMODULE offset_D2gfx = 0;
	HMODULE offset_D2Lang = 0;
	HMODULE offset_D2Launch = 0;
	HMODULE offset_D2Net = 0;
	HMODULE offset_D2Win = 0;
	HMODULE offset_Fog = 0;
	HMODULE offset_Storm = 0;

	offset_Game = (DWORD)GetModuleHandle(NULL);
	offset_D2Glide = GameHelper::GetModule(L"D2Glide.dll");
	offset_D2Client = GameHelper::GetModule(L"D2Client.dll");
	offset_D2CMP = GameHelper::GetModule(L"D2CMP.dll");
	offset_D2Common = GameHelper::GetModule(L"D2Common.dll");
	offset_D2Game = GameHelper::GetModule(L"D2Game.dll");
	offset_D2gfx = GameHelper::GetModule(L"D2gfx.dll");
	offset_D2Lang = GameHelper::GetModule(L"D2Lang.dll");
	offset_D2Launch = GameHelper::GetModule(L"D2Launch.dll");
	offset_D2Net = GameHelper::GetModule(L"D2Net.dll");
	offset_D2Win = GameHelper::GetModule(L"D2Win.dll");
	offset_Fog = GameHelper::GetModule(L"Fog.dll");
	offset_Storm = GameHelper::GetModule(L"Storm.dll");


	int count_109b = 0;
	int count_109d = 0;
	int count_110f = 0;
	int count_111 = 0;
	int count_111b = 0;
	int count_112a = 0;
	int count_113c = 0;
	int count_113d = 0;
	int count_114a = 0;
	int count_114b = 0;
	int count_114c = 0;
	int count_114d = 0;


	IMAGE_NT_HEADERS* pHeader;
	char buffer[MAX_PATH] = { 0 };

	if (offset_D2Client != NULL) {
		pHeader = GetHeader((LPBYTE)offset_D2Client);
		if (pHeader->OptionalHeader.AddressOfEntryPoint == 0x000C234D) count_109b++;
		if (pHeader->OptionalHeader.AddressOfEntryPoint == 0x000C16CD) count_109d++;
		if (pHeader->OptionalHeader.AddressOfEntryPoint == 0x000C1C1D) count_110f++;
		if (pHeader->OptionalHeader.AddressOfEntryPoint == 0x000045E6) count_111++;
		if (pHeader->OptionalHeader.AddressOfEntryPoint == 0x000045EE) count_111b++;
		if (pHeader->OptionalHeader.AddressOfEntryPoint == 0x000045FA) count_112a++;
		if (pHeader->OptionalHeader.AddressOfEntryPoint == 0x000045F6) count_113c++;
		if (pHeader->OptionalHeader.AddressOfEntryPoint == 0x000045DE) count_113d++;
		GetModuleFileNameA(offset_D2Client, buffer, MAX_PATH);
		D2DX_LOG("D2Client.dll\tbase address %08X, loaded at %08X path %s%s", pHeader->OptionalHeader.ImageBase, (unsigned int)offset_D2Client, buffer,
			(pHeader->OptionalHeader.ImageBase == (unsigned int)offset_D2Client) ? "" : " (relocated)");
	}

	if (offset_D2CMP != NULL) {
		pHeader = GetHeader((LPBYTE)offset_D2CMP);
		if (pHeader->OptionalHeader.AddressOfEntryPoint == 0x00011361) count_109b++;
		if (pHeader->OptionalHeader.AddressOfEntryPoint == 0x00011361) count_109d++;
		if (pHeader->OptionalHeader.AddressOfEntryPoint == 0x00010E61) count_110f++;
		if (pHeader->OptionalHeader.AddressOfEntryPoint == 0x00002C23) count_111++;
		if (pHeader->OptionalHeader.AddressOfEntryPoint == 0x00002C23) count_111b++;
		if (pHeader->OptionalHeader.AddressOfEntryPoint == 0x00002C23) count_112a++;
		if (pHeader->OptionalHeader.AddressOfEntryPoint == 0x00002C23) count_113c++;
		if (pHeader->OptionalHeader.AddressOfEntryPoint == 0x00002C23) count_113d++;
		GetModuleFileNameA(offset_D2CMP, buffer, MAX_PATH);
		D2DX_LOG("D2Cmp.dll\t\tbase address %08X, loaded at %08X path %s%s", pHeader->OptionalHeader.ImageBase, (unsigned int)offset_D2CMP, buffer,
			(pHeader->OptionalHeader.ImageBase == (unsigned int)offset_D2CMP) ? "" : " (relocated)");
	}

	if (offset_D2Common != NULL) {
		pHeader = GetHeader((LPBYTE)offset_D2Common);
		if (pHeader->OptionalHeader.AddressOfEntryPoint == 0x00074D1D) count_109b++;
		if (pHeader->OptionalHeader.AddressOfEntryPoint == 0x00074E2D) count_109d++;
		if (pHeader->OptionalHeader.AddressOfEntryPoint == 0x000856DD) count_110f++;
		if (pHeader->OptionalHeader.AddressOfEntryPoint == 0x00002C94) count_111++;
		if (pHeader->OptionalHeader.AddressOfEntryPoint == 0x00002C8D) count_111b++;
		if (pHeader->OptionalHeader.AddressOfEntryPoint == 0x00002C97) count_112a++;
		if (pHeader->OptionalHeader.AddressOfEntryPoint == 0x00002C8F) count_113c++;
		if (pHeader->OptionalHeader.AddressOfEntryPoint == 0x000047C7) count_113d++;
		GetModuleFileNameA(offset_D2Common, buffer, MAX_PATH);
		D2DX_LOG("D2Common.dll\tbase address %08X, loaded at %08X path %s%s", pHeader->OptionalHeader.ImageBase, (unsigned int)offset_D2Common, buffer,
			(pHeader->OptionalHeader.ImageBase == (unsigned int)offset_D2Common) ? "" : " (relocated)");
	}

	if (offset_D2Game != NULL) {
		pHeader = GetHeader((LPBYTE)offset_D2Game);
		if (pHeader->OptionalHeader.AddressOfEntryPoint == 0x000C66AC) count_109b++;
		if (pHeader->OptionalHeader.AddressOfEntryPoint == 0x000C6D5C) count_109d++;
		if (pHeader->OptionalHeader.AddressOfEntryPoint == 0x000EDC2C) count_110f++;
		if (pHeader->OptionalHeader.AddressOfEntryPoint == 0x000036E6) count_111++;
		if (pHeader->OptionalHeader.AddressOfEntryPoint == 0x0000373D) count_111b++;
		if (pHeader->OptionalHeader.AddressOfEntryPoint == 0x0000374B) count_112a++;
		if (pHeader->OptionalHeader.AddressOfEntryPoint == 0x0000373C) count_113c++;
		if (pHeader->OptionalHeader.AddressOfEntryPoint == 0x00003747) count_113d++;
		GetModuleFileNameA(offset_D2Game, buffer, MAX_PATH);
		D2DX_LOG("D2Game.dll\t\tbase address %08X, loaded at %08X path %s%s", pHeader->OptionalHeader.ImageBase, (unsigned int)offset_D2Game, buffer,
			(pHeader->OptionalHeader.ImageBase == (unsigned int)offset_D2Game) ? "" : " (relocated)");
	}

	if (offset_D2gfx != NULL) {
		pHeader = GetHeader((LPBYTE)offset_D2gfx);
		if (pHeader->OptionalHeader.AddressOfEntryPoint == 0x000054EB) count_109b++;
		if (pHeader->OptionalHeader.AddressOfEntryPoint == 0x000054EB) count_109d++;
		if (pHeader->OptionalHeader.AddressOfEntryPoint == 0x000054A5) count_110f++;
		if (pHeader->OptionalHeader.AddressOfEntryPoint == 0x00001807) count_111++;
		if (pHeader->OptionalHeader.AddressOfEntryPoint == 0x00001807) count_111b++;
		if (pHeader->OptionalHeader.AddressOfEntryPoint == 0x00001807) count_112a++;
		if (pHeader->OptionalHeader.AddressOfEntryPoint == 0x00001807) count_113c++;
		if (pHeader->OptionalHeader.AddressOfEntryPoint == 0x00001807) count_113d++;
		GetModuleFileNameA(offset_D2gfx, buffer, MAX_PATH);
		D2DX_LOG("D2Gfx.dll\t\tbase address %08X, loaded at %08X path %s%s", pHeader->OptionalHeader.ImageBase, (unsigned int)offset_D2gfx, buffer,
			(pHeader->OptionalHeader.ImageBase == (unsigned int)offset_D2gfx) ? "" : " (relocated)");
	}

	if (offset_D2Lang != NULL) {
		pHeader = GetHeader((LPBYTE)offset_D2Lang);
		if (pHeader->OptionalHeader.AddressOfEntryPoint == 0x00005148) count_109b++;
		if (pHeader->OptionalHeader.AddressOfEntryPoint == 0x00005138) count_109d++;
		if (pHeader->OptionalHeader.AddressOfEntryPoint == 0x00005048) count_110f++;
		if (pHeader->OptionalHeader.AddressOfEntryPoint == 0x00001A6A) count_111++;
		if (pHeader->OptionalHeader.AddressOfEntryPoint == 0x00001A5B) count_111b++;
		if (pHeader->OptionalHeader.AddressOfEntryPoint == 0x00001A75) count_112a++;
		if (pHeader->OptionalHeader.AddressOfEntryPoint == 0x00001A71) count_113c++;
		if (pHeader->OptionalHeader.AddressOfEntryPoint == 0x00001A5A) count_113d++;
		GetModuleFileNameA(offset_D2Lang, buffer, MAX_PATH);
		D2DX_LOG("D2Lang.dll\t\tbase address %08X, loaded at %08X path %s%s", pHeader->OptionalHeader.ImageBase, (unsigned int)offset_D2Lang, buffer,
			(pHeader->OptionalHeader.ImageBase == (unsigned int)offset_D2Lang) ? "" : " (relocated)");
	}

	if (offset_D2Launch != NULL) {
		pHeader = GetHeader((LPBYTE)offset_D2Launch);
		if (pHeader->OptionalHeader.AddressOfEntryPoint == 0x000172C3) count_109b++;
		if (pHeader->OptionalHeader.AddressOfEntryPoint == 0x00017243) count_109d++;
		if (pHeader->OptionalHeader.AddressOfEntryPoint == 0x00018DC7) count_110f++;
		if (pHeader->OptionalHeader.AddressOfEntryPoint == 0x00001A84) count_111++;
		if (pHeader->OptionalHeader.AddressOfEntryPoint == 0x00001A85) count_111b++;
		if (pHeader->OptionalHeader.AddressOfEntryPoint == 0x00001A85) count_112a++;
		if (pHeader->OptionalHeader.AddressOfEntryPoint == 0x00001A87) count_113c++;
		if (pHeader->OptionalHeader.AddressOfEntryPoint == 0x00001A84) count_113d++;
		GetModuleFileNameA(offset_D2Launch, buffer, MAX_PATH);
		D2DX_LOG("D2Launch.dll\tbase address %08X, loaded at %08X path %s%s", pHeader->OptionalHeader.ImageBase, (unsigned int)offset_D2Launch, buffer,
			(pHeader->OptionalHeader.ImageBase == (unsigned int)offset_D2Launch) ? "" : " (relocated)");
	}

	if (offset_D2Net != NULL) {
		pHeader = GetHeader((LPBYTE)offset_D2Net);
		if (pHeader->OptionalHeader.AddressOfEntryPoint == 0x00002BCE) count_109b++;
		if (pHeader->OptionalHeader.AddressOfEntryPoint == 0x00002BCE) count_109d++;
		if (pHeader->OptionalHeader.AddressOfEntryPoint == 0x00002C6E) count_110f++;
		if (pHeader->OptionalHeader.AddressOfEntryPoint == 0x00001676) count_111++;
		if (pHeader->OptionalHeader.AddressOfEntryPoint == 0x00001676) count_111b++;
		if (pHeader->OptionalHeader.AddressOfEntryPoint == 0x0000167E) count_112a++;
		if (pHeader->OptionalHeader.AddressOfEntryPoint == 0x00001676) count_113c++;
		if (pHeader->OptionalHeader.AddressOfEntryPoint == 0x0000167E) count_113d++;
		GetModuleFileNameA(offset_D2Net, buffer, MAX_PATH);
		D2DX_LOG("D2Net.dll\t\tbase address %08X, loaded at %08X path %s%s", pHeader->OptionalHeader.ImageBase, (unsigned int)offset_D2Net, buffer,
			(pHeader->OptionalHeader.ImageBase == (unsigned int)offset_D2Net) ? "" : " (relocated)");
	}

	if (offset_D2Win != NULL) {
		pHeader = GetHeader((LPBYTE)offset_D2Win);
		if (pHeader->OptionalHeader.AddressOfEntryPoint == 0x00014F38) count_109b++;
		if (pHeader->OptionalHeader.AddressOfEntryPoint == 0x00014F38) count_109d++;
		if (pHeader->OptionalHeader.AddressOfEntryPoint == 0x00012EC0) count_110f++;
		if (pHeader->OptionalHeader.AddressOfEntryPoint == 0x0000187E) count_111++;
		if (pHeader->OptionalHeader.AddressOfEntryPoint == 0x0000187E) count_111b++;
		if (pHeader->OptionalHeader.AddressOfEntryPoint == 0x0000188E) count_112a++;
		if (pHeader->OptionalHeader.AddressOfEntryPoint == 0x0000187E) count_113c++;
		if (pHeader->OptionalHeader.AddressOfEntryPoint == 0x00001887) count_113d++;
		GetModuleFileNameA(offset_D2Win, buffer, MAX_PATH);
		D2DX_LOG("D2Win.dll\t\tbase address %08X, loaded at %08X path %s%s", pHeader->OptionalHeader.ImageBase, (unsigned int)offset_D2Win, buffer,
			(pHeader->OptionalHeader.ImageBase == (unsigned int)offset_D2Win) ? "" : " (relocated)");
	}

	if (offset_Fog != NULL) {
		pHeader = GetHeader((LPBYTE)offset_Fog);
		if (pHeader->OptionalHeader.AddressOfEntryPoint == 0x00013658) count_109b++;
		if (pHeader->OptionalHeader.AddressOfEntryPoint == 0x000142E7) count_109d++;
		if (pHeader->OptionalHeader.AddressOfEntryPoint == 0x000162B0) count_110f++;
		if (pHeader->OptionalHeader.AddressOfEntryPoint == 0x00003159) count_111++;
		if (pHeader->OptionalHeader.AddressOfEntryPoint == 0x00003142) count_111b++;
		if (pHeader->OptionalHeader.AddressOfEntryPoint == 0x0000314A) count_112a++;
		if (pHeader->OptionalHeader.AddressOfEntryPoint == 0x00003162) count_113c++;
		if (pHeader->OptionalHeader.AddressOfEntryPoint == 0x00003142) count_113d++;
		GetModuleFileNameA(offset_Fog, buffer, MAX_PATH);
		D2DX_LOG("Fog.dll\t\t\tbase address %08X, loaded at %08X path %s%s", pHeader->OptionalHeader.ImageBase, (unsigned int)offset_Fog, buffer,
			(pHeader->OptionalHeader.ImageBase == (unsigned int)offset_Fog) ? "" : " (relocated)");
	}

	if (offset_Storm != NULL) {
		pHeader = GetHeader((LPBYTE)offset_Storm);
		if (pHeader->OptionalHeader.AddressOfEntryPoint == 0x00013658) count_109b++;
		if (pHeader->OptionalHeader.AddressOfEntryPoint == 0x000142E7) count_109d++;
		if (pHeader->OptionalHeader.AddressOfEntryPoint == 0x000162B0) count_110f++;
		if (pHeader->OptionalHeader.AddressOfEntryPoint == 0x00003159) count_111++;
		if (pHeader->OptionalHeader.AddressOfEntryPoint == 0x00003142) count_111b++;
		if (pHeader->OptionalHeader.AddressOfEntryPoint == 0x0000314A) count_112a++;
		if (pHeader->OptionalHeader.AddressOfEntryPoint == 0x00003162) count_113c++;
		if (pHeader->OptionalHeader.AddressOfEntryPoint == 0x0003C3E0) count_113d++;
		GetModuleFileNameA(offset_Storm, buffer, MAX_PATH);
		D2DX_LOG("Storm.dll\t\tbase address %08X, loaded at %08X path %s%s", pHeader->OptionalHeader.ImageBase, (unsigned int)offset_Storm, buffer,
			(pHeader->OptionalHeader.ImageBase == (unsigned int)offset_Storm) ? "" : " (relocated)");
	}

	if (offset_D2Glide != NULL) {
		GetModuleFileNameA((HMODULE)offset_D2Glide, buffer, MAX_PATH);
		D2DX_LOG("D2Glide.dll\t\tbase address %08X, loaded at %08X path %s%s", pHeader->OptionalHeader.ImageBase, (unsigned int)offset_D2Glide, buffer,
			(pHeader->OptionalHeader.ImageBase == (unsigned int)offset_D2Glide) ? "" : " (relocated)");
	}

	if (offset_Game != NULL) {
		pHeader = GetHeader((LPBYTE)offset_Game);
		if (pHeader->OptionalHeader.AddressOfEntryPoint == 0x00291342) count_114a++;
		if (pHeader->OptionalHeader.AddressOfEntryPoint == 0x002854F2) count_114b++;
		if (pHeader->OptionalHeader.AddressOfEntryPoint == 0x002850E2) count_114c++;
		if (pHeader->OptionalHeader.AddressOfEntryPoint == 0x00282985) count_114d++;
		GetModuleFileNameA((HMODULE)offset_Game, buffer, MAX_PATH);
		D2DX_LOG("Game.exe\t\tbase address %08X, loaded at %08X path %s%s", pHeader->OptionalHeader.ImageBase, (unsigned int)offset_Game, buffer,
			(pHeader->OptionalHeader.ImageBase == (unsigned int)offset_Game) ? "" : " (relocated)");
	}

	int minimum_match_dll = 7;

	//if (count_109b >= minimum_match_dll) version = GameVersion::Lod109b;
	if (count_109d >= minimum_match_dll) version = GameVersion::Lod109d;
	if (count_110f >= minimum_match_dll) version = GameVersion::Lod110f;
	//if (count_111 >= minimum_match_dll) version = GameVersion::Lod111;
	//if (count_111b >= minimum_match_dll) version = GameVersion::Lod111b;
	if (count_112a >= minimum_match_dll) version = GameVersion::Lod112;
	if (count_113c >= minimum_match_dll) version = GameVersion::Lod113c;
	if (count_113d >= minimum_match_dll) version = GameVersion::Lod113d;

	if (count_114d != 0) version = GameVersion::Lod114d;

	//FreeLibrary(offset_D2Client);
	//FreeLibrary(offset_D2CMP);
	//FreeLibrary(offset_D2Common);
	//FreeLibrary(offset_D2Game);
	//FreeLibrary(offset_D2gfx);
	//FreeLibrary(offset_D2Lang);
	//FreeLibrary(offset_D2Launch);
	//FreeLibrary(offset_D2Net);
	//FreeLibrary(offset_D2Win);
	//FreeLibrary(offset_Fog);
	//FreeLibrary(offset_Storm);

	if (version == GameVersion::Unsupported)
	{
		MessageBoxA(NULL, "This version of Diablo II is not supported by D2DX. Please upgrade or downgrade to a supported version.", "D2DX", MB_OK);
	}

	return version;
}

bool GameHelper::TryApplyInGameFpsFix()
{
	/* The offsets taken from The Phrozen Keep: https://d2mods.info/forum/viewtopic.php?t=65239. */

	switch (_version)
	{
	case GameVersion::Lod109d:
		if (ProbeUInt32(_hD2ClientDll, 0x9B63, 0x2B756FBB))
		{
			PatchUInt32(_hD2ClientDll, 0x9B5F, 0x90909090);
			PatchUInt32(_hD2ClientDll, 0x9B63, 0x90909090);
		}
		break;
	case GameVersion::Lod110f:
		if (ProbeUInt32(_hD2ClientDll, 0xA2C9, 0x2B75C085))
		{
			PatchUInt32(_hD2ClientDll, 0xA2C9, 0x90909090);
		}
		break;
	case GameVersion::Lod112:
		if (ProbeUInt32(_hD2ClientDll, 0x7D1E5, 0x35756FBD))
		{
			PatchUInt32(_hD2ClientDll, 0x7D1E1, 0x90909090);
			PatchUInt32(_hD2ClientDll, 0x7D1E5, 0x90909090);
		}
		break;
	case GameVersion::Lod113c:
		if (ProbeUInt32(_hD2ClientDll, 0x44E4D, 0xFFFC8455))
		{
			PatchUInt32(_hD2ClientDll, 0x44E51, 0x90909090);
			PatchUInt32(_hD2ClientDll, 0x44E55, 0x90909090);
		}
		break;
	case GameVersion::Lod113d:
		if (ProbeUInt32(_hD2ClientDll, 0x45E9D, 0xFFFC738B))
		{
			PatchUInt32(_hD2ClientDll, 0x45EA1, 0x90909090);
			PatchUInt32(_hD2ClientDll, 0x45EA5, 0x90909090);
		}
		break;
	case GameVersion::Lod114d:
		if (ProbeUInt32(_hGameExe, 0x4F274, 0x000C6A68))
		{
			PatchUInt32(_hGameExe, 0x4F278, 0x90909090);
			PatchUInt32(_hGameExe, 0x4F27C, 0x90909090);
		}
		break;
	default:
		D2DX_LOG("Fps fix aborted: unsupported game version.");
		return false;
	}

	D2DX_LOG("Fps fix applied.");
	return true;
}

bool GameHelper::TryApplyMenuFpsFix()
{
	/* Patches found using 1.10 lead from D2Tweaks: https://github.com/Revan600/d2tweaks/. */

	switch (_version) 
	{
	case GameVersion::Lod109d:
		if (ProbeUInt32(_hD2WinDll, 0xEC0C, 0x5051196A))
		{
			PatchUInt32(_hD2WinDll, 0xEC0C, 0x50517F6A);
		}
		break;
	case GameVersion::Lod110f:
		if (ProbeUInt32(_hD2WinDll, 0xD029, 0x8128C783))
		{
			PatchUInt32(_hD2WinDll, 0xD029, 0x81909090);
		}
		break;
	case GameVersion::Lod112:
		if (ProbeUInt32(_hD2WinDll, 0xD949, 0x8128C783))
		{
			PatchUInt32(_hD2WinDll, 0xD949, 0x81909090);
		}
		break;
	case GameVersion::Lod113c:
		if (ProbeUInt32(_hD2WinDll, 0x18A19, 0x8128C783))
		{
			PatchUInt32(_hD2WinDll, 0x18A19, 0x81909090);
		}
		break;
	case GameVersion::Lod113d:
		if (ProbeUInt32(_hD2WinDll, 0xED69, 0x8128C783))
		{
			PatchUInt32(_hD2WinDll, 0xED69, 0x81909090);
		}
		break;
	case GameVersion::Lod114d:
		if (ProbeUInt32(_hGameExe, 0xFA62B, 0x8128C783))
		{
			PatchUInt32(_hGameExe, 0xFA62B, 0x81909090);
		}
		break;
	default:
		D2DX_LOG("Menu fps fix aborted: unsupported game version.");
		return false;
	}

	D2DX_LOG("Menu fps fix applied.");
	return true;
}

bool GameHelper::TryApplyInGameSleepFixes()
{
	switch (_version)
	{
	case GameVersion::Lod110f: 
		if (ProbeUInt32(_hD2ClientDll, 0x2684, 0x15FF0A6A))
		{
			PatchUInt32(_hD2ClientDll, 0x2684, 0x90909090);
			PatchUInt32(_hD2ClientDll, 0x2688, 0x90909090);
		}
		if (ProbeUInt32(_hD2ClientDll, 0x9E68, 0x83D7FF53))
		{
			PatchUInt32(_hD2ClientDll, 0x9E68, 0x83909090);
		}
		if (ProbeUInt32(_hD2ClientDll, 0x9E8C, 0x83D7FF53))
		{
			PatchUInt32(_hD2ClientDll, 0x9E8C, 0x83909090);
		}
		break;
	case GameVersion::Lod112:
		if (ProbeUInt32(_hD2ClientDll, 0x6CFD4, 0x15FF0A6A))
		{
			PatchUInt32(_hD2ClientDll, 0x6CFD4, 0x90909090);
		}

		if (ProbeUInt32(_hD2ClientDll, 0x6CFD8, 0x6FB7EF7C))
		{
			PatchUInt32(_hD2ClientDll, 0x6CFD8, 0x90909090);
		}

		if (ProbeUInt32(_hD2ClientDll, 0x7BD18, 0xD3FF006A))
		{
			PatchUInt32(_hD2ClientDll, 0x7BD18, 0x90909090);
		}

		if (ProbeUInt32(_hD2ClientDll, 0x7BD3D, 0xD3FF006A))
		{
			PatchUInt32(_hD2ClientDll, 0x7BD3D, 0x90909090);
		}
		break;
	case GameVersion::Lod113c:
		if (ProbeUInt32(_hD2WinDll, 0x18A63, 0xC815FF50) &&
			ProbeUInt32(_hD2WinDll, 0x18A67, 0xA16F8FB2))
		{
			PatchUInt32(_hD2WinDll, 0x18A63, 0x90909090);
			PatchUInt32(_hD2WinDll, 0x18A67, 0xA1909090);
		}

		if (ProbeUInt32(_hD2ClientDll, 0x3CB92, 0x0A6A0874))
		{
			PatchUInt32(_hD2ClientDll, 0x3CB92, 0x0A6A08EB);
		}

		if (ProbeUInt32(_hD2ClientDll, 0x43988, 0xD3FF006A))
		{
			PatchUInt32(_hD2ClientDll, 0x43988, 0x90909090);
		}

		if (ProbeUInt32(_hD2ClientDll, 0x439AD, 0xD3FF006A))
		{
			PatchUInt32(_hD2ClientDll, 0x439AD, 0x90909090);
		}

		break;
	case GameVersion::Lod113d:
		if (ProbeUInt32(_hD2WinDll, 0xEDB3, 0xB815FF50))
		{
			PatchUInt32(_hD2WinDll, 0xEDB3, 0x90909090);
		}

		if (ProbeUInt32(_hD2WinDll, 0xEDB7, 0xA16F8FB2))
		{
			PatchUInt32(_hD2WinDll, 0xEDB7, 0xA1909090);
		}

		if (ProbeUInt32(_hD2ClientDll, 0x27724, 0x15FF0A6A))
		{
			PatchUInt32(_hD2ClientDll, 0x27724, 0x90909090);
		}

		if (ProbeUInt32(_hD2ClientDll, 0x27728, 0x6FB7FF6C))
		{
			PatchUInt32(_hD2ClientDll, 0x27728, 0x90909090);
		}

		if (ProbeUInt32(_hD2ClientDll, 0x4494D, 0xD3FF006A))
		{
			PatchUInt32(_hD2ClientDll, 0x4494D, 0x90909090);
		}

		if (ProbeUInt32(_hD2ClientDll, 0x44928, 0xD3FF006A))
		{
			PatchUInt32(_hD2ClientDll, 0x44928, 0x90909090);
		}

		break;
	case GameVersion::Lod114d:
		if (ProbeUInt32(_hGameExe, 0x51C42, 0x15FF0A6A))
		{
			PatchUInt32(_hGameExe, 0x51C42, 0x90909090);
		}

		if (ProbeUInt32(_hGameExe, 0x51C46, 0x006CC258))
		{
			PatchUInt32(_hGameExe, 0x51C46, 0x90909090);
		}

		if (ProbeUInt32(_hGameExe, 0x4C711, 0xD7FF006A))
		{
			PatchUInt32(_hGameExe, 0x4C711, 0x90909090);
		}

		if (ProbeUInt32(_hGameExe, 0x4C740, 0xD7FF006A))
		{
			PatchUInt32(_hGameExe, 0x4C740, 0x90909090);
		}
		break;
	default:
		D2DX_LOG("In-game sleep fixes aborted: unsupported game version.");
		return false;
	}

	D2DX_LOG("In-game sleep fixes applied.");
	return true;
}

typedef D2::UnitAny* (__stdcall* GetClientPlayerFunc)();

D2::UnitAny* GameHelper::GetPlayerUnit() const
{
	GetClientPlayerFunc getClientPlayerFunc;

	switch (_version)
	{
	case GameVersion::Lod109d:
		getClientPlayerFunc = (GetClientPlayerFunc)((uintptr_t)_hD2ClientDll + 0x8CFC0);
		return getClientPlayerFunc();
	case GameVersion::Lod110f:
		getClientPlayerFunc = (GetClientPlayerFunc)((uintptr_t)_hD2ClientDll + 0x883D0);
		return getClientPlayerFunc();
	case GameVersion::Lod112:
		return (D2::UnitAny*)*(const uint32_t*)((uint32_t)_hD2ClientDll + 0x11C3D0);
	case GameVersion::Lod113c:
		return (D2::UnitAny*)*(const uint32_t*)((uint32_t)_hD2ClientDll + 0x11BBFC);
	case GameVersion::Lod113d:
		return (D2::UnitAny*)*(const uint32_t*)((uint32_t)_hD2ClientDll + 0x11D050);
	case GameVersion::Lod114d:
		return (D2::UnitAny*)*(const uint32_t*)((uint32_t)_hGameExe + 0x3A6A70);
	default:
		return nullptr;
	}
}

_Use_decl_annotations_
bool GameHelper::ProbeUInt32(
	HANDLE hModule, 
	uint32_t offset, 
	uint32_t probeValue)
{ 
	uint32_t* patchLocation = (uint32_t*)((uint32_t)hModule + offset);

	if (*patchLocation != probeValue)
	{
		//D2DX_LOG("Probe failed at %#010x, expected %#010x but found %#010x.", offset, probeValue, *patchLocation);
		return false;
	}

	return true;
}

_Use_decl_annotations_
void GameHelper::PatchUInt32(
	HANDLE hModule,
	uint32_t offset,
	uint32_t value)
{
	uint32_t* patchLocation = (uint32_t*)((uint32_t)hModule + offset);

	DWORD dwOldPage;
	VirtualProtect(patchLocation, 4, PAGE_EXECUTE_READWRITE, &dwOldPage);
	
	*patchLocation = value;

	VirtualProtect(patchLocation, 4, dwOldPage, &dwOldPage);
}

_Use_decl_annotations_
D2::UnitType GameHelper::GetUnitType(
	const D2::UnitAny* unit) const
{
	switch (_version) {
	case GameVersion::Lod109d:
		return unit->u.v109.dwType;
	case GameVersion::Lod110f:
		return unit->u.v110.dwType;
	default:
		return unit->u.v112.dwType;
	}
}

_Use_decl_annotations_
uint32_t GameHelper::GetUnitId(
	const D2::UnitAny* unit) const
{
	switch (_version) {
	case GameVersion::Lod109d:
		return unit->u.v109.dwUnitId;
	case GameVersion::Lod110f:
		return unit->u.v110.dwUnitId;
	default:
		return unit->u.v112.dwUnitId;
	}
}

_Use_decl_annotations_
Offset GameHelper::GetUnitPos(
	const D2::UnitAny* unit) const
{
	auto unitType = GetUnitType(unit);

	if (unitType == D2::UnitType::Player ||
		unitType == D2::UnitType::Monster ||
		unitType == D2::UnitType::Missile)
	{
		D2::Path* path;
		switch (_version) {
		case GameVersion::Lod109d:
			path = unit->u.v109.path;
			break;
		case GameVersion::Lod110f:
			path = unit->u.v110.path;
			break;
		default:
			path = unit->u.v112.path;
			break;
		}
		return { (int32_t)path->x, (int32_t)path->y };
	}
	else
	{
		D2::StaticPath* path;
		switch (_version) {
		case GameVersion::Lod109d:
			path = unit->u.v109.staticPath;
			break;
		case GameVersion::Lod110f:
			path = unit->u.v110.staticPath;
			break;
		default:
			path = unit->u.v112.staticPath;
			break;
		}
		return { (int32_t)path->xPos * 65536 + (int32_t)path->xOffset, (int32_t)path->yPos * 65536 + (int32_t)path->yOffset };
	}
}

typedef D2::UnitAny* (__fastcall* FindUnitFunc)(DWORD dwId, DWORD dwType);

static D2::UnitAny* __fastcall FindClientSideUnit109d(DWORD unitId, DWORD unitType)
{
	uint32_t** unitPtrTable = (uint32_t**)0x6FBC4BF8;
	D2::Unit109* unit = (D2::Unit109*)unitPtrTable[unitType * 128 + (unitId & 127)];

	while (unit)
	{
		if ((uint32_t)unit->dwType == unitType && unit->dwUnitId == unitId)
		{
			return (D2::UnitAny*)unit;
		}

		unit = unit->pPrevUnit;
	}

	return nullptr;
}

static D2::UnitAny* __fastcall FindServerSideUnit109d(DWORD unitId, DWORD unitType)
{
	uint32_t** unitPtrTable = (uint32_t**)0x6FBC57F8;
	D2::Unit109* unit = (D2::Unit109*)unitPtrTable[unitType * 128 + (unitId & 127)];

	while (unit)
	{
		if ((uint32_t)unit->dwType == unitType && unit->dwUnitId == unitId)
		{
			return (D2::UnitAny*)unit;
		}

		unit = unit->pPrevUnit;
	}

	return nullptr;
}

static D2::UnitAny* __fastcall FindClientSideUnit110f(DWORD unitId, DWORD unitType)
{
	uint32_t** unitPtrTable = (uint32_t**)0x6FBBAA00;
	D2::Unit110* unit = (D2::Unit110*)unitPtrTable[unitType * 128 + (unitId & 127)];

	while (unit)
	{
		if ((uint32_t)unit->dwType == unitType && unit->dwUnitId == unitId)
		{
			return (D2::UnitAny*)unit;
		}

		unit = unit->pPrevUnit;
	}

	return nullptr;
}

static D2::UnitAny* __fastcall FindServerSideUnit110f(DWORD unitId, DWORD unitType)
{
	uint32_t** unitPtrTable = (uint32_t**)0x6FBBB600;
	D2::Unit110* unit = (D2::Unit110*)unitPtrTable[unitType * 128 + (unitId & 127)];

	while (unit)
	{
		if ((uint32_t)unit->dwType == unitType && unit->dwUnitId == unitId)
		{
			return (D2::UnitAny*)unit;
		}

		unit = unit->pPrevUnit;
	}

	return nullptr;
}

_Use_decl_annotations_
D2::UnitAny* GameHelper::FindUnit(
	uint32_t unitId,
	D2::UnitType unitType) const
{
	FindUnitFunc findClientSideUnit = (FindUnitFunc)GetFunction(D2Function::D2Client_FindClientSideUnit);
	FindUnitFunc findServerSideUnit = (FindUnitFunc)GetFunction(D2Function::D2Client_FindServerSideUnit);

	if (findClientSideUnit)
	{
		auto unit = findClientSideUnit((DWORD)unitId, (DWORD)unitType);

		if (unit)
		{
			return unit;
		}
	}

	return findServerSideUnit ? findServerSideUnit((DWORD)unitId, (DWORD)unitType) : nullptr;
}

_Use_decl_annotations_
void* GameHelper::GetFunction(
	D2Function function) const
{
	HANDLE hModule = nullptr;
	int32_t ordinal = 0;

	switch (_version)
	{
	case GameVersion::Lod109d:
		switch (function)
		{
		case D2Function::D2Gfx_DrawImage:
			hModule = _hD2GfxDll;
			ordinal = 10072;
			break;
		case D2Function::D2Gfx_DrawShiftedImage:
			hModule = _hD2GfxDll;
			ordinal = 10073;
			break;
		case D2Function::D2Gfx_DrawVerticalCropImage:
			hModule = _hD2GfxDll;
			ordinal = 10074;
			break;
		case D2Function::D2Gfx_DrawClippedImage:
			hModule = _hD2GfxDll;
			ordinal = 10077;
			break;
		case D2Function::D2Gfx_DrawImageFast:
			hModule = _hD2GfxDll;
			ordinal = 10076;
			break;
		case D2Function::D2Gfx_DrawShadow:
			hModule = _hD2GfxDll;
			ordinal = 10075;
			break;
		case D2Function::D2Win_DrawText:
			hModule = _hD2WinDll;
			ordinal = 10117;
			break;
		case D2Function::D2Win_DrawFramedText:
			hModule = _hD2WinDll;
			ordinal = 10129;
			break;
		case D2Function::D2Win_DrawRectangledText:
			hModule = _hD2WinDll;
			ordinal = 10132;
			break;
		case D2Function::D2Client_DrawUnit:
			return (void*)((uintptr_t)_hD2ClientDll + 0xB8350);
		case D2Function::D2Client_FindClientSideUnit:
			//return (void*)FindClientSideUnit109d;
			return (void*)((uintptr_t)_hD2ClientDll + 0x8B560);
		case D2Function::D2Client_DrawWeatherParticles:
			return (void*)((uintptr_t)_hD2ClientDll + 0x07BC0);
		case D2Function::D2Client_FindServerSideUnit:
			//return (void*)FindServerSideUnit109d;
			return (void*)((uintptr_t)_hD2ClientDll + 0x8B5D0);
		default:
			break;
		}
		break;
	case GameVersion::Lod110f:
		switch (function)
		{
		case D2Function::D2Gfx_DrawImage:
			hModule = _hD2GfxDll;
			ordinal = 10072;
			break;
		case D2Function::D2Gfx_DrawShiftedImage:
			hModule = _hD2GfxDll;
			ordinal = 10073;
			break;
		case D2Function::D2Gfx_DrawVerticalCropImage:
			hModule = _hD2GfxDll;
			ordinal = 10074;
			break;
		case D2Function::D2Gfx_DrawClippedImage:
			hModule = _hD2GfxDll;
			ordinal = 10077;
			break;
		case D2Function::D2Gfx_DrawImageFast:
			hModule = _hD2GfxDll;
			ordinal = 10076;
			break;
		case D2Function::D2Gfx_DrawShadow: 
			hModule = _hD2GfxDll;
			ordinal = 10075;
			break; 
		case D2Function::D2Win_DrawText:
			hModule = _hD2WinDll;
			ordinal = 10117;
			break;
		case D2Function::D2Win_DrawFramedText:
			hModule = _hD2WinDll;
			ordinal = 10129;
			break;
		case D2Function::D2Win_DrawRectangledText:
			hModule = _hD2WinDll;
			ordinal = 10132;
			break;
		case D2Function::D2Client_DrawUnit:
			return (void*)((uintptr_t)_hD2ClientDll + 0xBA720);
		case D2Function::D2Client_FindClientSideUnit:
			//return (void*)FindClientSideUnit110f;
			return (void*)((uintptr_t)_hD2ClientDll + 0x869F0);
		case D2Function::D2Client_DrawWeatherParticles:
			return (void*)((uintptr_t)_hD2ClientDll + 0x08240);
		case D2Function::D2Client_FindServerSideUnit:
			//return (void*)FindServerSideUnit110f;
			return (void*)((uintptr_t)_hD2ClientDll + 0x86A60);
		default:
			break;
		}
		break;
	case GameVersion::Lod112:
		switch (function)
		{
		case D2Function::D2Gfx_DrawImage:
			hModule = _hD2GfxDll;
			ordinal = 10024;
			break;
		case D2Function::D2Gfx_DrawShiftedImage:
			hModule = _hD2GfxDll;
			ordinal = 10044;
			break;
		case D2Function::D2Gfx_DrawVerticalCropImage:
			hModule = _hD2GfxDll;
			ordinal = 10046;
			break;
		case D2Function::D2Gfx_DrawClippedImage:
			hModule = _hD2GfxDll;
			ordinal = 10061;
			break;
		case D2Function::D2Gfx_DrawImageFast:
			hModule = _hD2GfxDll;
			ordinal = 10012;
			break;
		case D2Function::D2Gfx_DrawShadow:
			hModule = _hD2GfxDll;
			ordinal = 10030;
			break;
		case D2Function::D2Win_DrawText:
			hModule = _hD2WinDll;
			ordinal = 10001;
			break;
		case D2Function::D2Win_DrawFramedText:
			hModule = _hD2WinDll;
			ordinal = 10031;
			break;
		case D2Function::D2Win_DrawRectangledText:
			hModule = _hD2WinDll;
			ordinal = 10082;
			break;
		case D2Function::D2Client_DrawUnit:
			return (void*)((uintptr_t)_hD2ClientDll + 0x94250);
		case D2Function::D2Client_DrawMissile:
			return (void*)((uintptr_t)_hD2ClientDll + 0x949C0);
		case D2Function::D2Client_DrawWeatherParticles:
			return (void*)((uintptr_t)_hD2ClientDll + 0x14210);
		case D2Function::D2Client_FindClientSideUnit:
			return (void*)((uintptr_t)_hD2ClientDll + 0x1F1A0);
		case D2Function::D2Client_FindServerSideUnit:
			return (void*)((uintptr_t)_hD2ClientDll + 0x1F1C0);
		default:
			break;
		}
		break;
	case GameVersion::Lod113c:
		switch (function)
		{
		case D2Function::D2Gfx_DrawImage:
			hModule = _hD2GfxDll;
			ordinal = 10041;
			break;
		case D2Function::D2Gfx_DrawShiftedImage:
			hModule = _hD2GfxDll;
			ordinal = 10019;
			break;
		case D2Function::D2Gfx_DrawVerticalCropImage:
			hModule = _hD2GfxDll;
			ordinal = 10074;
			break;
		case D2Function::D2Gfx_DrawClippedImage:
			hModule = _hD2GfxDll;
			ordinal = 10079;
			break;
		case D2Function::D2Gfx_DrawImageFast:
			hModule = _hD2GfxDll;
			ordinal = 10046;
			break;
		case D2Function::D2Gfx_DrawShadow:
			hModule = _hD2GfxDll;
			ordinal = 10011;
			break;
		case D2Function::D2Win_DrawText:
			hModule = _hD2WinDll;
			ordinal = 10096;
			break;
		case D2Function::D2Win_DrawFramedText:
			hModule = _hD2WinDll; 
			ordinal = 10085;
			break;
		case D2Function::D2Win_DrawRectangledText:
			hModule = _hD2WinDll;
			ordinal = 10013;
			break;
		case D2Function::D2Client_DrawUnit:
			return (void*)((uintptr_t)_hD2ClientDll + 0x6C490);
		case D2Function::D2Client_DrawMissile:
			return (void*)((uintptr_t)_hD2ClientDll + 0x6CC00);
		case D2Function::D2Client_DrawWeatherParticles:
			return (void*)((uintptr_t)_hD2ClientDll + 0x7FE80);
		case D2Function::D2Client_FindClientSideUnit:
			return (void*)((uintptr_t)_hD2ClientDll + 0xA5B20);
		case D2Function::D2Client_FindServerSideUnit:
			return (void*)((uintptr_t)_hD2ClientDll + 0xA5B40);
		default:
			break;
		}
		break;
	case GameVersion::Lod113d:
		switch (function)
		{
		case D2Function::D2Gfx_DrawImage:
			hModule = _hD2GfxDll;
			ordinal = 10042;
			break;
		case D2Function::D2Gfx_DrawShiftedImage:
			hModule = _hD2GfxDll;
			ordinal = 10067;
			break;
		case D2Function::D2Gfx_DrawVerticalCropImage:
			hModule = _hD2GfxDll;
			ordinal = 10082;
			break;
		case D2Function::D2Gfx_DrawClippedImage:
			hModule = _hD2GfxDll;
			ordinal = 10015;
			break;
		case D2Function::D2Gfx_DrawImageFast:
			hModule = _hD2GfxDll;
			ordinal = 10006;
			break;
		case D2Function::D2Gfx_DrawShadow:
			hModule = _hD2GfxDll;
			ordinal = 10084;
			break;
		case D2Function::D2Win_DrawText:
			hModule = _hD2WinDll;
			ordinal = 10076;
			break;
		case D2Function::D2Win_DrawTextEx:
			hModule = _hD2WinDll;
			ordinal = 10084;
			break;
		case D2Function::D2Win_DrawFramedText:
			hModule = _hD2WinDll;
			ordinal = 10137;
			break;
		case D2Function::D2Win_DrawRectangledText:
			hModule = _hD2WinDll;
			ordinal = 10078;
			break;
		case D2Function::D2Client_DrawUnit:
			return (void*)((uintptr_t)_hD2ClientDll + 0x605b0);
		case D2Function::D2Client_DrawMissile:
			return (void*)((uintptr_t)_hD2ClientDll + 0x60C70);
		case D2Function::D2Client_DrawWeatherParticles:
			return (void*)((uintptr_t)_hD2ClientDll + 0x4AD90);
		case D2Function::D2Client_FindClientSideUnit:
			return (void*)((uintptr_t)_hD2ClientDll + 0x620B0);
		case D2Function::D2Client_FindServerSideUnit:
			return (void*)((uintptr_t)_hD2ClientDll + 0x620D0);
		default:
			break;
		}
		break;
	case GameVersion::Lod114d:
		switch (function)
		{
		case D2Function::D2Gfx_DrawImage:
			return (void*)((uintptr_t)_hGameExe + 0xF6480);
		case D2Function::D2Gfx_DrawShiftedImage:
			return (void*)((uintptr_t)_hGameExe + 0xF64B0);
		case D2Function::D2Gfx_DrawVerticalCropImage:
			return (void*)((uintptr_t)_hGameExe + 0xF64E0);
		case D2Function::D2Gfx_DrawClippedImage:
			return (void*)((uintptr_t)_hGameExe + 0xF6510);
		case D2Function::D2Gfx_DrawImageFast:
			return (void*)((uintptr_t)_hGameExe + 0xF6570);
		case D2Function::D2Gfx_DrawShadow:
			return (void*)((uintptr_t)_hGameExe + 0xF6540);
		case D2Function::D2Win_DrawText:
			return (void*)((uintptr_t)_hGameExe + 0x102320);
		case D2Function::D2Win_DrawTextEx:
			return (void*)((uintptr_t)_hGameExe + 0x102360);
		case D2Function::D2Win_DrawFramedText:
			return (void*)((uintptr_t)_hGameExe + 0x102280);
		case D2Function::D2Win_DrawRectangledText:
			return (void*)((uintptr_t)_hGameExe + 0x1023B0);
		case D2Function::D2Client_DrawUnit:
			return (void*)((uintptr_t)_hGameExe + 0x70EC0);
		case D2Function::D2Client_DrawMissile:
			return (void*)((uintptr_t)_hGameExe + 0x71EC0);
		case D2Function::D2Client_DrawWeatherParticles:
			return (void*)((uintptr_t)_hGameExe + 0x73470);
		case D2Function::D2Client_FindClientSideUnit:
			return (void*)((uintptr_t)_hGameExe + 0x63990);
		case D2Function::D2Client_FindServerSideUnit:
			return (void*)((uintptr_t)_hGameExe + 0x639B0);
		default:
			break;
		}
		break;
	default:
		break;
	}

	if (!hModule || !ordinal)
	{
		return nullptr;
	}

	return GetProcAddress((HMODULE)hModule, MAKEINTRESOURCEA(ordinal));
}

_Use_decl_annotations_
DrawParameters GameHelper::GetDrawParameters(
	const D2::CellContextAny* cellContext) const
{
	switch (GetVersion()) {
	case GameVersion::Lod109d:
	case GameVersion::Lod110f:
		return {
			.unitId = cellContext->u.v109.dwUnit,
			.unitType = cellContext->u.v109.dwClass,
			.unitToken = cellContext->u.v109.dwUnitToken,
			.unitMode = cellContext->u.v109.dwMode

		};
	case GameVersion::Lod112:
		return {
			.unitId = cellContext->u.v112.dwUnit,
			.unitType = cellContext->u.v112.dwClass,
			.unitToken = cellContext->u.v112.dwPlayerType,
			.unitMode = cellContext->u.v112.dwMode
		};
	case GameVersion::Lod113c:
	case GameVersion::Lod113d:
	case GameVersion::Lod114d:
	default:
		return {
			.unitId = cellContext->u.v113.dwUnit,
			.unitType = cellContext->u.v113.dwClass,
			.unitToken = cellContext->u.v113.dwPlayerType,
			.unitMode = cellContext->u.v113.dwMode
		};
	}
}

int32_t GameHelper::GetCurrentAct() const
{
	auto unit = GetPlayerUnit();

	if (!unit)
	{
		return -1;
	}

	switch (_version) {
	case GameVersion::Lod109d:
		return (int32_t)unit->u.v109.dwAct;
	case GameVersion::Lod110f:
		return (int32_t)unit->u.v110.dwAct;
	default:
		return (int32_t)unit->u.v112.dwAct;
	}
}

bool GameHelper::IsGameMenuOpen() const
{
	switch (_version)
	{
	case GameVersion::Lod109d:
		return *((uint32_t*)((uint32_t)_hD2ClientDll + 0x1248D8)) != 0;
	case GameVersion::Lod110f:
		return *((uint32_t*)((uint32_t)_hD2ClientDll + 0x11A6CC)) != 0;
	case GameVersion::Lod112:
		return *((uint32_t*)((uint32_t)_hD2ClientDll + 0x102B7C)) != 0;
	case GameVersion::Lod113c:
		return *((uint32_t*)((uint32_t)_hD2ClientDll + 0xFADA4)) != 0;
	case GameVersion::Lod113d:
		return *((uint32_t*)((uint32_t)_hD2ClientDll + 0x11C8B4)) != 0;
	case GameVersion::Lod114d:
		return *((uint32_t*)((uint32_t)_hGameExe + 0x3A27E4)) != 0;
	default:
		return false;
	}
}

bool GameHelper::IsInGame() const
{
	auto playerUnit = GetPlayerUnit();

	switch (_version)
	{
	case GameVersion::Lod109d:
		return *((uint32_t*)((uint32_t)_hD2ClientDll + 0x1109FC)) != 0 && playerUnit != 0 && playerUnit->u.v109.path != 0;
	case GameVersion::Lod110f:
		return *((uint32_t*)((uint32_t)_hD2ClientDll + 0x1077C4)) != 0 && playerUnit != 0 && playerUnit->u.v110.path != 0;
	case GameVersion::Lod112:
		return *((uint32_t*)((uint32_t)_hD2ClientDll + 0x11BCC4)) != 0 && playerUnit != 0 && playerUnit->u.v112.path != 0;
	case GameVersion::Lod113c:
		return *((uint32_t*)((uint32_t)_hD2ClientDll + 0xF8C9C)) != 0 && playerUnit != 0 && playerUnit->u.v112.path != 0;
	case GameVersion::Lod113d:
		return *((uint32_t*)((uint32_t)_hD2ClientDll + 0xF79E0)) != 0 && playerUnit != 0 && playerUnit->u.v112.path != 0;
	case GameVersion::Lod114d:
		return *((uint32_t*)((uint32_t)_hGameExe + 0x3A27C0)) != 0 && playerUnit != 0 && playerUnit->u.v112.path != 0;
	default:
		return false;
	}
}

bool GameHelper::IsProjectDiablo2() const
{
	return _isProjectDiablo2;
}
