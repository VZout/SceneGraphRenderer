#include "transform.h"

namespace rlr
{
	
	Transform::Transform()
		: position(0, 0, 0), rotation(0, 0, 0), scale(1, 1, 1), requires_update(true), parent_changed(true)
	{

	}

	Transform::~Transform()
	{

	}

	void Transform::Update(Transform* parent)
	{
		requires_update = false;
		parent_changed = false;

		DirectX::XMMATRIX translation_mat = DirectX::XMMatrixTranslation(position.x, position.y, position.z);
		DirectX::XMMATRIX rotation_mat = DirectX::XMMatrixRotationRollPitchYaw(rotation.x, rotation.y, rotation.z);
		DirectX::XMMATRIX scale_mat = DirectX::XMMatrixScaling(scale.x, scale.y, scale.z);
		local_model_mat = scale_mat * rotation_mat * translation_mat;

		if (parent) {
			world_model_mat = parent->GetWorldModel() * local_model_mat;
		}
		else {
			world_model_mat = local_model_mat;
		}
	}

	void Transform::UpdateWorldOnly(Transform * parent)
	{	
		if (!parent) throw "Tried to update world only transform without giving a parent.";

		world_model_mat = parent->GetWorldModel() * local_model_mat;
	}

	bool Transform::RequiresUpdate() const
	{
		return requires_update;
	}

	DirectX::XMMATRIX Transform::GetLocalModel() const
	{
		if (requires_update) throw "Transform component requires update.";

		return local_model_mat;
	}

	DirectX::XMMATRIX Transform::GetWorldModel() const
	{
		if (requires_update) throw "Transform component requires update.";

		return world_model_mat;
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
		if (position != pos)
		{
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
		if (rotation != rot)
		{
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
		if (scale != sc)
		{
			requires_update = true;
		}

		scale = sc;
	}

	fm::vec3 Transform::GetScale() const
	{
		return scale;
	}

	void Transform::SetParentChanged()
	{
		parent_changed = true;
	}

	bool Transform::ParentChanged() const
	{
		return parent_changed;
	}

} /* rlr */
