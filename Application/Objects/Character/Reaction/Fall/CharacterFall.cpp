#include "CharacterFall.h"
#include "Objects/Character/Character.h"

bool CharacterFall::Update(Character* self) {
	timer_.Update();
	if (timer_.GetState() == TimerState::Finished) {
		Transform t = self->GetTransform();
		t.translate.y = 0;
		self->OnFallEnd();
		return true;
	}

	// 落下
	Transform t = self->GetTransform();
	t.translate -= Vector3{ 0, kCharacterFallSpeed, 0 };
	self->SetTransform(t);
	return false;
}
