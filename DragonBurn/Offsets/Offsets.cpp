#include "Offsets.h"


// ========================================
// GITHUB TOKEN CONFIGURATION
// ========================================
// To get unlimited GitHub API calls (5000/hour instead of 60/hour):
// 1. Go to: https://github.com/settings/tokens
// 2. Click "Generate new token (classic)"
// 3. Give it a name like "DragonBurn Offsets"
// 4. No scopes needed - just click "Generate token"
// 5. Copy the token and paste it below (keep the quotes)
// 6. Leave empty "" for rate-limited mode (60 requests/hour)

const std::string GITHUB_TOKEN = "";  // <-- Paste your token here

Offsets::Offsets() {}
Offsets::~Offsets() {}

// ========================================
// MAIN UPDATE LOGIC
// ========================================

int Offsets::UpdateOffsets()
{
    // Configure offset files
    std::vector<OffsetFile> offsetFiles = {
        {"offsets.json", "https://raw.githubusercontent.com/a2x/cs2-dumper/main/output/offsets.json"},
        {"buttons.json", "https://raw.githubusercontent.com/a2x/cs2-dumper/main/output/buttons.json"},
        {"client_dll.json", "https://raw.githubusercontent.com/a2x/cs2-dumper/main/output/client_dll.json"}
    };

    // Check internet connection
    if (!Web::CheckConnection())
    {
        return HandleOfflineMode(offsetFiles);
    }

    printf(" - Checking for latest offsets...\n");
    
    // Handle online mode with cache checking
    if (CacheExists())
    {
        return HandleCachedMode(offsetFiles);
    }
    
    // No cache - download fresh
    return HandleFreshDownload(offsetFiles);
}

// ========================================
// UPDATE MODE HANDLERS
// ========================================

int Offsets::HandleOfflineMode(std::vector<OffsetFile>& offsetFiles)
{
    if (CacheExists() && LoadOffsetsFromCache(offsetFiles))
    {
        printf(" - No internet connection - using cached offsets\n");
        ApplyOffsets(offsetFiles);
        return 2; // Success with cached data
    }
    
    printf(" - ERROR: No internet connection and no cached offsets found!\n");
    printf(" - You must run the program online at least once to download offsets.\n");
    return 0; // Error
}

int Offsets::HandleCachedMode(std::vector<OffsetFile>& offsetFiles)
{
    printf(" - Found cached offsets - checking if update needed...\n");
    
    if (!ShouldCheckForUpdates())
    {
        printf(" - Using cached offsets (rate limit protection)\n");
        LoadOffsetsFromCache(offsetFiles);
        ApplyOffsets(offsetFiles);
        return 2;
    }
    
    if (CheckForUpdates(offsetFiles))
    {
        printf(" - New offsets detected! Downloading updates...\n");
        return DownloadAndCache(offsetFiles);
    }
    
    printf(" - Offsets are up to date. Using cached version.\n");
    LoadOffsetsFromCache(offsetFiles);
    ApplyOffsets(offsetFiles);
    return 2;
}

int Offsets::HandleFreshDownload(std::vector<OffsetFile>& offsetFiles)
{
    printf(" - No cache found. Downloading fresh offsets...\n");
    return DownloadAndCache(offsetFiles);
}

int Offsets::DownloadAndCache(std::vector<OffsetFile>& offsetFiles)
{
    if (!DownloadOffsetFiles(offsetFiles))
    {
        printf(" - Failed to download offsets from server\n");
        
        // Try to use cached version as fallback
        if (CacheExists() && LoadOffsetsFromCache(offsetFiles))
        {
            printf(" - Using cached offsets as fallback\n");
            ApplyOffsets(offsetFiles);
            return 2;
        }
        return 1; // Complete failure
    }

    // Save to cache and SHA hashes
    SaveOffsetsToCache(offsetFiles);
    SaveSHAHashes(offsetFiles);
    
    ApplyOffsets(offsetFiles);
    return 2;
}

void Offsets::SaveSHAHashes(const std::vector<OffsetFile>& offsetFiles)
{
    for (const auto& file : offsetFiles)
    {
        std::string sha = GetGitHubFileSHA(file.fileName);
        if (!sha.empty())
        {
            SaveCachedSHA(file.fileName, sha);
        }
    }
}

// ========================================
// OFFSET PARSING & APPLICATION
// ========================================

