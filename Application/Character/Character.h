#pragma once
#include "GameCommon.h"

class Character {
public:
	virtual Transform GetTransform() const = 0;

protected:
};