#pragma once
// ゲームシーンで主に使うインクルード
#include <memory>
#include <vector>
#include <string>
#include <list>
#include <numbers>
#include <array>

// 配置物
#include "Engine/Scene/Camera/Camera.h"
#include "Engine/SceneObject/Particle/ParticleSystem/ParticleSystem.h"

// 構造体、関数
#include "Engine/Math/MathUtils.h"
#include "Engine/SceneObject/Transform.h"

// エンジン側との窓口
#include "Engine/Contexts/GameContext/GameContext.h"

// デバッグ
#include "Engine/Graphics/Debug/ImGuiManager/ImGuiManager.h"