void Offsets::SetOffsets(const std::string& offsetsData, const std::string& buttonsData, const std::string& client_dllData)
{
    json offsetsJson = json::parse(offsetsData);
    json buttonsJson = json::parse(buttonsData);
    json client_dllJson = json::parse(client_dllData)["client.dll"]["classes"];

    // Core offsets
    EntityList = offsetsJson["client.dll"]["dwEntityList"];
    Matrix = offsetsJson["client.dll"]["dwViewMatrix"];
    ViewAngle = offsetsJson["client.dll"]["dwViewAngles"];
    LocalPlayerController = offsetsJson["client.dll"]["dwLocalPlayerController"];
    LocalPlayerPawn = offsetsJson["client.dll"]["dwLocalPlayerPawn"];
    GlobalVars = offsetsJson["client.dll"]["dwGlobalVars"];
    PlantedC4 = offsetsJson["client.dll"]["dwPlantedC4"];
    InputSystem = offsetsJson["inputsystem.dll"]["dwInputSystem"];
    Sensitivity = offsetsJson["client.dll"]["dwSensitivity"];

    // Button offsets
    Buttons.Attack = buttonsJson["client.dll"]["attack"];
    Buttons.Jump = buttonsJson["client.dll"]["jump"];
    Buttons.Right = buttonsJson["client.dll"]["right"];
    Buttons.Left = buttonsJson["client.dll"]["left"];

    // Entity offsets
    Entity.IsAlive = client_dllJson["CCSPlayerController"]["fields"]["m_bPawnIsAlive"];
    Entity.PlayerPawn = client_dllJson["CCSPlayerController"]["fields"]["m_hPlayerPawn"];
    Entity.iszPlayerName = client_dllJson["CBasePlayerController"]["fields"]["m_iszPlayerName"];

    // Pawn offsets
    Pawn.BulletServices = client_dllJson["C_CSPlayerPawn"]["fields"]["m_pBulletServices"];
    Pawn.CameraServices = client_dllJson["C_BasePlayerPawn"]["fields"]["m_pCameraServices"];
    Pawn.pClippingWeapon = client_dllJson["C_CSPlayerPawnBase"]["fields"]["m_pClippingWeapon"];
    Pawn.isScoped = client_dllJson["C_CSPlayerPawn"]["fields"]["m_bIsScoped"];
    Pawn.isDefusing = client_dllJson["C_CSPlayerPawn"]["fields"]["m_bIsDefusing"];
    Pawn.TotalHit = client_dllJson["CCSPlayer_BulletServices"]["fields"]["m_totalHitsOnServer"];
    Pawn.Pos = client_dllJson["C_BasePlayerPawn"]["fields"]["m_vOldOrigin"];
    Pawn.CurrentArmor = client_dllJson["C_CSPlayerPawn"]["fields"]["m_ArmorValue"];
    Pawn.MaxHealth = client_dllJson["C_BaseEntity"]["fields"]["m_iMaxHealth"];
    Pawn.CurrentHealth = client_dllJson["C_BaseEntity"]["fields"]["m_iHealth"];
    Pawn.GameSceneNode = client_dllJson["C_BaseEntity"]["fields"]["m_pGameSceneNode"];
    Pawn.BoneArray = 0x1F0;
    Pawn.angEyeAngles = client_dllJson["C_CSPlayerPawnBase"]["fields"]["m_angEyeAngles"];
    Pawn.vecLastClipCameraPos = client_dllJson["C_CSPlayerPawnBase"]["fields"]["m_vecLastClipCameraPos"];
    Pawn.iShotsFired = client_dllJson["C_CSPlayerPawn"]["fields"]["m_iShotsFired"];
    Pawn.flFlashDuration = client_dllJson["C_CSPlayerPawnBase"]["fields"]["m_flFlashDuration"];
    Pawn.aimPunchAngle = client_dllJson["C_CSPlayerPawn"]["fields"]["m_aimPunchAngle"];
    Pawn.aimPunchCache = client_dllJson["C_CSPlayerPawn"]["fields"]["m_aimPunchCache"];
    Pawn.iIDEntIndex = client_dllJson["C_CSPlayerPawnBase"]["fields"]["m_iIDEntIndex"];
    Pawn.iTeamNum = client_dllJson["C_BaseEntity"]["fields"]["m_iTeamNum"];
    Pawn.iFovStart = client_dllJson["CCSPlayerBase_CameraServices"]["fields"]["m_iFOVStart"];
    Pawn.fFlags = client_dllJson["C_BaseEntity"]["fields"]["m_fFlags"];
    Pawn.bSpottedByMask = DWORD(client_dllJson["C_CSPlayerPawn"]["fields"]["m_entitySpottedState"]) + 
                         DWORD(client_dllJson["EntitySpottedState_t"]["fields"]["m_bSpottedByMask"]);
    Pawn.AbsVelocity = client_dllJson["C_BaseEntity"]["fields"]["m_vecAbsVelocity"];
    Pawn.m_bWaitForNoAttack = client_dllJson["C_CSPlayerPawn"]["fields"]["m_bWaitForNoAttack"];

    // Global variable offsets (hardcoded)
    GlobalVar.RealTime = 0x00;
    GlobalVar.FrameCount = 0x04;
    GlobalVar.MaxClients = 0x10;
    GlobalVar.IntervalPerTick = 0x14;
    GlobalVar.CurrentTime = 0x2C;
    GlobalVar.CurrentTime2 = 0x30;
    GlobalVar.TickCount = 0x40;
    GlobalVar.IntervalPerTick2 = 0x44;
    GlobalVar.CurrentNetchan = 0x0048;
    GlobalVar.CurrentMap = 0x0180;
    GlobalVar.CurrentMapName = 0x0188;

    // Player controller offsets
    PlayerController.m_steamID = client_dllJson["CBasePlayerController"]["fields"]["m_steamID"];
    PlayerController.m_hPawn = client_dllJson["CBasePlayerController"]["fields"]["m_hPawn"];
    PlayerController.m_pObserverServices = client_dllJson["C_BasePlayerPawn"]["fields"]["m_pObserverServices"];
    PlayerController.m_hObserverTarget = client_dllJson["CPlayer_ObserverServices"]["fields"]["m_hObserverTarget"];
    PlayerController.m_hController = client_dllJson["C_BasePlayerPawn"]["fields"]["m_hController"];
    PlayerController.PawnArmor = client_dllJson["CCSPlayerController"]["fields"]["m_iPawnArmor"];
    PlayerController.HasDefuser = client_dllJson["CCSPlayerController"]["fields"]["m_bPawnHasDefuser"];
    PlayerController.HasHelmet = client_dllJson["CCSPlayerController"]["fields"]["m_bPawnHasHelmet"];

    // Economy entity offsets
    EconEntity.AttributeManager = client_dllJson["C_EconEntity"]["fields"]["m_AttributeManager"];

    // Weapon data offsets
    WeaponBaseData.WeaponDataPTR = DWORD(client_dllJson["C_BaseEntity"]["fields"]["m_nSubclassID"]) + 0x08;
    WeaponBaseData.szName = client_dllJson["CCSWeaponBaseVData"]["fields"]["m_szName"];
    WeaponBaseData.Clip1 = client_dllJson["C_BasePlayerWeapon"]["fields"]["m_iClip1"];
    WeaponBaseData.MaxClip = client_dllJson["CBasePlayerWeaponVData"]["fields"]["m_iMaxClip1"];
    WeaponBaseData.Item = client_dllJson["C_AttributeContainer"]["fields"]["m_Item"];
    WeaponBaseData.ItemDefinitionIndex = client_dllJson["C_EconItemView"]["fields"]["m_iItemDefinitionIndex"];

    // C4 offsets
    C4.m_bBeingDefused = client_dllJson["C_PlantedC4"]["fields"]["m_bBeingDefused"];
    C4.m_flDefuseCountDown = client_dllJson["C_PlantedC4"]["fields"]["m_flDefuseCountDown"];
    C4.m_nBombSite = client_dllJson["C_PlantedC4"]["fields"]["m_nBombSite"];
}

