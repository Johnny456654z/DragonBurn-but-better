#pragma once

#include "..\OS-ImGui\imgui\imgui.h"
#include <string>
#include <json.hpp>

namespace MyConfigSaver {
    extern void SaveConfig(const std::string& filename, const std::string& author = "");
    extern void LoadConfig(const std::string& filename);

    template <typename T>
    static T ReadData(const nlohmann::json& node, const std::vector<std::string>& keys, const T& defaultValue)
    {
        nlohmann::json currentNode = node;
        
        for (const auto& key : keys)
        {
            if (!currentNode.contains(key) || currentNode[key].is_null())
            {
                return defaultValue;
            }
            currentNode = currentNode[key];
        }
        
        try
        {
            return currentNode.get<T>();
        }
        catch (const nlohmann::json::exception&)
        {
            return defaultValue;
        }
    }
    
    static uint32_t ImColorToUInt32(const ImColor& color)
    {
        constexpr float COLOR_SCALE = 255.0f;
        
        uint32_t r = static_cast<uint32_t>(color.Value.x * COLOR_SCALE);
        uint32_t g = static_cast<uint32_t>(color.Value.y * COLOR_SCALE) << 8;
        uint32_t b = static_cast<uint32_t>(color.Value.z * COLOR_SCALE) << 16;
        uint32_t a = static_cast<uint32_t>(color.Value.w * COLOR_SCALE) << 24;

        return r | g | b | a;
    }

    static ImColor UInt32ToImColor(uint32_t value)
    {
        constexpr float INV_COLOR_SCALE = 1.0f / 255.0f;
        
        ImColor result;
        result.Value.x = static_cast<float>(value & 0xFF) * INV_COLOR_SCALE;
        result.Value.y = static_cast<float>((value >> 8) & 0xFF) * INV_COLOR_SCALE;
        result.Value.z = static_cast<float>((value >> 16) & 0xFF) * INV_COLOR_SCALE;
        result.Value.w = static_cast<float>((value >> 24) & 0xFF) * INV_COLOR_SCALE;
        return result;
    }

    static std::vector<int> LoadVector(const nlohmann::json& node, const std::string& key, const std::vector<int>& defaultValue) 
    {
        if (!node.contains(key) || node[key].is_null() || !node[key].is_array())
        {
            return defaultValue;
        }
        
        try
        {
            std::vector<int> result;
            result.reserve(node[key].size()); // Pre-allocate for better performance
            
            for (const auto& element : node[key])
            {
                if (element.is_number_integer())
                {
                    result.push_back(element.get<int>());
                }
                else
                {
                    // If any element is invalid, return default value
                    return defaultValue;
                }
            }
            return result;
        }
        catch (const nlohmann::json::exception&)
        {
            return defaultValue;
        }
    }
}
