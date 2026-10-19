#pragma once

#include <fstream>
#include <iostream>
#include <vector>

namespace SerializerUtils {

	template <typename T>
	inline void Write(std::ofstream& file, const T& data)
	{
		file.write(reinterpret_cast<const char*>(&data), sizeof(T));
	}

	inline void WriteString(std::ofstream& file, const std::string& str)
	{
		uint32_t len = static_cast<uint32_t>(str.size());
		Write(file, len);
		if (len > 0)
			file.write(str.data(), len);
	}

	template <typename T>
	inline void WriteVector(std::ofstream& file, const std::vector<T>& vec)
	{
		uint32_t count = static_cast<uint32_t>(vec.size());
		Write(file, count);
		if (count > 0)
			file.write(reinterpret_cast<const char*>(vec.data()), count * sizeof(T));
	}
}