void Offsets::ApplyOffsets(const std::vector<OffsetFile>& offsetFiles)
{
    std::string offsetsData, buttonsData, client_dllData;
    
    for (const auto& file : offsetFiles)
    {
        if (file.fileName == "offsets.json")
            offsetsData = file.content;
        else if (file.fileName == "buttons.json")
            buttonsData = file.content;
        else if (file.fileName == "client_dll.json")
            client_dllData = file.content;
    }
    
    SetOffsets(offsetsData, buttonsData, client_dllData);
}

bool Offsets::DownloadOffsetFiles(std::vector<OffsetFile>& offsetFiles)
{
    printf(" - Downloading offset files...\n");
    
    std::vector<std::future<bool>> futures;
    auto globalStart = std::chrono::high_resolution_clock::now();
    
    // Launch parallel downloads
    for (auto& file : offsetFiles)
    {
        futures.push_back(std::async(std::launch::async, [&file]() -> bool {
            return Web::Get(file.url, file.content);
        }));
    }
    
    // Wait for all downloads
    bool allSuccessful = true;
    for (auto& future : futures)
    {
        if (!future.get())
            allSuccessful = false;
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - globalStart);
    
    if (allSuccessful)
    {
        printf(" - Successfully downloaded all offset files (%lld ms)\n", duration.count());
    }
    else
    {
        printf(" - Failed to download some offset files\n");
    }
    
    return allSuccessful;
}

