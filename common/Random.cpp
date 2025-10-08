#include "Common.h"
#include <random>
#include <concepts>

static std::mt19937 generator(std::random_device{}());
/*
auto Random::get(auto min, auto max)
{
	if constexpr (std::is_integral_v<decltype(min)> && std::is_integral_v<decltype(max)>) {
		std::uniform_int_distribution<int> distribution(min, max);
		return distribution(generator);
	} else if constexpr (std::is_floating_point_v<decltype(min)> && std::is_floating_point_v<decltype(max)>) {
		std::uniform_real_distribution<double> distribution(min, max);
		return distribution(generator);
	}
	else {
		static_assert(std::is_integral_v<decltype(min)> || std::is_floating_point_v<decltype(min)>,
			"Unsupported type for random number generation. Use integral or floating-point types.");
	}

	return decltype(min){}; // Return a default value if types are unsupported

}
*/
int Random::getInt(int min_v, int max_v)
{
	std::uniform_int_distribution<int> distribution(min_v, max_v);
	return distribution(generator);
}
