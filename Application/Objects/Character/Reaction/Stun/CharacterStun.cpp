#include "CharacterStun.h"
#include "Objects/Character/Character.h"

bool CharacterStun::Update(Character* self) {
	timer_.Update();
	if (timer_.GetState() == TimerState::Finished) {
		self->OnStunEnd();
		return true;
	}
	return false;
}
