#pragma once
#define _USE_MATH_DEFINES
#define MAXV 10000e9
#include <math.h>
#include <thread>
#include <chrono>
#include "..\Game\Game.h"
#include "..\Game\Entity.h"
#include "..\Core\Config.h"
#include <iostream>
#include "..\Game\View.h"
#include "..\Features/RCS.h"
#include "TriggerBot.h"

extern "C" {
#include "..\Helpers\Mouse.h"
#include "..\Game\Entity.h"
}


namespace AimControl
{
    inline int HotKey = VK_LBUTTON;
    inline int AimBullet = 1;
    inline bool ScopeOnly = true;
    inline bool IgnoreFlash = false;
    inline float AimFov = 10;
    inline float AimFovMin = 0.4f;
    inline float Smooth = 5.0f;
    inline std::vector<int> HitboxList{ BONEINDEX::head };
    inline bool HasTarget = false;
    inline bool onlyAuto = false;

    // Enhanced precision aimbot settings
    inline bool PrecisionMode = false;          // Enable ultra-precise aiming
    inline bool PredictiveAiming = false;       // Predict enemy movement using real velocity
    inline bool InstantLock = false;            // Instant snap to target (risky)
    inline bool HeadshotOnly = false;           // Force headshot targeting
    inline float PrecisionThreshold = 0.1f;    // Pixel-perfect threshold
    inline bool AdaptiveSmoothing = false;      // Dynamic smoothing based on distance
    inline bool MicroAdjustments = false;       // Sub-pixel adjustments
    inline int MaxAdjustmentSteps = 10;         // Max steps for precision aiming

    void AimBot(const CEntity& Local, Vec3 LocalPos, std::vector<Vec3>& AimPosList, std::vector<CEntity>& EntityList);
    void PrecisionAimBot(const CEntity& Local, Vec3 LocalPos, std::vector<Vec3>& AimPosList, std::vector<CEntity>& EntityList);
    void switchToggle();
    bool CheckAutoMode(const std::string& WeaponName);
    Vec3 PredictEnemyPosition(const CEntity& Enemy, const Vec3& targetPos, float predictionTime);
    float CalculateOptimalSmoothing(float distance, float targetSize);
    bool IsTargetStable(const Vec2& screenPos, float threshold);
}
