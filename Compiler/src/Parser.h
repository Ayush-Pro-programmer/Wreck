#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "Brush.h"
#include "Entity.h"

namespace MapParser
{

	inline bool ParsePlane(const std::string& line, PlaneData& outPlane)
	{
		char texBuffer[256] = { 0 };
		int matched = sscanf(line.c_str(),
			"( %f %f %f ) ( %f %f %f ) ( %f %f %f ) %256s [ %f %f %f %f ] [ %f %f %f %f ] %f %f %f",
			&outPlane.p1.x, &outPlane.p1.y, &outPlane.p1.z,
			&outPlane.p2.x, &outPlane.p2.y, &outPlane.p2.z,
			&outPlane.p3.x, &outPlane.p3.y, &outPlane.p3.z,
			texBuffer,
			&outPlane.uAxis.x, &outPlane.uAxis.y, &outPlane.uAxis.z, &outPlane.uShift,
			&outPlane.vAxis.x, &outPlane.vAxis.y, &outPlane.vAxis.z, &outPlane.vShift,
			&outPlane.rotation, &outPlane.scaleX, &outPlane.scaleY
		);

		if (matched >= 19)
		{
			outPlane.textureName = std::string(texBuffer);
			outPlane.mathPlane = Plane::FromPoints(outPlane.p1, outPlane.p2, outPlane.p3);
			return true;
		}

		return false;
	}

	inline bool ParseKeyValue(
		const std::string& line, 
		std::string& outKey, std::string& outValue)
	{
		size_t firstQuote1 = line.find('"');
		if (firstQuote1 == std::string::npos) return false;
		size_t firstQuote2 = line.find('"', firstQuote1 + 1);
		if (firstQuote2 == std::string::npos) return false;

		size_t secondQuote1 = line.find('"', firstQuote2 + 1);
		if (secondQuote1 == std::string::npos) return false;
		size_t secondQuote2 = line.find('"', secondQuote1 + 1);
		if (secondQuote2 == std::string::npos) return false;

		outKey = line.substr(firstQuote1 + 1, firstQuote2 - firstQuote1 - 1);
		outValue = line.substr(secondQuote1 + 1, secondQuote2 - secondQuote1 - 1);
		return true;
	}

	inline bool ParseFile(
		const std::string& inputPath,
		std::vector<EntityData>& outEntities)
	{
		std::cout << " - Parsing map file: " << inputPath << "\n";

		std::ifstream file(inputPath);
		if (!file.is_open())
		{
			std::cerr << "Error: Could not open map file!\n";
			return false;
		}

		std::string line;
		BrushData currentBrush;
		EntityData currentEntity;

		bool insideBrushBlock = false;
		bool insideEntityBlock = false;

		while (std::getline(file, line))
		{
			// skip empty lines or comments
			if (line.empty() || line[0] == '/') continue;

			// in valve 220 format, an opening brace defines a block.
			// entity blocks contain key-values (like "classname"), while brush block contains plane lines.
			if (line.find('{') != std::string::npos)
			{
				// if we are already inside an entity and hit another '{', it's a brush block inside entity
				if (insideEntityBlock && !insideBrushBlock)
				{
					insideBrushBlock = true;
					currentBrush.planes.clear();
				}
				else if (!insideEntityBlock)
				{
					insideEntityBlock = true;
					currentEntity = EntityData();
				}
				continue;
			}

			if (line.find('}') != std::string::npos)
			{
				if (insideBrushBlock)
				{
					if (!currentBrush.planes.empty())
						currentEntity.brushes.push_back(currentBrush);
					
					insideBrushBlock = false;
				}
				else if (insideEntityBlock)
				{
					outEntities.push_back(currentEntity);
					insideEntityBlock = false;
				}
				continue;
			}

			if (insideEntityBlock) 
			{
				if (insideBrushBlock)
				{
					PlaneData plane;
					if (ParsePlane(line, plane))
						currentBrush.planes.push_back(plane);
				}
				else
				{
					// Check for key-value pair
					std::string key, value;
					if (ParseKeyValue(line, key, value))
						if (key == "classname")
							currentEntity.classname = value;

					currentEntity.properties[key] = value;
				}
			}
		}

		size_t totalBrushes = 0;
		for (const auto& entity : outEntities)
			totalBrushes += entity.brushes.size();

		std::cout << " - Successfully parsed " << outEntities.size() <<
			" entities (containing " << totalBrushes << " total brushes)\n";
		return true;
	}
}