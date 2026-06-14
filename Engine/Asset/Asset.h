#pragma once
#include <string>

class Asset {
public:
	explicit Asset(uint32_t id) : id_(id) {}

	uint32_t GetID() { return id_; }

protected:
	uint32_t id_;
	std::string name_;
};