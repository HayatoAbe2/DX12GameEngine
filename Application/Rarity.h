#pragma once
#include <Externals/nlohmann/json.hpp>

enum Rarity {
	Common,
	Rare,
	Epic,
	Legendary,
};

NLOHMANN_JSON_SERIALIZE_ENUM(
	Rarity,
	{
		{Rarity::Common, "Common"},
		{Rarity::Rare, "Rare"},
		{Rarity::Epic, "Epic"},
		{Rarity::Legendary, "Legendary"},
	}
	)