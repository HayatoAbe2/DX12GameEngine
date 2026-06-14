#pragma once

// 貫通
struct Piercing {

};

// 跳弾
struct Ricochet {

};

struct BulletTraits {
	std::optional<Piercing> piercing;
	std::optional<Ricochet> ricochet;
};