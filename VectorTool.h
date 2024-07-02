#pragma once
#include<vector>

class VectorTool
{
public:

	template<class T>
	static bool isValid(const std::vector<T>& vec, int index) noexcept
	{
		return (!vec.empty()) && index >= 0 && index <= vec.size() - 1;
	}

	template<class T>
	static T* safeGet(const std::vector<T>& vec, int index) noexcept
	{
		if (VectorTool::isValid(vec, index)) {
			return const_cast<T*>(&vec[index]);
		}
		return nullptr;
	}

	template<std::equality_comparable T>
	static void erase(std::vector<T>& vec, const T& item) noexcept
	{
		if (vec.empty()) return;
		vec.erase(std::remove_if(vec.begin(), vec.end(),
								 [&](const T& elem) { return elem == item; }), vec.end());
	}

	template<std::equality_comparable T>
	static bool replace(std::vector<T>& vec, const T& oldItem, const T& newItem)
	{
		auto it = std::find(vec.begin(), vec.end(), oldItem);
		if (it == vec.end()) {
			return false;
		}
		std::replace(vec.begin(), vec.end(), oldItem, newItem);
		return true;
	}
};

