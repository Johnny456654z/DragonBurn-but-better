#pragma once
#include "..\Core\Config.h"
#include "..\Game\Entity.h"
#include <vector>
#include <string>
#include <iostream>
#include <map>

namespace SpecList
{
    // Helper function to add spectator to list if not already present
    void AddSpectatorIfNotExists(std::vector<std::string>& spectatorList, const std::string& playerName)
    {
        if (std::find(spectatorList.begin(), spectatorList.end(), playerName) == spectatorList.end())
        {
            spectatorList.push_back(playerName);
        }
    }

    struct SpectatorInfo
    {
        std::string playerName;
        bool isAlive;
        int teamID;
        uintptr_t pawnAddress;
        uintptr_t controllerAddress;
    };

    struct SpectatorData
    {
        std::vector<std::string> mySpectators;           // Who's watching me
        std::string targetName;                          // Who I'm watching (when dead)
        uintptr_t targetPawnAddress;                     // Target's pawn address
        std::vector<std::string> targetSpectators;       // Who's watching my target
        bool isLocalPlayerAlive;
        bool isSpectatingTarget;
    };

    static SpectatorData spectatorData;

    // Helper function to resolve entity handles to addresses
    uintptr_t ResolveEntityHandle(uintptr_t entityList, uint32_t handle)
    {
        if (handle == 0 || entityList == 0)
            return 0;

        uintptr_t listEntry = 0;
        if (!memoryManager.ReadMemory<uintptr_t>(entityList + 0x10 + 8 * ((handle & 0x7FFF) >> 9), listEntry))
            return 0;

        uintptr_t entityAddress = 0;
        if (!memoryManager.ReadMemory<uintptr_t>(listEntry + 0x78 * (handle & 0x1FF), entityAddress))
            return 0;

        return entityAddress;
    }

    // Get who a specific entity is spectating
    uintptr_t GetSpectatorTarget(uintptr_t entityPawnAddress)
    {
        if (entityPawnAddress == 0)
            return 0;

        // Get observer services
        uintptr_t observerServices = 0;
        if (!memoryManager.ReadMemory<uintptr_t>(entityPawnAddress + Offset.PlayerController.m_pObserverServices, observerServices) || observerServices == 0)
            return 0;

        // Get observer target handle
        uint32_t observedTargetHandle = 0;
        if (!memoryManager.ReadMemory<uint32_t>(observerServices + Offset.PlayerController.m_hObserverTarget, observedTargetHandle) || observedTargetHandle == 0)
            return 0;

        // Get entity list
        uintptr_t entityList = 0;
        if (!memoryManager.ReadMemory<uintptr_t>(gGame.GetEntityListAddress(), entityList) || entityList == 0)
            return 0;

        // Resolve target address
        return ResolveEntityHandle(entityList, observedTargetHandle);
    }

    // Get entity name from pawn address
    std::string GetEntityNameFromPawn(uintptr_t pawnAddress, const std::vector<CEntity>& allEntities)
    {
        for (const auto& entity : allEntities)
        {
            if (entity.Pawn.Address == pawnAddress)
                return entity.Controller.PlayerName;
        }
        return "Unknown";
    }

    // Main spectator detection function
    void AnalyzeSpectators(const std::vector<CEntity>& allEntities, CEntity& LocalEntity)
    {
        if (!MiscCFG::SpecList || LocalEntity.Controller.TeamID == 0)
            return;

        // Clear previous data
        spectatorData.mySpectators.clear();
        spectatorData.targetSpectators.clear();
        spectatorData.targetName = "";
        spectatorData.targetPawnAddress = 0;
        spectatorData.isLocalPlayerAlive = LocalEntity.IsAlive();
        spectatorData.isSpectatingTarget = false;

        uintptr_t entityList = 0;
        if (!memoryManager.ReadMemory<uintptr_t>(gGame.GetEntityListAddress(), entityList) || entityList == 0)
            return;

        // If local player is dead, find who they're spectating
        if (!spectatorData.isLocalPlayerAlive && LocalEntity.Pawn.Address != 0)
        {
            spectatorData.targetPawnAddress = GetSpectatorTarget(LocalEntity.Pawn.Address);
            if (spectatorData.targetPawnAddress != 0)
            {
                spectatorData.targetName = GetEntityNameFromPawn(spectatorData.targetPawnAddress, allEntities);
                spectatorData.isSpectatingTarget = true;
            }
        }

        // Analyze all entities to find spectators
        for (const auto& entity : allEntities)
        {
            if (entity.Controller.Address == 0 || entity.Controller.PlayerName.empty())
                continue;

            // Get entity's pawn handle
            uint32_t entityPawnHandle = 0;
            if (!memoryManager.ReadMemory<uint32_t>(entity.Controller.Address + Offset.PlayerController.m_hPawn, entityPawnHandle) || entityPawnHandle == 0)
                continue;

            // Resolve entity's pawn address
            uintptr_t entityPawnAddr = ResolveEntityHandle(entityList, entityPawnHandle);
            if (entityPawnAddr == 0)
                continue;

            // Get who this entity is spectating
            uintptr_t spectatorTarget = GetSpectatorTarget(entityPawnAddr);
            if (spectatorTarget == 0)
                continue;

            // Check if entity is spectating the local player (when alive)
            if (spectatorData.isLocalPlayerAlive && spectatorTarget == LocalEntity.Pawn.Address)
            {
                AddSpectatorIfNotExists(spectatorData.mySpectators, entity.Controller.PlayerName);
            }
            // Check if entity is spectating the same target as local player (when dead)
            else if (!spectatorData.isLocalPlayerAlive && spectatorData.isSpectatingTarget && spectatorTarget == spectatorData.targetPawnAddress)
            {
                AddSpectatorIfNotExists(spectatorData.targetSpectators, entity.Controller.PlayerName);
            }
            
        }

        // Store data in LocalEntity for compatibility
        LocalEntity.Controller.spectators = spectatorData.mySpectators;
    }

