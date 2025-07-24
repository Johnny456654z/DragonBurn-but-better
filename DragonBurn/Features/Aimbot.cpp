#include "Aimbot.h"

void AimControl::switchToggle()
{
    LegitBotConfig::AimAlways = !LegitBotConfig::AimAlways;
}

void AimControl::AimBot(const CEntity& Local, Vec3 LocalPos, std::vector<Vec3>& AimPosList, std::vector<CEntity>& EntityList)
{
    // Use precision aimbot if enabled
    if (PrecisionMode)
    {
        PrecisionAimBot(Local, LocalPos, AimPosList, EntityList);
        return;
    }

    // Original aimbot code as fallback
    if (MenuConfig::ShowMenu)
        return;

    std::string curWeapon = TriggerBot::GetWeapon(Local);
    if (!TriggerBot::CheckWeapon(curWeapon))
        return;

    if (onlyAuto && !CheckAutoMode(curWeapon))
        return;

    if (Local.Pawn.ShotsFired <= AimBullet - 1 && AimBullet != 0)
    {
        HasTarget = false;
        return;
    }

    if (AimControl::ScopeOnly)
    {
        bool isScoped;
        memoryManager.ReadMemory<bool>(Local.Pawn.Address + Offset.Pawn.isScoped, isScoped);
        if (!isScoped and TriggerBot::CheckScopeWeapon(curWeapon))
        {
            HasTarget = false;
            return;
        }
    }

    if (!IgnoreFlash && Local.Pawn.FlashDuration > 0.f)
        return;

    int ListSize = AimPosList.size();
    float BestNorm = MAXV;
    Vec2 BestScreenPos;

    for (int i = 0; i < ListSize; i++)
    {
        Vec3 OppPos = AimPosList[i] - LocalPos;
        float Distance = sqrt(pow(OppPos.x, 2) + pow(OppPos.y, 2));

        if (LegitBotConfig::RCS)
        {
            Vec2 Angles{ 0,0 };
            RCS::UpdateAngles(Local, Angles);
            float rad = Angles.x * RCS::RCSScale.x / 360.f * M_PI;
            float si = sinf(rad);
            float co = cosf(rad);

            float z = OppPos.z * co + Distance * si;
            float d = (Distance * co - OppPos.z * si) / Distance;

            rad = -Angles.y * RCS::RCSScale.y / 360.f * M_PI;
            si = sinf(rad);
            co = cosf(rad);

            float x = (OppPos.x * co - OppPos.y * si) * d;
            float y = (OppPos.x * si + OppPos.y * co) * d;

            OppPos = Vec3{ x, y, z };
            AimPosList[i] = LocalPos + OppPos;
        }

        float Yaw = atan2f(OppPos.y, OppPos.x) * 57.295779513 - Local.Pawn.ViewAngle.y;
        float Pitch = -atan(OppPos.z / Distance) * 57.295779513 - Local.Pawn.ViewAngle.x;
        float Norm = sqrt(pow(Yaw, 2) + pow(Pitch, 2));

        if (Norm < BestNorm)
        {
            BestNorm = Norm;
            gGame.View.WorldToScreen(Vec3(AimPosList[i]), BestScreenPos);
        }
    }

    if (BestNorm < AimFov && BestNorm > AimFovMin)
    {
        HasTarget = true;
        int ScreenCenterX = Gui.Window.Size.x / 2;
        int ScreenCenterY = Gui.Window.Size.y / 2;
        
        float TargetX = BestScreenPos.x - ScreenCenterX;
        float TargetY = BestScreenPos.y - ScreenCenterY;

        if (InstantLock)
        {
            mouse_event(MOUSEEVENTF_MOVE, (DWORD)TargetX, (DWORD)TargetY, NULL, NULL);
            return;
        }

        float effectiveSmooth = Smooth != 0.0f ? Smooth : 1.5f;
        TargetX /= effectiveSmooth;
        TargetY /= effectiveSmooth;

        mouse_event(MOUSEEVENTF_MOVE, (DWORD)TargetX, (DWORD)TargetY, NULL, NULL);

        int FrameWait = round(1000000.0f / MenuConfig::RenderFPS);
        std::this_thread::sleep_for(std::chrono::microseconds(FrameWait));
    }
    else
        HasTarget = false;
}