bool Offsets::CheckForUpdates(const std::vector<OffsetFile>& offsetFiles)
{
    printf(" - Checking for updates...\n");
    
    std::vector<std::future<bool>> futures;
    auto globalStart = std::chrono::high_resolution_clock::now();
    
    // Launch parallel SHA checks
    for (const auto& file : offsetFiles)
    {
        futures.push_back(std::async(std::launch::async, [this, &file]() -> bool {
            std::string currentSHA = GetGitHubFileSHA(file.fileName);
            std::string cachedSHA = GetCachedSHA(file.fileName);
            
            if (currentSHA.empty())
                return false;
            
            return cachedSHA.empty() || currentSHA != cachedSHA;
        }));
    }
    
    // Check if any updates are needed
    bool hasUpdates = false;
    for (auto& future : futures)
    {
        if (future.get())
            hasUpdates = true;
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - globalStart);
    
    if (hasUpdates)
    {
        printf(" - Updates available (%lld ms)\n", duration.count());
    }
    else
    {
        printf(" - All files up to date (%lld ms)\n", duration.count());
    }
    
    return hasUpdates;
}

std::string Offsets::GetGitHubFileSHA(const std::string& fileName)
{
    std::string apiUrl = "https://api.github.com/repos/a2x/cs2-dumper/contents/output/" + fileName;
    std::string response;
    
    if (!Web::Get(apiUrl, response) || response.empty())
        return "";
    
    try
    {
        json apiResponse = json::parse(response);
        
        if (apiResponse.contains("message"))
        {
            // API error (rate limit, etc.) - silently fail
            return "";
        }
        
        if (apiResponse.contains("sha"))
            return apiResponse["sha"];
    }
    catch (const std::exception&)
    {
        // JSON parsing error - silently fail
    }
    
    return "";
}

// ========================================
// CACHE MANAGEMENT
// ========================================

std::string Offsets::GetCacheDirectory()
{
    static std::string cachedDir = "";
    
    if (!cachedDir.empty())
        return cachedDir;
    
    char tempPath[MAX_PATH];
    GetTempPathA(MAX_PATH, tempPath);
    
    std::string folderSuffix = GetOrCreateFolderSuffix();
    cachedDir = std::string(tempPath) + "DB-" + folderSuffix + "\\";
    return cachedDir;
}

bool Offsets::InitializeCacheDirectory()
{
    std::string cacheDir = GetCacheDirectory();
    
    if (!std::filesystem::exists(cacheDir))
    {
        try
        {
            std::filesystem::create_directories(cacheDir);
            return true;
        }
        catch (const std::exception&)
        {
            return false;
        }
    }
    return true;
}

bool Offsets::SaveOffsetsToCache(const std::vector<OffsetFile>& offsetFiles)
{
    if (!InitializeCacheDirectory())
        return false;
    
    std::string cacheDir = GetCacheDirectory();
    
    try
    {
        for (const auto& file : offsetFiles)
        {
            std::ofstream outFile(cacheDir + file.fileName);
            outFile << file.content;
            outFile.close();
        }
        return true;
    }
    catch (const std::exception&)
    {
        return false;
    }
}

bool Offsets::LoadOffsetsFromCache(std::vector<OffsetFile>& offsetFiles)
{
    std::string cacheDir = GetCacheDirectory();
    
    try
    {
        for (auto& file : offsetFiles)
        {
            std::ifstream inFile(cacheDir + file.fileName);
            if (!inFile.is_open()) 
                return false;
            
            file.content = std::string((std::istreambuf_iterator<char>(inFile)), std::istreambuf_iterator<char>());
            inFile.close();
        }
        return true;
    }
    catch (const std::exception&)
    {
        return false;
    }
}

bool Offsets::CacheExists()
{
    std::string cacheDir = GetCacheDirectory();
    return std::filesystem::exists(cacheDir + "offsets.json") &&
           std::filesystem::exists(cacheDir + "buttons.json") &&
           std::filesystem::exists(cacheDir + "client_dll.json");
}

void Offsets::ClearCache()
{
    std::string cacheDir = GetCacheDirectory();
    try
    {
        std::filesystem::remove_all(cacheDir);
    }
    catch (const std::exception&) {}
}

