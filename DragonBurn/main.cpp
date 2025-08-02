//
//______                            ______                  
//|  _  \                           | ___ \                 
//| | | |_ __ __ _  __ _  ___  _ __ | |_/ /_   _ _ __ _ __  
//| | | | '__/ _` |/ _` |/ _ \| '_ \| ___ \ | | | '__| '_ \ 
//| |/ /| | | (_| | (_| | (_) | | | | |_/ / |_| | |  | | | |
//|___/ |_|  \__,_|\__, |\___/|_| |_\____/ \__,_|_|  |_| |_|
//                  __/ |                                   
//                 |___/                                    
//https://github.com/ByteCorum/DragonBurn

#include "Core\Cheats.h"
#include "Offsets\Offsets.h"
#include "Resources\Language.hpp"
#include "Core\Init.h"
#include "Config\ConfigSaver.h"
#include "Helpers\Logger.h"
#include <filesystem>
#include <KnownFolders.h>
#include <ShlObj.h>

using namespace std;

namespace fs = filesystem;
string fileName;

// Constants for timing (non-security related)
constexpr int CS2_CONNECTION_WAIT_MS = 20000;
constexpr int CONSOLE_HIDE_DELAY_MS = 3000;

// Helper functions for error handling

void HandleOffsetUpdateResult(int result)
{
	Log::PreviousLine();
	switch (result)
	{
	case 0:
		Log::Error("Bad internet connection");
		break;
	case 1:
		Log::Error("Failed to UpdateOffsets");
		break;
	case 2:
		Log::Fine("Offsets updated");
		break;
	default:
		Log::Error("Unknown connection error");
		break;
	}
}

void HandleCS2VersionCheckResult(int result)
{
	Log::PreviousLine();
	switch (result)
	{
	case 0:
		Log::Error("Failed to get the current game version");
		break;
	case 1:
		Log::Warning("Offsets are outdated, we'll update them asap. With current offsets, cheat may work unstable", true);
		break;
	case 2:
		Log::Error("Failed to get cloud version");
		break;
	case 3:
		break; // Success, no message needed
	default:
		Log::Error("Failed to get the current game version");
		break;
	}
}

void Cheat();

int main()
{
	Cheat();
}

void Cheat()
{
	ShowWindow(GetConsoleWindow(), SW_SHOWNORMAL);
	SetConsoleTitle(L"DragonBurn");
	//Init::Verify::RandTitle();

	Log::Custom(R"LOGO(______                            ______                  
|  _  \                           | ___ \                 
| | | |_ __ __ _  __ _  ___  _ __ | |_/ /_   _ _ __ _ __  
| | | | '__/ _` |/ _` |/ _ \| '_ \| ___ \ | | | '__| '_ \ 
| |/ /| | | (_| | (_| | (_) | | | | |_/ / |_| | |  | | | |
|___/ |_|  \__,_|\__, |\___/|_| |_\____/ \__,_|_|  |_| |_|
                  __/ |                                   
                 |___/                                    
https://github.com/ByteCorum/DragonBurn


)LOGO", 13);

	if (!Init::Verify::CheckWindowVersion())
	{
		Log::Warning("Your os is unsupported, bugs may occurred", true);
	}


	Log::Info("Updating offsets");
	HandleOffsetUpdateResult(Offset.UpdateOffsets());

	Log::Info("Connecting to kernel mode driver");
	if (memoryManager.ConnectDriver(L"\\\\.\\DragonBurn-kernel"))
	{
		Log::PreviousLine();
		Log::Fine("Successfully connected to kernel mode driver");
	}
	else
	{
		Log::PreviousLine();
		Log::Error("Failed to connect to kernel mode driver");
	}

	std::cout << '\n';
	bool preStart = false;
	while (memoryManager.GetProcessID(L"cs2.exe") == 0)
	{
		Log::PreviousLine();
		Log::Info("Waiting for CS2");
		preStart = true;
	}

	if (preStart)
	{
		Log::PreviousLine();
		Log::Info("Connecting to CS2(it may take some time)");
		Sleep(CS2_CONNECTION_WAIT_MS);
	}

	Log::PreviousLine();
	Log::Fine("Connected to CS2");
	Log::Info("Linking to CS2");

#ifndef DBDEBUG
	HandleCS2VersionCheckResult(Init::Client::CheckCS2Version());
#endif

	if (!memoryManager.Attach(memoryManager.GetProcessID(L"cs2.exe")))
	{
		Log::PreviousLine();
		Log::Error("Failed to attach to the process");
	}

	if (!gGame.InitAddress())
	{
		Log::PreviousLine();
		Log::Error("Failed to Init Address");
	}

	Log::PreviousLine();
	Log::Fine("Linked to CS2");

	char documentsPath[MAX_PATH];
	if (SHGetFolderPathA(NULL, CSIDL_PERSONAL, NULL, 0, documentsPath) != S_OK)
		Log::Error("Failed to get the Documents folder path");

	MenuConfig::path = documentsPath;
	MenuConfig::docPath = documentsPath;
	MenuConfig::path += "\\DragonBurn";

	if (fs::exists(MenuConfig::docPath + "\\Adobe Software Data"))
	{
		fs::rename(MenuConfig::docPath + "\\Adobe Software Data", MenuConfig::path);
	}

	if (fs::exists(MenuConfig::path))
	{
		Log::Fine("Config folder connected: " + MenuConfig::path);
	}
	else
	{
		if (fs::create_directory(MenuConfig::path))
		{
			Log::Fine("Config folder connected: " + MenuConfig::path);
		}
		else
		{
			Log::Error("Failed to create the config directory");
		}
	}

	if (fs::exists(MenuConfig::path + "\\default.cfg"))
		MenuConfig::defaultConfig = true;

	Log::Fine("DragonBurn loaded");

#ifndef DBDEBUG
	Sleep(CONSOLE_HIDE_DELAY_MS);
	ShowWindow(GetConsoleWindow(), SW_HIDE);
#endif

	try
	{
		Gui.AttachAnotherWindow("Counter-Strike 2", "SDL_app", Cheats::Run);
	}
	catch (OSImGui::OSException& e)
	{
		Log::Error(e.what());
	}
}