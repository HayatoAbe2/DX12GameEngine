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

    void Initialize(const ParticleShape& shape, std::unique_ptr<Material> material, int numInstance);
    void Emit(const Transform& baseTransform, const Vector3& velocity);
    void Update();

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

    void SetInstanceTransformData(InstanceGPUData* data) { instanceTransformationData_ = data; }

    void AddInstanceTransform() {
        transforms_.push_back({});
    }

    const D3D12_GPU_DESCRIPTOR_HANDLE& GetInstanceSRVHandle() const { return instanceSRVHandleGPU_; }
    void SetSRVHandle(D3D12_GPU_DESCRIPTOR_HANDLE srvHandleGPU) { instanceSRVHandleGPU_ = srvHandleGPU; }

    const D3D12_GPU_VIRTUAL_ADDRESS GetInstanceCBV()const { return instanceTransformationResource_->GetGPUVirtualAddress(); }

    int GetNumInstance() { return int(particles_.size()); }
    Material* GetMaterial() { return material_.get(); }

    ParticleShape GetShape() { return particleShape_; }
private:
    std::vector<Particle> particles_;
	std::vector<std::unique_ptr<ParticleField>> fields_;
    ParticleShape particleShape_;
    std::unique_ptr<Material> material_;
    int maxLifeTime_ = 1;

    D3D12_GPU_DESCRIPTOR_HANDLE instanceSRVHandleGPU_{};
    std::vector<Transform> transforms_{};
    Microsoft::WRL::ComPtr<ID3D12Resource> instanceTransformationResource_ = nullptr;
    InstanceGPUData* instanceTransformationData_ = nullptr;
};