    // Enhanced spectator window with different modes
    void SpectatorWindowList(CEntity& LocalEntity)
    {
        if ((!MiscCFG::SpecList || LocalEntity.Controller.TeamID == 0) && !(MiscCFG::SpecList && MenuConfig::ShowMenu))
            return;

        ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar;
        static float fontHeight = ImGui::GetFontSize();
        
        // Calculate window dimensions based on content - reduced width
        float windowWidth = 140.0f;
        float windowHeight = 0.0f;
        
        // Calculate required height more accurately
        float titleBarHeight = ImGui::GetFrameHeight();
        float contentPadding = ImGui::GetStyle().WindowPadding.y * 2;
        float itemSpacing = ImGui::GetStyle().ItemSpacing.y;
        
        if (spectatorData.isLocalPlayerAlive)
        {
            // Title bar + spectators or "No spectators"
            int spectatorCount = spectatorData.mySpectators.empty() ? 1 : spectatorData.mySpectators.size();
            windowHeight = titleBarHeight + contentPadding + (fontHeight + itemSpacing) * spectatorCount;
        }
        else
        {
            if (spectatorData.isSpectatingTarget)
            {
                // Title bar + target name + spectators or "No spectators"
                int spectatorCount = spectatorData.targetSpectators.empty() ? 1 : spectatorData.targetSpectators.size();
                windowHeight = titleBarHeight + contentPadding + (fontHeight + itemSpacing) * (1 + spectatorCount);
            }
            else
            {
                // Title bar + "Not spectating anyone"
                windowHeight = titleBarHeight + contentPadding + (fontHeight + itemSpacing) * 1;
            }
        }
        
        // Add some extra padding to prevent clipping
        windowHeight += 10.0f;
        
        ImGui::SetNextWindowPos(MenuConfig::SpecWinPos, ImGuiCond_Once);
        ImGui::SetNextWindowSize({ windowWidth, windowHeight }, ImGuiCond_Always);
        ImGui::GetStyle().WindowRounding = 8.0f;

        // Simple title - no target name in title bar
        std::string windowTitle = "Spectator Info";

        ImGui::Begin(windowTitle.c_str(), NULL, flags);

        if (MenuConfig::SpecWinChangePos) 
        {
            ImGui::SetWindowPos(windowTitle.c_str(), MenuConfig::SpecWinPos);
            MenuConfig::SpecWinChangePos = false;
        }

        if (spectatorData.isLocalPlayerAlive)
        {
            // Show who's watching me when alive - just the list
            if (spectatorData.mySpectators.empty())
            {
                ImGui::TextColored(ImColor(150, 150, 150, 200), "No spectators");
            }
            else
            {
                for (const auto& spectator : spectatorData.mySpectators)
                {
                    ImGui::TextColored(ImColor(100, 255, 100, 255), spectator.c_str());
                }
            }
        }
        else
        {
            // Show spectator info when dead
            if (spectatorData.isSpectatingTarget)
            {
                // Show target name first in red
                ImGui::TextColored(ImColor(255, 100, 100, 255), spectatorData.targetName.c_str());
                
                // Then show other spectators
                if (!spectatorData.targetSpectators.empty())
                {
                    for (const auto& spectator : spectatorData.targetSpectators)
                    {
                        ImGui::TextColored(ImColor(100, 255, 255, 255), spectator.c_str());
                    }
                }
            }
            else
            {
                ImGui::TextColored(ImColor(150, 150, 150, 200), "Not spectating anyone");
            }
        }

        MenuConfig::SpecWinPos = ImGui::GetWindowPos();
        ImGui::End();
        
        // Clear spectator data for next frame
        LocalEntity.Controller.spectators.clear();
    }

    // Legacy function for compatibility - now calls the new system
    void GetSpectatorList(CEntity Entity, CEntity& LocalEntity)
    {
        // This function is now handled by AnalyzeSpectators
        // Keep for compatibility but functionality moved to new system
    }

    // New function to be called from main loop with all entities
    void ProcessAllSpectators(const std::vector<CEntity>& allEntities, CEntity& LocalEntity)
    {
        AnalyzeSpectators(allEntities, LocalEntity);
    }
}