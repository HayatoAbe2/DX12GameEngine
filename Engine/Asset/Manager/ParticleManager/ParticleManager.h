#pragma once
#include <cstdint>
#include <Engine/SceneObject/Particle/ParticleSystem/ParticleSystem.h>
#include <memory>

class DirectXContext;

class ParticleManager {
public:
	ParticleManager(DirectXContext* dxContext);
	std::unique_ptr<ParticleSystem> CreateParticle(uint32_t size, uint32_t id);

	// 初期化
	void InitializeParticles(ParticleSystem* particleSys, uint32_t size);

private:
	DirectXContext* dxContext_ = nullptr;
};

