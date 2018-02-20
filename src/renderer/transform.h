#pragma once

#include <DirectXMath.h>

#include "math\vec.hpp"

namespace rlr {

	class Transform
	{
	public:
		Transform();
		~Transform();

		void Update();

		bool RequiresUpdate() const;
		DirectX::XMMATRIX GetModel() const;

		void Set(fm::vec3 pos, fm::vec3 rot, fm::vec3 sc);
		void SetPosition(fm::vec3 pos);
		fm::vec3 GetPosition() const;
		void SetRotation(fm::vec3 rot);
		fm::vec3 GetRotation() const;
		void SetScale(fm::vec3 sc);
		fm::vec3 GetScale() const;

	private:
		bool requires_update;

		fm::vec3 position;
		fm::vec3 rotation;
		fm::vec3 scale;

		DirectX::XMMATRIX model_mat;
	};

} /* rlr */
