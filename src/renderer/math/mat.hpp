#pragma once

namespace fm {

	namespace storage {
		template<class T, int R, int C>
		struct Mat {
			union {
				T data[R*C];
				T m[R][C];
			};
		};
	}

	template<class T, int R, int C>
	class Mat : public storage::Mat<T, R, C> {
	public:
		using storage::Mat<T, R, C>::data;
		using storage::Mat<T, R, C>::m;

		Mat() {
			for (auto i = 0; i < R * C; i++) {
				data[i] = 0;
			}
		}

		Mat(T d[R*C]) {
			memcpy(data, d, R * C * sizeof(T));
		}

		~Mat() {

		}

		T& operator[] (int i) {
			return data[i];
		}

		Mat& operator+=(const Mat& rhs) {
			for (auto i = 0; i < R * C; i++) {
				data[i] += rhs.data[i];
			}
			return *this;
		}

		Mat& operator-=(const Mat& rhs) {
			for (auto i = 0; i < R * C; i++) {
				data[i] -= rhs.data[i];
			}
			return *this;
		}

		Mat& operator*=(const Mat& rhs) {
			Mat product;

			for (auto i = 0; i < R; ++i)
				for (auto j = 0; j < R; ++j)
					for (auto k = 0; k < R; ++k)
						product.m[i][j] += m[i][k] * rhs.m[k][j];

			memcpy(data, product.data, R * C * sizeof(T));
			return *this;
		}

		friend Mat operator+(Mat lhs, const Mat& rhs) {
			lhs += rhs;
			return lhs;
		}

		friend Mat operator-(Mat lhs, const Mat& rhs) {
			lhs -= rhs;
			return lhs;
		}

		friend Mat operator*(Mat lhs, const Mat& rhs) {
			lhs *= rhs;
			return lhs;
		}

		Mat Transpose() {
			Mat retval;

			#pragma omp parallel for
			for (int n = 0; n < R * C; n++) {
				int i = n / R;
				int j = n % R;
				retval.data[n] = data[C * j + i];
			}

			return retval;
		}

		std::string ToString() {
			std::string str = "(";
			int cr = 1;
			for (auto i = 0; i < R * C; i++) {
				if (i > 0) {
					str += ", ";
				}
				if (i == R * cr) {
					cr++;
					str += "\n";
				}
				std::cout << data[i];
			}
			str += ")";

			return str;
		}

		static Mat Identity() {
			Mat retval;

			for (auto i = 0; i < R; i++) {
				for (auto j = 0; j < C; j++) {
					if (i == j) {
						retval.m[i][j] = 1;
					}
					else {
						retval.m[i][j] = 0;
					}
				}
			}

			return retval;
		}
	};

	// typedefs
	using mat4 = Mat<float, 4, 4>;
	using mat44 = Mat<float, 4, 4>;

} /* fm */
