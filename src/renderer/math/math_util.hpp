#pragma once

#include <math.h>
#include <random>

#ifdef MATH_UTIL_USE_GLM
#include <glm/glm.hpp>
#endif
#include "vec.hpp"
#include "mat.hpp"

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

#ifdef MATH_UTIL_USE_GLM
	/*inline glm::mat4 perspective(float const& fovy, float const& aspect, float const& near, float const& far) {
		glm::mat4 proj(1);
		proj[0][0] = vik::cot(fovy / 2) / aspect;
		proj[1][1] = vik::cot(fovy / 2);
		proj[2][2] = (near + far) / (near - far);
		proj[3][2] = (2 * near * far) / (near - far);
		proj[2][3] = -1;

		return proj;
	}

	inline glm::mat4 look_at(glm::vec3 const& eye, glm::vec3 const& center, glm::vec3 const& up) {
		glm::mat4 look;
		glm::vec3 x, y, z;

		z = eye - center;
		z = glm::normalize(z);
		y = up;
		x = glm::cross(y, z);
		y = glm::cross(z, x);
		x = glm::normalize(x);
		y = glm::normalize(y);

		look[0][0] = x.x;
		look[1][0] = x.y;
		look[2][0] = x.z;
		look[3][0] = -glm::dot(x, eye);
		look[0][1] = y.x;
		look[1][1] = y.y;
		look[2][1] = y.z;
		look[3][1] = -glm::dot(y, eye);
		look[0][2] = z.x;
		look[1][2] = z.y;
		look[2][2] = z.z;
		look[3][2] = -glm::dot(z, eye);
		look[0][3] = 0;
		look[1][3] = 0;
		look[2][3] = 0;
		look[3][3] = 1.0f;

		return look;
	}

	inline glm::mat4 rotate(glm::mat4 const& mat, float const& angle, glm::vec3 const& axis) {
		float cosA = cos(angle);
		float sinA = sin(angle);
		glm::mat4 rmat;

		rmat[0][0] = cosA + (1 - cosA) * axis.x * axis.x;
		rmat[0][1] = (1 - cosA) * axis.x * axis.y - axis.z * sinA;
		rmat[0][2] = (1 - cosA) * axis.x * axis.z + axis.y * sinA;
		rmat[0][3] = 0.f;

		rmat[1][0] = (1 - cosA) * axis.x * axis.y + axis.z * sinA;
		rmat[1][1] = cosA + (1 - cosA) * axis.y * axis.y;
		rmat[1][2] = (1 - cosA) * axis.y * axis.z - axis.x * sinA;
		rmat[1][3] = 0.f;

		rmat[2][0] = (1 - cosA) * axis.x * axis.z - axis.y * sinA;
		rmat[2][1] = (1 - cosA) * axis.y * axis.z + axis.x * sinA;
		rmat[2][2] = cosA + (1 - cosA) * axis.z * axis.z;
		rmat[2][3] = 0.f;

		rmat[3][0] = 0.f;
		rmat[3][1] = 0.f;
		rmat[3][2] = 0.f;
		rmat[3][3] = 1.f;

		return rmat;
	}*/
#endif

#ifdef MATH_UTIL_USE_FMATH
	template<class M = fm::mat4>
	inline M perspective(float const& fovy, float const& aspect, float const& near, float const& far) {
		M proj = M::Identity();
		proj.m[0][0] = vik::cot(fovy / 2) / aspect;
		proj.m[1][1] = vik::cot(fovy / 2); // remove * -1 for OGL.
		proj.m[2][2] = (near + far) / (near - far);
		proj.m[3][2] = (2 * near * far) / (near - far);
		proj.m[2][3] = -1;

		return proj;
	}

	template<class M = fm::mat4>
	inline M orthographic(float const& left, float const& right, float const& bottom, float const& top, float const& near, float const& far) {
		M proj = M::Identity();

		proj.m[0][0] = 2 / (right - left);
		proj.m[1][1] = 2 / (top - bottom);
		proj.m[2][2] = 2 / (far - near);
		proj.m[2][3] = (far + near) / (far - near);
		proj.m[1][3] = (top + bottom) / (top - bottom);
		proj.m[0][3] = (right + left) / (right - left);

		return proj;
	}

	template<class M = fm::mat4>
	inline M look_at(fm::vec const& eye, fm::vec const& center, fm::vec const& up) {
		M look = M::Identity();
		fm::vec x, y, z;

		z = eye - center;
		z = z.Normalized();
		y = up;
		x = y.Cross(z);
		y = z.Cross(x);
		x = x.Normalized();
		y = y.Normalized();

		look.m[0][0] = x.x;
		look.m[1][0] = x.y;
		look.m[2][0] = x.z;
		look.m[3][0] = -x.Dot(eye);
		look.m[0][1] = y.x;
		look.m[1][1] = y.y;
		look.m[2][1] = y.z;
		look.m[3][1] = -y.Dot(eye);
		look.m[0][2] = z.x;
		look.m[1][2] = z.y;
		look.m[2][2] = z.z;
		look.m[3][2] = -z.Dot(eye);
		look.m[0][3] = 0;
		look.m[1][3] = 0;
		look.m[2][3] = 0;
		look.m[3][3] = 1.0f;

		return look;
	}

	template<class M = fm::mat4>
	inline M rotate(M const& mat, float const& angle, fm::vec const& axis) {
		float cosA = cos(angle);
		float sinA = sin(angle);

		M rmat = mat;

		rmat.m[0][0] = cosA + (1 - cosA) * axis.x * axis.x;
		rmat.m[0][1] = (1 - cosA) * axis.x * axis.y - axis.z * sinA;
		rmat.m[0][2] = (1 - cosA) * axis.x * axis.z + axis.y * sinA;

		rmat.m[1][0] = (1 - cosA) * axis.x * axis.y + axis.z * sinA;
		rmat.m[1][1] = cosA + (1 - cosA) * axis.y * axis.y;
		rmat.m[1][2] = (1 - cosA) * axis.y * axis.z - axis.x * sinA;

		rmat.m[2][0] = (1 - cosA) * axis.x * axis.z - axis.y * sinA;
		rmat.m[2][1] = (1 - cosA) * axis.y * axis.z + axis.x * sinA;
		rmat.m[2][2] = cosA + (1 - cosA) * axis.z * axis.z;

		return rmat;
	}

	template<class M = fm::mat4>
	inline M translate(M const& mat, fm::vec const& pos) {
		M rmat = mat;

		rmat.m[0][3] = pos.x;
		rmat.m[1][3] = pos.y;
		rmat.m[2][3] = pos.z;

		return rmat.Transpose();
	}	

	template<class M = fm::mat4>
	inline M scale(M const& mat, fm::vec const& scale) {
		M rmat = mat;

		rmat.m[0][0] = scale.data[0];
		rmat.m[1][1] = scale.data[1];
		rmat.m[2][2] = scale.data[2];

		return rmat;
	}

	template<class M = fm::mat4>
	inline M scale(M const& mat, float const& scale) {
		M rmat = mat;

		rmat.m[0][0] = scale;
		rmat.m[1][1] = scale;
		rmat.m[2][2] = scale;

		return rmat;
	}
#endif
}