std::string Offsets::GetCachedSHA(const std::string& fileName)
{
    std::string cacheDir = GetCacheDirectory();
    std::string shaFile = cacheDir + fileName + ".sha";
    
    try
    {
        std::ifstream file(shaFile);
        if (!file.is_open()) 
            return "";
        
        std::string sha;
        std::getline(file, sha);
        file.close();
        return sha;
    }
    catch (const std::exception&)
    {
        return "";
    }
}

bool Offsets::SaveCachedSHA(const std::string& fileName, const std::string& sha)
{
    if (!InitializeCacheDirectory())
        return false;
    
    std::string cacheDir = GetCacheDirectory();
    std::string shaFile = cacheDir + fileName + ".sha";
    
    try
    {
        std::ofstream file(shaFile);
        file << sha;
        file.close();
        return true;
    }
    catch (const std::exception&)
    {
        return false;
    }
}

// ========================================
// UTILITY FUNCTIONS
// ========================================

std::string Offsets::GenerateRandomSuffix()
{
    const char charset[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    std::string result;
    result.reserve(32);
    
    srand(static_cast<unsigned int>(time(nullptr)) + static_cast<unsigned int>(GetTickCount()));
    for (int i = 0; i < 32; ++i)
    {
        result += charset[rand() % (sizeof(charset) - 1)];
    }
    return result;
}

std::string Offsets::GetOrCreateFolderSuffix()
{
    static std::string cachedSuffix = "";
    
    if (!cachedSuffix.empty())
        return cachedSuffix;
    
    char tempPath[MAX_PATH];
    GetTempPathA(MAX_PATH, tempPath);
    std::string tempDir = std::string(tempPath);
    
    // Look for existing DB- folders
    try
    {
        for (const auto& entry : std::filesystem::directory_iterator(tempDir))
        {
            if (entry.is_directory())
            {
                std::string folderName = entry.path().filename().string();
                if (folderName.length() == 35 && folderName.substr(0, 3) == "DB-")
                {
                    cachedSuffix = folderName.substr(3);
                    return cachedSuffix;
                }
            }
        }
    }
    catch (const std::exception&) {}
    
    // Generate new folder
    cachedSuffix = GenerateRandomSuffix();
    return cachedSuffix;
}

// ========================================
// GITHUB TOKEN MANAGEMENT
// ========================================

std::string Offsets::GetGitHubToken()
{
    return GITHUB_TOKEN;
}

bool Offsets::HasGitHubToken()
{
    return !GetGitHubToken().empty();
}

bool Offsets::ShouldCheckForUpdates()
{
    if (HasGitHubToken())
    {
        printf(" - GitHub token detected - unlimited API calls available\n");
        return true;
    }
    // Rate limiting for non-token users (persistent across restarts)
    auto lastCheck = GetLastCheckTime();
    auto now = std::chrono::steady_clock::now();
    auto timeSinceLastCheck = std::chrono::duration_cast<std::chrono::minutes>(now - lastCheck);
    if (static_cast<int>(timeSinceLastCheck.count()) < 5)
    {
        printf(" - No GitHub token - rate limiting active\n");
        printf(" - Skipping API check (last check was %d minutes ago, waiting 5 minutes)\n", static_cast<int>(timeSinceLastCheck.count()));
        return false;
    }
    printf(" - No GitHub token - rate limiting active, but 5+ minutes passed\n");
    SaveLastCheckTime(now);
    return true;
}

// Save timestamp to file (1 line, ms since epoch)
void Offsets::SaveLastCheckTime(std::chrono::steady_clock::time_point time)
{
    if (!InitializeCacheDirectory())
        return;
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(time.time_since_epoch()).count();
    std::string cacheDir = GetCacheDirectory();
    std::ofstream file(cacheDir + "last_check.txt", std::ios::trunc);
    if (file.is_open())
        file << ms << std::endl;
}

// Read timestamp from file (1 line, ms since epoch)
std::chrono::steady_clock::time_point Offsets::GetLastCheckTime()
{
    std::string cacheDir = GetCacheDirectory();
    std::ifstream file(cacheDir + "last_check.txt");
    if (!file.is_open())
        return std::chrono::steady_clock::time_point{}; // epoch
    long long ms = 0;
    file >> ms;
    if (ms <= 0)
        return std::chrono::steady_clock::time_point{};
    return std::chrono::steady_clock::time_point(std::chrono::milliseconds(ms));
}