#pragma once
// ゲームシーンで主に使うインクルード
#include <memory>
#include <vector>
#include <string>
#include <list>

// 配置物
#include "Engine/Scene/Camera/Camera.h"
#include "Engine/Object/Particle/ParticleSystem/ParticleSystem.h"

// 構造体、関数
#include "Engine/Math/MathUtils.h"
#include "Engine/Object/Transform.h"

// エンジン側との窓口
#include "Engine/Contexts/GameContext/GameContext.h"

// デバッグ
#include "Engine/Graphics/Debug/ImGuiManager/ImGuiManager.h"