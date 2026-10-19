#pragma once
#include <string>
#include <unordered_map>

struct EntityData
{
	std::string classname = "unknown";
	std::unordered_map<std::string, std::string> properties;
};