#pragma once
#include "Engine/SceneObject/Particle/Particle.h"
#include "Engine/SceneObject/Particle/ParticleField/ParticleField.h"
#include "Engine/Asset/Resource/Material/Material.h"
#include <vector>
#include <memory>
#include <Engine/Graphics/GPUData/InstanceGPUData.h>
#include "Engine/SceneObject/SceneObject.h"

class InstancedModel;
class Camera;

class ParticleSystem : public SceneObject {
public:
    ParticleSystem(uint32_t id) : SceneObject(id) {};

    void Initialize(std::unique_ptr<Material> material, int numInstance);
    void InitializeGPUParticle();
    void Emit(const Transform& baseTransform, const Vector3& velocity);
    void Update();
    void UpdateEmitterForGPUParticle(float deltatime);

    void PreDraw(Camera* camera);

    void SetColor(const Vector4& color);
    void SetLifeTime(int lifeTime) { maxLifeTime_ = lifeTime; }

	void AddField(std::unique_ptr<ParticleField> field) {
		fields_.push_back(std::move(field));
	}

    void RemoveField() {
        fields_.clear();
    }

    const Microsoft::WRL::ComPtr<ID3D12Resource>& GetInstanceResource() const { return instanceTransformationResource_; }
    void SetInstanceResource(Microsoft::WRL::ComPtr<ID3D12Resource> resource) {
        instanceTransformationResource_ = resource;
    }
    void SetFreeCounterResource(Microsoft::WRL::ComPtr<ID3D12Resource> resource) {
        freeCounterResource_ = resource;
    }

    // パーティクルデータUAV
    void SetParticleUAVIndex(uint32_t uavIndex) { particleDataUAVIndex_ = uavIndex; }
    uint32_t GetParticleUAVIndex() { return particleDataUAVIndex_; }

    // 空きパーティクルUAV
    void SetFreeCounterUAVIndex(uint32_t uavIndex) { freeCounterUAVIndex_ = uavIndex; }
    uint32_t GetFreeCounterUAVIndex() { return freeCounterUAVIndex_; }

    void SetInstanceTransformData(InstanceGPUData* data) { instanceTransformationData_ = data; }
    void SetParticleData(GPUParticle* data) { particleData_ = data; }

    const D3D12_GPU_DESCRIPTOR_HANDLE& GetParticleSRVHandle() const { return instanceSRVHandleGPU_; }
    void SetSRVHandle(D3D12_GPU_DESCRIPTOR_HANDLE srvHandleGPU) { instanceSRVHandleGPU_ = srvHandleGPU; }

    const D3D12_GPU_VIRTUAL_ADDRESS GetInstanceCBV()const { return instanceTransformationResource_->GetGPUVirtualAddress(); }

    int GetNumInstance() { return int(particles_.size()); }
    Material* GetMaterial() { return material_.get(); }

    const D3D12_RESOURCE_STATES& GetCurrentState() { return resourceStates_; }
    void SetState(const D3D12_RESOURCE_STATES& state) { resourceStates_ = state; }

    void SetEmitterResource(Microsoft::WRL::ComPtr<ID3D12Resource> emitterResource) { emitterResource_ = emitterResource; }
    void SetEmitterData(EmitterSphere* data) { emitterSphereData_ = data; }
    Microsoft::WRL::ComPtr<ID3D12Resource> GetEmitterResource() { return emitterResource_; }
private:
    std::vector<Particle> particles_;
	std::vector<std::unique_ptr<ParticleField>> fields_;
    std::unique_ptr<Material> material_;
    int maxLifeTime_ = 1;

    D3D12_GPU_DESCRIPTOR_HANDLE instanceSRVHandleGPU_{};
    Microsoft::WRL::ComPtr<ID3D12Resource> instanceTransformationResource_ = nullptr;
    InstanceGPUData* instanceTransformationData_ = nullptr;

    Microsoft::WRL::ComPtr<ID3D12Resource> particleDataResource_;
    uint32_t particleDataUAVIndex_;
    GPUParticle* particleData_ = nullptr;

    Microsoft::WRL::ComPtr<ID3D12Resource> freeCounterResource_;
    uint32_t freeCounterUAVIndex_;

    // 現在ResourceState
    D3D12_RESOURCE_STATES resourceStates_ = D3D12_RESOURCE_STATE_COMMON;

    Microsoft::WRL::ComPtr<ID3D12Resource> emitterResource_;
    EmitterSphere* emitterSphereData_;
};
