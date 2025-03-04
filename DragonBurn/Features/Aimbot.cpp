#include "Aimbot.h"

void AimControl::switchToggle()
{
    LegitBotConfig::AimAlways = !LegitBotConfig::AimAlways;
}

void AimControl::AimBot(const CEntity& Local, Vec3 LocalPos, std::vector<Vec3>& AimPosList)
{
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

    float Yaw, Pitch;
    float Distance, Norm, Length;
    Vec2 Angles{ 0,0 };
    int ScreenCenterX = Gui.Window.Size.x / 2;
    int ScreenCenterY = Gui.Window.Size.y / 2;
    float TargetX = 0.f;
    float TargetY = 0.f;

    Vec2 ScreenPos;

    for (int i = 0; i < ListSize; i++)
    {
        Vec3 OppPos;

        OppPos = AimPosList[i] - LocalPos;

        Distance = sqrt(pow(OppPos.x, 2) + pow(OppPos.y, 2));

        Length = sqrt(Distance * Distance + OppPos.z * OppPos.z);

        if (LegitBotConfig::RCS)
        {
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

        Yaw = atan2f(OppPos.y, OppPos.x) * 57.295779513 - Local.Pawn.ViewAngle.y;
        Pitch = -atan(OppPos.z / Distance) * 57.295779513 - Local.Pawn.ViewAngle.x;
        Norm = sqrt(pow(Yaw, 2) + pow(Pitch, 2));

        if (Norm < BestNorm)
            BestNorm = Norm;

        gGame.View.WorldToScreen(Vec3(AimPosList[i]), ScreenPos);
    }

    if (Norm < AimFov && Norm > AimFovMin)
    {
        HasTarget = true;
        if (ScreenPos.x != ScreenCenterX)
        {
            TargetX = (ScreenPos.x > ScreenCenterX) ? -(ScreenCenterX - ScreenPos.x) : ScreenPos.x - ScreenCenterX;
            TargetX /= Smooth != 0.0f ? Smooth : 1.5f;
            TargetX = (TargetX + ScreenCenterX > ScreenCenterX * 2 || TargetX + ScreenCenterX < 0) ? 0 : TargetX;
        }

        if (ScreenPos.y != 0)
        {
            if (ScreenPos.y != ScreenCenterY)
            {
                TargetY = (ScreenPos.y > ScreenCenterY) ? -(ScreenCenterY - ScreenPos.y) : ScreenPos.y - ScreenCenterY;
                TargetY /= Smooth != 0.0f ? Smooth : 1.5f;
                TargetY = (TargetY + ScreenCenterY > ScreenCenterY * 2 || TargetY + ScreenCenterY < 0) ? 0 : TargetY;
            }
        }

        if (!Smooth)
        {
            mouse_event(MOUSEEVENTF_MOVE, (DWORD)(TargetX), (DWORD)(TargetY), NULL, NULL);
            return;
        }

        // Dynamic AimSmooth based on distance
        float DistanceRatio = Norm / AimFov; // Calculate the distance ratio
        float SpeedFactor = 1.0f + (1.0f - DistanceRatio); // Determine the speed factor based on the distance ratio
        TargetX /= (Smooth * SpeedFactor);
        TargetY /= (Smooth * SpeedFactor);

        if (ScreenPos.x != ScreenCenterX)
        {
            TargetX = (ScreenPos.x > ScreenCenterX) ? -(ScreenCenterX - ScreenPos.x) : ScreenPos.x - ScreenCenterX;
            TargetX /= Smooth != 0.0f ? Smooth : 1.5f;
            TargetX = (TargetX + ScreenCenterX > ScreenCenterX * 2 || TargetX + ScreenCenterX < 0) ? 0 : TargetX;
        }

        if (ScreenPos.y != 0)
        {
            if (ScreenPos.y != ScreenCenterY)
            {
                TargetY = (ScreenPos.y > ScreenCenterY) ? -(ScreenCenterY - ScreenPos.y) : ScreenPos.y - ScreenCenterY;
                TargetY /= Smooth != 0.0f ? Smooth : 1.5f;
                TargetY = (TargetY + ScreenCenterY > ScreenCenterY * 2 || TargetY + ScreenCenterY < 0) ? 0 : TargetY;
            }
        }

        mouse_event(MOUSEEVENTF_MOVE, TargetX, TargetY, NULL, NULL);

        int FrameWait = round(1000000.0f / MenuConfig::RenderFPS);
        std::this_thread::sleep_for(std::chrono::microseconds(FrameWait));
    }
    else
        HasTarget = false;
}

bool AimControl::CheckAutoMode(const std::string& WeaponName)
{
    if (WeaponName == "deagle" || WeaponName == "elite" || WeaponName == "fiveseven" || WeaponName == "glock" || WeaponName == "awp" || WeaponName == "xm1014" || WeaponName == "mag7" || WeaponName == "sawedoff" || WeaponName == "tec9" || WeaponName == "zeus" || WeaponName == "p2000" || WeaponName == "nova" || WeaponName == "p250" || WeaponName == "ssg08" || WeaponName == "usp" || WeaponName == "revolver")
        return false;
    else
        return true;
}