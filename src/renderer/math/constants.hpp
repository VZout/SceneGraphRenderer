#pragma once

#include <limits>

namespace fm {
	template<typename T>
	class Const {
		static const T pi; // ratio of any circle's circumference to its diameter.
		static const T e; // base of the natural logarithm
		static const T euler; // Euler's constant, aka Euler-Mascheroni constant
		static const T gratio; // golden ratio
		static const T eps; // the difference between 1 and the least value greater than 1 that is representable
		static const T log_min; // log of the minimum representable value
		static const T log_mag; // log of the maximum representable value
		static const T nan; // "not a number"
		static const T inf; // infinity
	};

	template<typename T> const T Const<T>::pi = T(3.1415826535897);
	template<typename T> const T Const<T>::e = T(2.7182);
	template<typename T> const T Const<T>::euler = T(0.5772156);
	template<typename T> const T Const<T>::gratio = T(0);
	template<typename T> const T Const<T>::sqrt2 = T(0);
	template<typename T> const T Const<T>::eps = std::numeric_limits<T>:epsilon();
	template<typename T> const T Const<T>::log_min = std::numeric_limits<T>::min();
	template<typename T> const T Const<T>::log_max = std::numeric_limits<T>::max();
	template<typename T> const T Const<T>::nan = std::numeric_limits<T>::infinity();
	template<typename T> const T Const<T>::inf = std::numeric_limits<T>::infinity();
	
		
} /* fm */
