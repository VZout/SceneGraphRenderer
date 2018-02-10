#pragma once

#include <math.h>
#include <random>

#ifdef MATH_UTIL_USE_GLM
#include <glm/glm.hpp>
#endif
#include "vec.hpp"

namespace vik {
	inline float cot(float val) {
		return(1 / tan(val));
	}

	inline float frandr(float min, float max) {
		return min + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (max - min)));
	}

	template<typename T = float>
	inline T rads(T deg) {
		return deg * 0.0174532925;
	}
}
