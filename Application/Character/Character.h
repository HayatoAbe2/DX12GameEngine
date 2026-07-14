#pragma once
#include "GameCommon.h"

class Character {
public:
	virtual Transform GetTransform() const = 0;
	virtual Vector3 GetPrePos() const = 0;
protected:
};