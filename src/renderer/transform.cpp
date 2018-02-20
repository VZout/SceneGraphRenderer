#include "transform.h"

namespace rlr
{
	
	Transform::Transform()
		: position(0, 0, 0), rotation(0, 0, 0), scale(1, 1, 1), requires_update(true)
	{

	}

	Transform::~Transform()
	{

	}

	void Transform::Update()
	{
		DirectX::XMMATRIX translation_mat = DirectX::XMMatrixTranslation(position.x, position.y, position.z);
		DirectX::XMMATRIX rotation_mat = DirectX::XMMatrixRotationRollPitchYaw(rotation.x, rotation.y, rotation.z);
		DirectX::XMMATRIX scale_mat = DirectX::XMMatrixScaling(scale.x, scale.y, scale.z);

		model_mat = scale_mat * rotation_mat * translation_mat;
		
		requires_update = false;
	}

	bool Transform::RequiresUpdate() const
	{
		return requires_update;
	}

	DirectX::XMMATRIX Transform::GetModel() const
	{
		if (requires_update) throw "Transform component requires update.";

		return model_mat;
	}

	void Transform::Set(fm::vec3 pos, fm::vec3 rot, fm::vec3 sc)
	{
		position = pos;
		rotation = rot;
		scale = sc;

		requires_update = true;
	}

	void Transform::SetPosition(fm::vec3 pos)
	{
		if (position != pos) {
			requires_update = true;
		}

		position = pos;
	}

	fm::vec3 Transform::GetPosition() const
	{
		return position;
	}

	void Transform::SetRotation(fm::vec3 rot)
	{
		if (rotation != rot) {
			requires_update = true;
		}

		rotation = rot;
	}

	fm::vec3 Transform::GetRotation() const
	{
		return rotation;
	}

	void Transform::SetScale(fm::vec3 sc)
	{
		if (scale != sc) {
			requires_update = true;
		}

		scale = sc;
	}

	fm::vec3 Transform::GetScale() const
	{
		return scale;
	}

} /* rlr */
