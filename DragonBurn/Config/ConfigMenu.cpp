#include "ConfigMenu.h"
#include "..\Core\Config.h"
#include "ConfigSaver.h"
#include "..\Features\TriggerBot.h"
#include "..\Features\Aimbot.h"
#include <filesystem>
#include <string>
#include "..\Resources\Language.hpp"
#include "..\Features\RCS.h"
#include "..\Helpers\KeyManager.h"

namespace ConfigMenu {
	
	// Helper function to render a button with conditional disable state
	bool RenderConditionalButton(const char* label, const ImVec2& size, bool condition)
	{
		if (!condition) ImGui::BeginDisabled();
		bool result = ImGui::Button(label, size) && condition;
		if (!condition) ImGui::EndDisabled();
		return result;
	}
	
	void RenderCFGmenu()
	{
		static char configNameBuffer[128] = "";
		static std::string selectedConfigName = "";
		static int selectedConfigIndex = -1;
		static bool showSaveConfirm = false;

		const std::string configDir = MenuConfig::path;
		static std::vector<std::string> configFiles;
		std::vector<const char*> configFilesCStr;

		// Rebuild config file list
		configFiles.clear();
		for (const auto& entry : std::filesystem::directory_iterator(configDir))
		{
			if (entry.is_regular_file() && entry.path().extension() == ".cfg")
			{
				std::string filename = entry.path().filename().string();
				if (filename.length() > 4)
				{
					filename = filename.substr(0, filename.length() - 4);
				}
				configFiles.push_back(filename);
			}
		}
		
		for (const auto& file : configFiles)
		{
			configFilesCStr.push_back(file.c_str());
		}

		// Find selected config index
		selectedConfigIndex = -1;
		if (!selectedConfigName.empty())
		{
			for (int i = 0; i < configFiles.size(); i++)
			{
				if (configFiles[i] == selectedConfigName)
				{
					selectedConfigIndex = i;
					break;
				}
			}
			if (selectedConfigIndex == -1)
			{
				selectedConfigName = "";
			}
		}

		// Save confirmation popup - at the top level
		if (showSaveConfirm)
		{
			ImGui::OpenPopup("Save Config Confirmation");
			showSaveConfirm = false;
		}

		if (ImGui::BeginPopupModal("Save Config Confirmation", NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::Text("Save current settings to '%s'?", selectedConfigName.c_str());
			ImGui::Text("This will overwrite the existing configuration.");
			ImGui::Separator();
			
			// Green Yes button
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.7f, 0.2f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.8f, 0.3f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 0.6f, 0.1f, 1.0f));
			if (ImGui::Button("Yes", ImVec2(80, 0)))
			{
				MyConfigSaver::SaveConfig(selectedConfigName + ".cfg");
				ImGui::CloseCurrentPopup();
			}
			ImGui::PopStyleColor(3);
			
			ImGui::SameLine();
			
			// Red No button
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.2f, 0.2f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.8f, 0.3f, 0.3f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.6f, 0.1f, 0.1f, 1.0f));
			if (ImGui::Button("No", ImVec2(80, 0)))
			{
				ImGui::CloseCurrentPopup();
			}
			ImGui::PopStyleColor(3);
			
			ImGui::EndPopup();
		}

		// Two-column layout
		ImGui::BeginTable("ConfigTable", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersInnerV);
		ImGui::TableSetupColumn("Configurations", ImGuiTableColumnFlags_WidthStretch, 0.4f);
		ImGui::TableSetupColumn("Actions", ImGuiTableColumnFlags_WidthStretch, 0.6f);
		
		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		
		// ================== LEFT COLUMN ==================
		ImGui::TextUnformatted("Saved Configurations");
		ImGui::Separator();
		
		// Config list
		bool hasSelection = !selectedConfigName.empty() && selectedConfigIndex >= 0;
		
		if (ImGui::BeginListBox("##ConfigList", ImVec2(-1, 200)))
		{
			for (int i = 0; i < configFiles.size(); i++)
			{
				bool isSelected = (selectedConfigIndex == i);
				if (ImGui::Selectable(configFiles[i].c_str(), isSelected))
				{
					selectedConfigIndex = i;
					selectedConfigName = configFiles[i];
				}
			}
			ImGui::EndListBox();
		}
		
		ImGui::Spacing();
		
		// Selected config info
		if (hasSelection)
		{
			ImGui::Text("Selected: %s", selectedConfigName.c_str());
		}
		else
		{
			ImGui::TextDisabled("No configuration selected");
		}
		
		ImGui::Spacing();
		
		// Action buttons
		ImGui::BeginTable("ActionButtons", 3, ImGuiTableFlags_SizingStretchSame);
		ImGui::TableNextColumn();
		
		if (RenderConditionalButton("Load", ImVec2(-1, 0), hasSelection))
		{
			MyConfigSaver::LoadConfig(selectedConfigName + ".cfg");
		}
		
		ImGui::TableNextColumn();
		if (RenderConditionalButton("Save", ImVec2(-1, 0), hasSelection))
		{
			showSaveConfirm = true;
		}
		
		ImGui::TableNextColumn();
		if (RenderConditionalButton("Delete", ImVec2(-1, 0), hasSelection))
		{
			ImGui::OpenPopup("Delete Config");
		}
		
		ImGui::EndTable();
		
		// Delete confirmation popup
		if (ImGui::BeginPopupModal("Delete Config", NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::Text("Delete configuration '%s'?", selectedConfigName.c_str());
			ImGui::Text("This cannot be undone.");
			ImGui::Separator();
			
			if (ImGui::Button("Yes", ImVec2(80, 0)))
			{
				std::string fullPath = configDir + "\\" + selectedConfigName + ".cfg";
				std::remove(fullPath.c_str());
				selectedConfigName = "";
				selectedConfigIndex = -1;
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			if (ImGui::Button("No", ImVec2(80, 0)))
			{
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}
		
		// ================== RIGHT COLUMN ==================
		ImGui::TableNextColumn();
		
		ImGui::TextUnformatted("Create New Configuration");
		ImGui::Separator();
		
		ImGui::SetNextItemWidth(-1);
		ImGui::InputTextWithHint("##NewConfig", "Enter configuration name...", configNameBuffer, sizeof(configNameBuffer));
		
		if (ImGui::Button("Create", ImVec2(-1, 0)) && strlen(configNameBuffer) > 0)
		{
			std::string configFileName = std::string(configNameBuffer) + ".cfg";
			MyConfigSaver::SaveConfig(configFileName);
			memset(configNameBuffer, 0, sizeof(configNameBuffer));
		}
		
		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();
		
		ImGui::TextUnformatted("System");
		ImGui::Separator();
		
		if (ImGui::Button("Reset to Default", ImVec2(-1, 0)))
		{
			ImGui::OpenPopup("Reset Settings");
		}
		
		if (ImGui::Button("Open Config Folder", ImVec2(-1, 0)))
		{
			Gui.OpenWebpage(configDir.c_str());
		}
		
		// Reset confirmation popup
		if (ImGui::BeginPopupModal("Reset Settings", NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::Text("Reset all settings to default values?");
			ImGui::Text("This will not affect saved configurations.");
			ImGui::Separator();
			
			if (ImGui::Button("Yes", ImVec2(80, 0)))
			{
				ConfigMenu::ResetToDefault();
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			if (ImGui::Button("No", ImVec2(80, 0)))
			{
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}
		
		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();
		
		// Info section
		ImGui::TextUnformatted("Info");
		ImGui::Separator();
		ImGui::Text("Configurations: %d", (int)configFiles.size());
		
		ImGui::Text("Folder:");
		// Make the path clickable to change folder
		if (ImGui::Button(configDir.c_str(), ImVec2(-1, 0)))
		{
			ImGui::OpenPopup("Change Folder");
		}
		if (ImGui::IsItemHovered())
		{
			ImGui::SetTooltip("Click to change configuration folder");
		}
		
		// Folder selection popup
		if (ImGui::BeginPopupModal("Change Folder", NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			static char folderPath[512];
			static bool initialized = false;
			
			if (!initialized)
			{
				strcpy_s(folderPath, sizeof(folderPath), configDir.c_str());
				initialized = true;
			}
			
			ImGui::Text("Select new configuration folder:");
			ImGui::Separator();
			ImGui::Spacing();
			
			ImGui::Text("Path:");
			ImGui::SetNextItemWidth(400);
			ImGui::InputText("##FolderPath", folderPath, sizeof(folderPath));
			
			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();
			
			if (ImGui::Button("Apply", ImVec2(80, 0)))
			{
				// Validate and set new path
				std::string newPath = std::string(folderPath);
				if (std::filesystem::exists(newPath) && std::filesystem::is_directory(newPath))
				{
					MenuConfig::path = newPath;
					initialized = false; // Reset for next time
					ImGui::CloseCurrentPopup();
				}
				else
				{
					// Could add error message here
					ImGui::SetTooltip("Invalid folder path!");
				}
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel", ImVec2(80, 0)))
			{
				initialized = false; // Reset for next time
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}
		
		ImGui::EndTable();
	}

	void ResetToDefault() {
		TriggerBot::IgnoreFlash = false;
		TriggerBot::ScopeOnly = true;
		AimControl::IgnoreFlash = false;

		ESPConfig::ArmorBar = false;
		ESPConfig::ShowArmorNum = false;
		ESPConfig::ShowIsScoped = true;
		ESPConfig::AmmoBar = false;
		ESPConfig::OutLine = true;
		ESPConfig::ShowHealthNum = false;
		ESPConfig::FilledColor = ImColor(59, 71, 148, 128);
		ESPConfig::FilledColor2 = ImColor(59, 71, 148, 128);
		ESPConfig::MultiColor = false;
		ESPConfig::BoxFilledVisColor = ImColor(0, 98, 98, 128);
		ESPConfig::FilledVisBox = false;

		MiscCFG::SpecList = false;
		MiscCFG::BombTimerCol = ImColor(131, 137, 150, 255);
		MiscCFG::bmbTimer = true;

		ESPConfig::VisibleColor = ImColor(59, 71, 148, 180);
		ESPConfig::VisibleCheck = false;

		MenuConfig::WindowStyle = 0;
		ESPConfig::ShowPreview = true;
		ESPConfig::ShowHeadBox = true;
		ESPConfig::HeadBoxColor = ImColor(131, 137, 150, 180);

		ESPConfig::ShowDistance = false;
		ESPConfig::ShowBoneESP = true;
		ESPConfig::ShowBoxESP = true;
		ESPConfig::ShowHealthBar = true;
		ESPConfig::ShowWeaponESP = true;
		ESPConfig::ShowEyeRay = false;
		ESPConfig::ShowPlayerName = true;
		ESPConfig::BoxRounding = 5.0f;

		LegitBotConfig::AimBot = true;
		LegitBotConfig::AimToggleMode = false;
		LegitBotConfig::AimPosition = 0;
		LegitBotConfig::AimPositionIndex = BONEINDEX::head;
		LegitBotConfig::HitboxUpdated = false;

		ESPConfig::BoxType = 0;
		ESPConfig::BoneColor = ImColor(131, 137, 150, 180);
		ESPConfig::BoxColor = ImColor(59, 71, 148, 180);
		ESPConfig::EyeRayColor = ImVec4(0, 98, 98, 255);

		MenuConfig::ShowMenu = true;
		MenuConfig::WorkInSpec = true;

		RadarCFG::ShowRadar = false;
		RadarCFG::RadarRange = 125;
		RadarCFG::ShowRadarCrossLine = false;
		RadarCFG::RadarCrossLineColor = ImColor(131, 137, 150, 180);
		RadarCFG::RadarType = 2;
		RadarCFG::RadarPointSizeProportion = 1.f;
		RadarCFG::RadarBgAlpha = 0.1f;
		RadarCFG::Proportion = 2700.f;

		LegitBotConfig::TriggerBot = true;
		LegitBotConfig::TriggerAlways = false;
		TriggerBot::HotKey = 6;
		Text::Trigger::HotKey = KeyMgr::GetKeyName(TriggerBot::HotKey);

		MenuConfig::TeamCheck = true;
		MenuConfig::BypassOBS = false;
		LegitBotConfig::VisibleCheck = true;

		MiscCFG::ShowHeadShootLine = false;
		MiscCFG::HeadShootLineColor = ImColor(131, 137, 150, 200);

		AimControl::HotKey = 1;
		Text::Aimbot::HotKey = KeyMgr::GetKeyName(AimControl::HotKey);
		AimControl::AimFov = 10;
		AimControl::AimFovMin = 0.4f;
		AimControl::Smooth = 5.0f;

		// Reset Precision Mode Settings to default (off)
		AimControl::PrecisionMode = false;
		AimControl::PredictiveAiming = false;
		AimControl::InstantLock = false;
		AimControl::HeadshotOnly = false;
		AimControl::AdaptiveSmoothing = false;
		AimControl::MicroAdjustments = false;
		AimControl::PrecisionThreshold = 0.1f;

		ESPConfig::ShowLineToEnemy = false;
		LegitBotConfig::FovLineSize = 60.0f;
		TriggerBot::TriggerDelay = 10;
		TriggerBot::ShotDuration = 400;

		RCS::RCSBullet = 1;

		RCS::RCSScale = ImVec2(1.4f, 1.4f);
		AimControl::onlyAuto = false;
		AimControl::ScopeOnly = true;
		AimControl::AimBullet = 1;

		LegitBotConfig::FovLineColor = ImVec4(0, 98, 98, 220);
		ESPConfig::LineToEnemyColor = ImVec4(59, 71, 148, 180);

		MiscCFG::WaterMark = true;
		MiscCFG::BunnyHop = false;
		MiscCFG::HitSound = 0;
		MiscCFG::HitMarker = false;
		MiscCFG::SniperCrosshair = true;
		MiscCFG::SniperCrosshairColor = ImColor(32, 178, 170, 255);

		ESPConfig::ESPenabled = true;
		ESPConfig::DrawFov = false;

		LegitBotConfig::FovCircleColor = ImColor(131, 137, 150, 180);

		MenuConfig::MarkWinPos = ImVec2(ImGui::GetIO().DisplaySize.x - 300.0f, 100.f);
		MenuConfig::RadarWinPos = ImVec2(25.f, 25.f);
		MenuConfig::SpecWinPos = ImVec2(10.0f, ImGui::GetIO().DisplaySize.y / 2 - 200);
		MenuConfig::BombWinPos = ImVec2((ImGui::GetIO().DisplaySize.x - 200.0f) / 2.0f, 80.0f);

		MenuConfig::MarkWinChangePos = true;
		MenuConfig::BombWinChangePos = true;
		MenuConfig::RadarWinChangePos = true;
		MenuConfig::SpecWinChangePos = true;

		MenuConfig::HotKey = VK_END;
	}
}