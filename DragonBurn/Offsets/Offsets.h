#pragma once
#include <Windows.h>
#include "..\Core\MemoryMgr.h"
#include <json.hpp>
#include "..\Helpers\WebApi.h"
#include <fstream>
#include <filesystem>
#include <sstream>
#include <ctime>
#include <vector>
#include <thread>
#include <future>
#include <atomic>
#include <chrono>

using json = nlohmann::json;

// Modular offset file structure
struct OffsetFile
{
    std::string fileName;
    std::string url;
    std::string content;
    
    OffsetFile(const std::string& name, const std::string& fileUrl) 
        : fileName(name), url(fileUrl) {}
};

class Offsets
{
public:
	Offsets();
	~Offsets();
	int UpdateOffsets();

	DWORD EntityList;
	DWORD Matrix;
	DWORD ViewAngle;
	DWORD LocalPlayerController;
	DWORD LocalPlayerPawn;
	DWORD GlobalVars;
	DWORD PlantedC4;
	DWORD InputSystem;
	DWORD Sensitivity;

	struct
	{
		DWORD Jump;
		DWORD Left;
		DWORD Right;
		DWORD Attack;
	}Buttons;

	struct
	{
		DWORD IsAlive;
		DWORD PlayerPawn;
		DWORD iszPlayerName;
	}Entity;

	struct
	{
		DWORD BulletServices;
		DWORD CameraServices;
		DWORD pClippingWeapon;

		DWORD isScoped;
		DWORD isDefusing;
		DWORD TotalHit;
		DWORD Pos;
		DWORD CurrentArmor;
		DWORD MaxHealth;
		DWORD CurrentHealth;
		DWORD GameSceneNode;
		DWORD BoneArray;
		DWORD angEyeAngles;
		DWORD vecLastClipCameraPos;
		DWORD iShotsFired;
		DWORD flFlashDuration;
		DWORD aimPunchAngle;
		DWORD aimPunchCache;
		DWORD iIDEntIndex;
		DWORD iTeamNum;
		DWORD iFovStart;
		DWORD fFlags;
		DWORD bSpottedByMask;
		DWORD AbsVelocity;
		DWORD m_bWaitForNoAttack;

	} Pawn;

	struct
	{
		DWORD RealTime;
		DWORD FrameCount;
		DWORD MaxClients;
		DWORD IntervalPerTick;
		DWORD CurrentTime;
		DWORD CurrentTime2;
		DWORD TickCount;
		DWORD IntervalPerTick2;
		DWORD CurrentNetchan;
		DWORD CurrentMap;
		DWORD CurrentMapName;
	} GlobalVar;

	struct
	{
		DWORD m_steamID;
		DWORD m_hPawn;
		DWORD m_pObserverServices;
		DWORD m_hObserverTarget;
		DWORD m_hController;
		DWORD PawnArmor;
		DWORD HasDefuser;
		DWORD HasHelmet;
	} PlayerController;

	struct
	{
		DWORD AttributeManager;
	} EconEntity;

	struct
	{
		DWORD WeaponDataPTR;
		DWORD szName;
		DWORD Clip1;
		DWORD MaxClip;
		DWORD Item;
		DWORD ItemDefinitionIndex;
	} WeaponBaseData;

	struct
	{
		DWORD m_bBeingDefused;
		DWORD m_flDefuseCountDown;
		DWORD m_nBombSite;
	} C4;

private:
	void SetOffsets(const std::string&, const std::string&, const std::string&);
	
	// Update mode handlers
	int HandleOfflineMode(std::vector<OffsetFile>& offsetFiles);
	int HandleCachedMode(std::vector<OffsetFile>& offsetFiles);
	int HandleFreshDownload(std::vector<OffsetFile>& offsetFiles);
	int DownloadAndCache(std::vector<OffsetFile>& offsetFiles);
	void SaveSHAHashes(const std::vector<OffsetFile>& offsetFiles);
	
	// Cache management methods
	std::string GetCacheDirectory();
	bool InitializeCacheDirectory();
	bool SaveOffsetsToCache(const std::vector<OffsetFile>& offsetFiles);
	bool LoadOffsetsFromCache(std::vector<OffsetFile>& offsetFiles);
	bool CacheExists();
	void ClearCache();
	std::string GetCachedSHA(const std::string& fileName);
	bool SaveCachedSHA(const std::string& fileName, const std::string& sha);
	
	// Download and update operations
	bool DownloadOffsetFiles(std::vector<OffsetFile>& offsetFiles);
	void ApplyOffsets(const std::vector<OffsetFile>& offsetFiles);
	bool CheckForUpdates(const std::vector<OffsetFile>& offsetFiles);
	std::string GetGitHubFileSHA(const std::string& fileName);
	
	// Utility methods
	std::string GenerateRandomSuffix();
	std::string GetOrCreateFolderSuffix();
	
	// GitHub token management
	std::string GetGitHubToken();
	bool HasGitHubToken();
	bool ShouldCheckForUpdates();

	// Persistent timing for rate limiting
	void SaveLastCheckTime(std::chrono::steady_clock::time_point time);
	std::chrono::steady_clock::time_point GetLastCheckTime();
};

inline Offsets Offset;