#include "ParticleSystem.h"
#include "Engine/Scene/Camera/Camera.h"
#include "Engine/Asset/Model/InstancedModel.h"

void ParticleSystem::Initialize(const ParticleShape& shape, std::unique_ptr<Material> material, int numInstance) {
	particleShape_ = shape;
	material_ = std::move(material);
	particles_.resize(numInstance);
}

void ParticleSystem::Update() {
	for (auto& particle : particles_) {
		if (particle.alive) {
			particle.transform.translate += particle.velocity;

			// フィールドの影響
			for (const auto& field : fields_) {
				field->Update(&particle);
			}

			// 時間
			particle.lifeTime--;
			if (particle.lifeTime < 0) {
				particle.alive = false;
			}
		}
	}
}

void ParticleSystem::PreDraw(Camera* camera) {
	std::vector<Transform> transforms;
	std::vector<Vector4> colors;

	for (auto& particle : particles_) {

		particle.transform.rotate = camera->transform_.rotate;
		transforms.push_back(particle.transform);

		particle.color.w = float(particle.lifeTime) / float(maxLifeTime_);
		colors.push_back(particle.color);
	}

	// WVPMatrixを作る
	for (int i = 0; i < particles_.size(); ++i) {
		Matrix4x4 worldMatrix = MakeAffineMatrix(transforms[i]);
		Matrix4x4 worldViewProjectionMatrix = Multiply(worldMatrix, Multiply(camera->viewMatrix_, camera->projectionMatrix_));
		instanceTransformationData_[i].WVP = worldViewProjectionMatrix;
		instanceTransformationData_[i].World = worldMatrix;
		instanceTransformationData_[i].WorldInverseTranspose = Transpose(Inverse(worldMatrix));
		instanceTransformationData_[i].Color = colors[i];
	}
}

void ParticleSystem::SetColor(const Vector4& color) {
	for (auto& particle : particles_) {
		particle.color = color;
		auto data = material_->GetData();
		data.color = color;
		material_->SetData(data);
	}
}

void ParticleSystem::Emit(const Transform& baseTransform, const Vector3& velocity) {
	for (auto& particle : particles_) {
		if (!particle.alive) {
			particle.alive = true;
			particle.lifeTime = maxLifeTime_;
			particle.transform = baseTransform;
			particle.velocity = velocity;
			break;
		}
	}
}