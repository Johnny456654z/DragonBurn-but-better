#pragma once
#include <string>
#include <Windows.h>
#include <vector>
#include <regex>

namespace Web
{
    inline bool CheckConnection()
    {
        int result = system("ping google.com > nul");
        if (result == 0)
            return true;
        else
            return false;
    }

    inline bool Get(std::string url, std::string& response)
    {
        response = "";
        std::string cmd = "curl -s -X GET " + url;

        std::array<char, 128> buffer;
        std::unique_ptr<FILE, decltype(&_pclose)> pipe(_popen(cmd.c_str(), "r"), _pclose);

        if (!pipe)
        {
            return false;
        }
        while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe.get()) != nullptr)
        {
            response += buffer.data();
        }

        std::regex pattern("\\d{3}:");
        if (std::regex_search(response, pattern))
        {
            return false;
        }

        return true;
    }

    inline bool Post(std::string url, std::string& params)
    {
        return true;
    }

    inline std::string GetLastModified(const std::string& url)
    {
        std::string cmd = "curl -s -I \"" + url + "\"";
        std::string response = "";

        std::array<char, 128> buffer;
        std::unique_ptr<FILE, decltype(&_pclose)> pipe(_popen(cmd.c_str(), "r"), _pclose);

        if (!pipe)
        {
            return "";
        }

        while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe.get()) != nullptr)
        {
            response += buffer.data();
        }

        // Look for Last-Modified header
        std::regex pattern("Last-Modified:\\s*(.+)\\r?\\n", std::regex_constants::icase);
        std::smatch match;
        
        if (std::regex_search(response, match, pattern))
        {
            std::string lastModified = match[1].str();
            // Remove any trailing whitespace/carriage returns
            lastModified.erase(lastModified.find_last_not_of(" \t\r\n") + 1);
            return lastModified;
        }

        return "";
    }
}