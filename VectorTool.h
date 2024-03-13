#pragma once
#include<vector>

class VectorTool
{
public:

	template<class T>
	static bool isValid(const std::vector<T>& vec, int index) noexcept
	{
		return !vec.empty() && index >= 0 && index <= vec.size()-1;
	}

	template<class T>
	static T* safeGet(const std::vector<T>& vec, int index) noexcept
	{
		if (VectorTool::isValid(vec, index)){
			return const_cast<T*>(&vec[index]);
		}
		return nullptr;
	}
};