void AimControl::PrecisionAimBot(const CEntity& Local, Vec3 LocalPos, std::vector<Vec3>& AimPosList, std::vector<CEntity>& EntityList)
{
    // Early exit conditions
    if (MenuConfig::ShowMenu)
        return;

    std::string curWeapon = TriggerBot::GetWeapon(Local);
    if (!TriggerBot::CheckWeapon(curWeapon))
        return;

    if (onlyAuto && !CheckAutoMode(curWeapon))
        return;

    if (Local.Pawn.ShotsFired <= AimBullet - 1 && AimBullet != 0)
    {
        HasTarget = false;
        return;
    }

    if (AimControl::ScopeOnly)
    {
        bool isScoped;
        memoryManager.ReadMemory<bool>(Local.Pawn.Address + Offset.Pawn.isScoped, isScoped);
        if (!isScoped && TriggerBot::CheckScopeWeapon(curWeapon))
        {
            HasTarget = false;
            return;
        }
    }

    if (!IgnoreFlash && Local.Pawn.FlashDuration > 0.f)
        return;

    if (AimPosList.empty())
    {
        HasTarget = false;
        return;
    }

    // Force headshot targeting if enabled
    if (HeadshotOnly)
    {
        HitboxList.clear();
        HitboxList.push_back(BONEINDEX::head);
    }

    const int ScreenCenterX = Gui.Window.Size.x / 2;
    const int ScreenCenterY = Gui.Window.Size.y / 2;

    // Find the best target with enhanced precision
    float bestNorm = MAXV;
    Vec3 bestAimPos;
    Vec2 bestScreenPos;
    int bestTargetIndex = -1;

    for (int i = 0; i < AimPosList.size(); i++)
    {
        Vec3 targetPos = AimPosList[i];
        
        // Apply robust movement prediction if enabled
        if (PredictiveAiming && i < EntityList.size())
        {
            // Get real server tick interval for accurate prediction
            float serverTickInterval = 0.0078125f; // Default 128 tick
            DWORD64 globalVarsAddress = gGame.GetGlobalVarsAddress();
            if (globalVarsAddress != 0)
                memoryManager.ReadMemory(globalVarsAddress + Offset.GlobalVar.IntervalPerTick, serverTickInterval);

            // Clamp tick interval to avoid extreme values
            if (serverTickInterval < 0.001f || serverTickInterval > 0.1f)
                serverTickInterval = 0.0078125f;

            // Prediction time: 1-2 server ticks
            float predictionTime = std::clamp(serverTickInterval * 1.5f, 0.005f, 0.03f);

            // Use helper for prediction
            const CEntity& enemy = EntityList[i];
            targetPos = AimControl::PredictEnemyPosition(enemy, targetPos, predictionTime);
        }

        Vec3 diff = targetPos - LocalPos;
        float horizDist = sqrtf(diff.x * diff.x + diff.y * diff.y);

        // Apply RCS compensation if enabled
        if (LegitBotConfig::RCS)
        {
            Vec2 rcsAngles{ 0, 0 };
            RCS::UpdateAngles(Local, rcsAngles);
            
            float rad = rcsAngles.x * RCS::RCSScale.x / 360.f * M_PI;
            float si = sinf(rad), co = cosf(rad);
            float z = diff.z * co + horizDist * si;
            float d = (horizDist * co - diff.z * si) / (horizDist > 0.001f ? horizDist : 0.001f);
            
            rad = -rcsAngles.y * RCS::RCSScale.y / 360.f * M_PI;
            si = sinf(rad); co = cosf(rad);
            float x = (diff.x * co - diff.y * si) * d;
            float y = (diff.x * si + diff.y * co) * d;
            
            diff = Vec3{ x, y, z };
            targetPos = LocalPos + diff;
        }

        // Calculate aim angles
        float targetYaw = atan2f(diff.y, diff.x) * 57.29578f - Local.Pawn.ViewAngle.y;
        float targetPitch = -atan2f(diff.z, horizDist) * 57.29578f - Local.Pawn.ViewAngle.x;
        float norm = sqrtf(targetYaw * targetYaw + targetPitch * targetPitch);

        // Convert to screen coordinates
        Vec2 screenPos;
        if (!gGame.View.WorldToScreen(targetPos, screenPos))
            continue;

        // Select best target based on multiple criteria
        float distanceFromCenter = sqrtf(pow(screenPos.x - ScreenCenterX, 2) + pow(screenPos.y - ScreenCenterY, 2));
        float combinedScore = norm + (distanceFromCenter * 0.01f); // Prioritize angle over screen distance

        if (combinedScore < bestNorm)
        {
            bestNorm = combinedScore;
            bestAimPos = targetPos;
            bestScreenPos = screenPos;
            bestTargetIndex = i;
        }
    }

    // Check if target is within FOV
    if (bestNorm >= AimFov || bestNorm <= AimFovMin || bestTargetIndex == -1)
    {
        HasTarget = false;
        return;
    }

    HasTarget = true;

    // Calculate precise movement
    float offsetX = bestScreenPos.x - ScreenCenterX;
    float offsetY = bestScreenPos.y - ScreenCenterY;

    // Check if we're already close enough (pixel-perfect threshold)
    float distanceToTarget = sqrtf(offsetX * offsetX + offsetY * offsetY);
    if (distanceToTarget <= PrecisionThreshold)
    {
        // We're already on target, no movement needed
        return;
    }

    // Instant lock mode for maximum accuracy (risky for detection)
    if (InstantLock)
    {
        mouse_event(MOUSEEVENTF_MOVE, (DWORD)offsetX, (DWORD)offsetY, NULL, NULL);
        return;
    }

    // Adaptive smoothing based on distance and target size
    float optimalSmoothing = CalculateOptimalSmoothing(distanceToTarget, 10.0f); // Assume 10px head size
    
    if (AdaptiveSmoothing)
    {
        // Closer targets need more smoothing, distant targets need less
        float distanceRatio = distanceToTarget / 100.0f; // Normalize to 100px
        optimalSmoothing = Smooth * (0.5f + distanceRatio * 0.5f);
        optimalSmoothing = max(0.1f, min(optimalSmoothing, 10.0f)); // Clamp between 0.1 and 10
    }
    else
    {
        optimalSmoothing = Smooth != 0.0f ? Smooth : 1.5f;
    }

    // Micro-adjustments for sub-pixel precision
    if (MicroAdjustments && distanceToTarget < 5.0f)
    {
        // Use smaller, more precise movements when very close to target
        optimalSmoothing *= 2.0f; // Double smoothing for micro-adjustments
        
        // Limit movement to prevent overshooting
        float maxMove = 2.0f;
        offsetX = max(-maxMove, min(maxMove, offsetX / optimalSmoothing));
        offsetY = max(-maxMove, min(maxMove, offsetY / optimalSmoothing));
    }
    else
    {
        // Normal movement calculation
        offsetX /= optimalSmoothing;
        offsetY /= optimalSmoothing;
    }

    // Ensure we don't move off-screen
    if ((offsetX + ScreenCenterX) < 0 || (offsetX + ScreenCenterX) > (2 * ScreenCenterX))
        offsetX = 0;
    if ((offsetY + ScreenCenterY) < 0 || (offsetY + ScreenCenterY) > (2 * ScreenCenterY))
        offsetY = 0;

    // Execute mouse movement
    mouse_event(MOUSEEVENTF_MOVE, (DWORD)offsetX, (DWORD)offsetY, NULL, NULL);

    // Adaptive frame timing for precision
    int frameWait;
    if (distanceToTarget < 10.0f)
    {
        // Slower updates when close to target for stability
        frameWait = round(1000000.0f / (MenuConfig::RenderFPS * 0.5f));
    }
    else
    {
        // Normal update rate
        frameWait = round(1000000.0f / MenuConfig::RenderFPS);
    }
    
    std::this_thread::sleep_for(std::chrono::microseconds(frameWait));
}

