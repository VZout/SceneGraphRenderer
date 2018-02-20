#pragma once

#include <iostream>
#include <cmath>

namespace fm {

	namespace storage {
		template<class T, int R>
		struct Vec {
			union {
				T data[R];
			};
		};

		template<class T>
		struct Vec<T, 4> {
		public:
			union {
				T data[4];
				struct { T x, y, z, w; };
				struct { T r, g, b, a; };
			};
		};

		template<class T>
		struct Vec<T, 3> {
		public:
			union {
				T data[3];
				struct { T x, y, z; };
				struct { T r, g, b; };
			};
		};

		template<class T>
		struct Vec<T, 2> {
			union {
				T data[2];
				struct { T x, y; };
			};
		};
	}

	template<class T = float, int R = 3>
	class Vec : public storage::Vec<T, R> {
	public:
		using storage::Vec<T, R>::data;

		Vec(T x, T y) {
			data[0] = x;
			data[1] = y;
		}

		Vec(T x, T y, T z) {
			data[0] = x;
			data[1] = y;
			data[2] = z;
		}

		Vec(T x, T y, T z, T w) {
			data[0] = x;
			data[1] = y;
			data[2] = z;
			data[3] = w;
		}

		Vec() {
			for (auto i = 0; i < R; i++) {
				data[i] = 0;
			}
		}

		Vec(T data[R]) {
			memcpy(this->data, data, sizeof(T) * R);
		}

		~Vec() {

		}

		T& operator[] (int i) {
			return data[i];
		}

		Vec& operator+=(const Vec& rhs) {
			for (auto i = 0; i < R; i++) {
				data[i] += rhs.data[i];
			}
			return *this;
		}

		Vec& operator-=(const Vec& rhs) {
			for (auto i = 0; i < R; i++) {
				data[i] -= rhs.data[i];
			}
			return *this;
		}

		Vec& operator*=(const Vec& rhs) {
			for (auto i = 0; i < R; i++) {
				data[i] *= rhs.data[i];
			}
			return *this;
		}

		Vec operator*(const T& scalar) {
			Vec r = *this;
			for (auto i = 0; i < R; i++) {
				r.data[i] *= scalar;
			}
			return r;
		}

		friend Vec operator+(Vec lhs, const Vec& rhs) {
			lhs += rhs;
			return lhs;
		}

		friend Vec operator-(Vec lhs, const Vec& rhs) {
			lhs -= rhs;
			return lhs;
		}

		friend Vec operator*(Vec lhs, const Vec& rhs) {
			lhs *= rhs;
			return lhs;
		}

		bool operator==(const Vec& other) const {
			for (auto i = 0; i < R; i++) {
				if (data[i] != other.data[i]) {
					return false;
				}
			}
			
			return true;
		}

		bool operator!=(const Vec& other) const {
			for (auto i = 0; i < R; i++) {
				if (data[i] != other.data[i]) {
					return true;
				}
			}

			return false;
		}

		T SqrtLength() {
			T retval = 0;

			for (auto i = 0; i < R; i++) {
				retval += data[i] * data[i];
			}

			return retval;
		}

		T Length() {
			return std::sqrt(SqrtLength());
		}

		Vec Normalized() {
			Vec retval;
			float l = Length();

			for (auto i = 0; i < R; i++) {
				retval.data[i] = data[i] / l;
			}

			return retval;
		}

		T Dot(const Vec& other) const {
			T retval = 0;

			for (auto i = 0; i < R; i++) {
				retval += data[i] * other.data[i];
			}

			return retval;
		}

		// 3D Cross product.
		Vec Cross(const Vec& other) const {
			Vec retval;

			retval.data[0] = data[1] * other.data[2] - data[2] * other.data[1];
			retval.data[1] = data[2] * other.data[0] - data[0] * other.data[2];
			retval.data[2] = data[0] * other.data[1] - data[1] * other.data[0];

			return retval;
		}
	};

	// typedefs
	using vec = Vec<float, 3>;
	using vec2 = Vec<float, 2>;
	using vec3 = Vec<float, 3>;
	using vec4 = Vec<float, 4>;

	using dvec = Vec<double, 3>;
	using dvec2 = Vec<double, 2>;
	using dvec3 = Vec<double, 3>;
	using dvec4 = Vec<double, 4>;

	using ivec = Vec<int, 3>;
	using ivec2 = Vec<int, 2>;
	using ivec3 = Vec<int, 3>;
	using ivec4 = Vec<int, 4>;

} /* fm */
