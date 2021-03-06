#include <Windows.h>
#include <Psapi.h>

namespace util
{
	uintptr_t FindPattern(const char *pattern, const char *mask, const char* address, size_t size)
	{
		const char* addressEnd = address + size;
		const size_t maskLength = static_cast<size_t>(strlen(mask) - 1);

		for (size_t i = 0; address < addressEnd; address++)
		{
			if (*address == pattern[i] || mask[i] == '?')
			{
				if (mask[i + 1] == '\0')
				{
					return reinterpret_cast<uintptr_t>(address) - maskLength;
				}

				i++;
			}
			else
			{
				i = 0;
			}
		}

		return 0;
	}

	uintptr_t FindPattern(const char *pattern, const char *mask)
	{
		MODULEINFO module = {};
		GetModuleInformation(GetCurrentProcess(), GetModuleHandle(nullptr), &module, sizeof(MODULEINFO));

		return FindPattern(pattern, mask, (const char*)module.lpBaseOfDll, module.SizeOfImage);
	}
}

void Adjust()
{
	int newSize = GetPrivateProfileInt("Adjuster", "CWeaponInfoBlobs", 0xFFFFFFFF, ".\\CWeaponInfoBlob Adjuster.ini");

	uintptr_t address = util::FindPattern("\xBA\x00\x00\x00\x00\x4C\x89\x83\x00\x00\x00\x00\x44\x89\x83\x00\x00\x00\x00\xE8", "x????xxx????xxx????x");
	if(address)
	{
		address += 1;

		DWORD oldProtect;
		VirtualProtect((void*)address, sizeof(int), PAGE_EXECUTE_READWRITE, &oldProtect);

		int* size = (int*)address;

		if(newSize == 0xFFFFFFFF)
		{
			newSize = *size + 25;
		}
		
		*size = newSize;

		VirtualProtect((void*)address, sizeof(int), oldProtect, &oldProtect);
	}
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved)
{
	if (reason == DLL_PROCESS_ATTACH)
	{
		Adjust();
	}

	return TRUE;
}