Vec3 AimControl::PredictEnemyPosition(const CEntity& Enemy, const Vec3& targetPos, float predictionTime)
{
    // Use real enemy velocity for accurate prediction
    Vec3 enemyVelocity = Enemy.Pawn.Velocity;
    Vec3 predictedPos = targetPos;
    
    // Predict where the enemy will be based on their current velocity
    predictedPos.x += enemyVelocity.x * predictionTime;
    predictedPos.y += enemyVelocity.y * predictionTime;
    predictedPos.z += enemyVelocity.z * predictionTime;
    
    return predictedPos;
}

float AimControl::CalculateOptimalSmoothing(float distance, float targetSize)
{
    // Calculate optimal smoothing based on distance to target and target size
    float baseSmoothness = Smooth != 0.0f ? Smooth : 1.5f;
    
    // Closer targets need more smoothing to prevent overshooting
    float distanceFactor = distance / targetSize;
    
    if (distanceFactor < 1.0f)
    {
        // Very close - increase smoothing significantly
        return baseSmoothness * (2.0f + (1.0f - distanceFactor));
    }
    else if (distanceFactor < 5.0f)
    {
        // Medium distance - moderate smoothing
        return baseSmoothness * (1.0f + (distanceFactor * 0.1f));
    }
    else
    {
        // Far distance - reduce smoothing for faster movement
        return baseSmoothness * 0.8f;
    }
}

bool AimControl::IsTargetStable(const Vec2& screenPos, float threshold)
{
    // Check if target position is stable (not moving rapidly)
    static Vec2 lastPos = screenPos;
    static auto lastTime = std::chrono::high_resolution_clock::now();
    
    auto currentTime = std::chrono::high_resolution_clock::now();
    auto timeDiff = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastTime).count();
    
    if (timeDiff > 100) // Check every 100ms
    {
        float movement = sqrtf(pow(screenPos.x - lastPos.x, 2) + pow(screenPos.y - lastPos.y, 2));
        lastPos = screenPos;
        lastTime = currentTime;
        
        return movement < threshold;
    }
    
    return true; // Assume stable if not enough time has passed
}

bool AimControl::CheckAutoMode(const std::string& WeaponName)
{
    if (WeaponName == "deagle" || WeaponName == "elite" || WeaponName == "fiveseven" || WeaponName == "glock" || WeaponName == "awp" || WeaponName == "xm1014" || WeaponName == "mag7" || WeaponName == "sawedoff" || WeaponName == "tec9" || WeaponName == "zeus" || WeaponName == "p2000" || WeaponName == "nova" || WeaponName == "p250" || WeaponName == "ssg08" || WeaponName == "usp" || WeaponName == "revolver")
        return false;
    else
        return true;
}