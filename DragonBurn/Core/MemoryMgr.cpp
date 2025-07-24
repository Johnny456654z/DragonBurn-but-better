#include "MemoryMgr.h"

MemoryMgr::MemoryMgr() 
{
	ProcessID = 0;
	kernelDriver = nullptr;
}

MemoryMgr::~MemoryMgr() 
{
	// Ensure proper cleanup
	if (kernelDriver != nullptr && kernelDriver != INVALID_HANDLE_VALUE)
	{
		DisconnectDriver();
	}

	ProcessID = 0;
	kernelDriver = nullptr;
}

bool MemoryMgr::ConnectDriver(const LPCWSTR name)
{
	kernelDriver = CreateFile(name, GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (kernelDriver == INVALID_HANDLE_VALUE)
		return false;

	return true;
}

bool MemoryMgr::DisconnectDriver() 
{
	if (kernelDriver != nullptr)
	{
		return CloseHandle(kernelDriver);
	}
	else
		return false;
}

bool MemoryMgr::Attach(const DWORD pid)
{
	if (pid == 0)
		return false;

	ProcessID = pid;
	return true;
}

DWORD MemoryMgr::GetProcessID(const wchar_t* processName)
{
	if (kernelDriver != nullptr)
	{
		PID_PACK PidPack;
		RtlZeroMemory(PidPack.name, PROCESS_NAME_BUFFER_SIZE * sizeof(WCHAR));
		wcsncpy(PidPack.name, processName, PROCESS_NAME_BUFFER_SIZE - 1);
		PidPack.name[PROCESS_NAME_BUFFER_SIZE - 1] = L'\0'; // Ensure null termination

		BOOL result = DeviceIoControl(kernelDriver,
			IOCTL_GET_PID,
			&PidPack,
			sizeof(PidPack),
			&PidPack,
			sizeof(PidPack),
			nullptr,
			nullptr);

		if (result == TRUE)
			return PidPack.pid;
		else
			return 0;
	}
	else
		return 0;
}

DWORD64 MemoryMgr::GetModuleBase(const wchar_t* moduleName)
{
	if (kernelDriver != nullptr && ProcessID != 0)
	{
		MODULE_PACK ModulePack;
		DWORD64 address = 0;
		ModulePack.pid = ProcessID;
		ModulePack.baseAddress = address;
		RtlZeroMemory(ModulePack.moduleName, MODULE_NAME_BUFFER_SIZE * sizeof(WCHAR));
		wcsncpy(ModulePack.moduleName, moduleName, MODULE_NAME_BUFFER_SIZE - 1);
		ModulePack.moduleName[MODULE_NAME_BUFFER_SIZE - 1] = L'\0'; // Ensure null termination

		BOOL result = DeviceIoControl(kernelDriver,
			IOCTL_GET_MODULE_BASE,
			&ModulePack,
			sizeof(ModulePack),
			&ModulePack,
			sizeof(ModulePack),
			nullptr,
			nullptr);

		if (result == TRUE)
			return ModulePack.baseAddress;
		else
			return 0;
	}
	else
		return 0;
}

DWORD64 MemoryMgr::TraceAddress(DWORD64 baseAddress, std::vector<DWORD> offsets)
{
	if (kernelDriver != nullptr && ProcessID != 0)
	{
		DWORD64 address = 0;

		if (offsets.size() == 0)
			return baseAddress;

		if (!ReadMemory<DWORD64>(baseAddress, address))
			return 0;

		for (int i = 0; i < offsets.size() - 1; i++)
		{
			if (!ReadMemory<DWORD64>(address + offsets[i], address))
				return 0;
		}
		return address == 0 ? 0 : address + offsets[offsets.size() - 1];
	}
	else
		return 0;
}