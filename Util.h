#pragma once

#include <assert.h>

namespace sb {
	namespace util {
		template <typename T>
		T normalize(T value, T min, T max) {
			assert(value > min);
			assert(value < max);
			return (value - min) / (max - min);
		}
	}

}