#pragma once
#include<vector>

class VectorTool
{
public:

	template<class T>
	static bool isValid(const std::vector<T>& vec, int index)
	{
		return !vec.empty() && index >= 0 && index <= vec.size();
	}

	template<class T>
	static T* safeGet(const std::vector<T>& vec, int index)
	{
		if (isValid(vec, index){
			return const_cast<T*>(&vector[index]);
		}
		return nullptr;
	}
};

