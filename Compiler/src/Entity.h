#pragma once
#include <string>
#include <vector>
#include <unordered_map>

#include "Brush.h"

struct EntityData
{
	std::string classname = "unknown";
	std::unordered_map<std::string, std::string> properties;
	std::vector<BrushData> brushes;
};