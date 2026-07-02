#include "ParticleSystem.h"
#include "Engine/Scene/Camera/Camera.h"

void ParticleSystem::Initialize(std::unique_ptr<Material> material, int numInstance) {
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

		particle.transform.rotate.x = camera->transform_.rotate.x;
		particle.transform.rotate.y = camera->transform_.rotate.y;
		transforms.push_back(particle.transform);

		particle.color.w = float(particle.lifeTime) / float(maxLifeTime_);
		colors.push_back(particle.color);
	}
	auto data = material_->GetData();
	data.color = colors[0];

	// WVPMatrixを作る
	for (int i = 0; i < particles_.size(); ++i) {
		Transform& t = transforms[i];

		// カメラのX/Y回転）
		Matrix4x4 camRot = Multiply(MakeRotateXMatrix(camera->transform_.rotate.x),
			MakeRotateYMatrix(camera->transform_.rotate.y));

		// Z回転
		Matrix4x4 rotZ = MakeRotateZMatrix(t.rotate.z);
		Matrix4x4 rotateMatrix = Multiply(rotZ, camRot);

		Matrix4x4 worldMatrix = {};
		worldMatrix.m[0][0] = t.scale.x * rotateMatrix.m[0][0];
		worldMatrix.m[0][1] = t.scale.x * rotateMatrix.m[0][1];
		worldMatrix.m[0][2] = t.scale.x * rotateMatrix.m[0][2];
		worldMatrix.m[1][0] = t.scale.y * rotateMatrix.m[1][0];
		worldMatrix.m[1][1] = t.scale.y * rotateMatrix.m[1][1];
		worldMatrix.m[1][2] = t.scale.y * rotateMatrix.m[1][2];
		worldMatrix.m[2][0] = t.scale.z * rotateMatrix.m[2][0];
		worldMatrix.m[2][1] = t.scale.z * rotateMatrix.m[2][1];
		worldMatrix.m[2][2] = t.scale.z * rotateMatrix.m[2][2];
		worldMatrix.m[3][0] = t.translate.x;
		worldMatrix.m[3][1] = t.translate.y;
		worldMatrix.m[3][2] = t.translate.z;
		worldMatrix.m[3][3] = 1.0f;

		//Matrix4x4 worldMatrix = MakeAffineMatrix(transforms[i]);
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