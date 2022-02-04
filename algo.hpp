#pragma once
#include <vector>

namespace sb
{
	namespace algo {
		template<typename T>
		inline void remove(std::vector<T> & v, const T & item)
		{
			// Be careful: if you omit the v.end() parameter to v.erase, the
			// code will compile but only a single item will be removed.
			v.erase(std::remove(v.begin(), v.end(), item), v.end());
		}

	}
}
