#include "camera.h"

#include "math/math_util.hpp"

namespace rlr {

	Camera::Camera(float aspect_ratio, bool temp)
	{
		if (temp)
		{
			frustum_near = 0.1f;
			frustum_far = 64.f;
			fov = 1.74532925;
		}
		else
		{
			frustum_near = 0.1f;
			frustum_far = 64.f;
			fov = 0.785398163;
		}
		ratio = aspect_ratio;

		SetPerspective();

		pos = fm::vec(0, 0, -4);
		world_up = fm::vec(0, 1, 0);
		euler = fm::vec(0, 0, 0);
		forward = fm::vec(0, 0, -1);

		right = forward.Cross(world_up).Normalized();
		up = right.Cross(forward).Normalized();
	}

	Camera::~Camera()
	{
	}

	void Camera::SetPerspective()
	{
		DirectX::XMMATRIX proj = DirectX::XMMatrixPerspectiveFovRH(fov, ratio, frustum_near, frustum_far);
		DirectX::XMStoreFloat4x4(&projection, proj);
	}

	void Camera::SetOrthographic(float width, float height)
	{
		DirectX::XMMATRIX proj = DirectX::XMMatrixOrthographicRH(width, height, frustum_near, frustum_far);
		DirectX::XMStoreFloat4x4(&projection, proj);
	}

	void Camera::Update()
	{
		fm::vec new_forward(0, 0, 0);

		new_forward[2] = cos(fm::rads(euler[1])) * cos(fm::rads(euler[0]));
		new_forward[1] = sin(fm::rads(euler[0]));
		new_forward[0] = sin(fm::rads(euler[1])) * cos(fm::rads(euler[0]));

		forward = new_forward.Normalized();
		right = forward.Cross(world_up).Normalized();
		up = right.Cross(forward).Normalized();

		fm::vec3 at = pos + forward;
		DirectX::XMMATRIX new_view = DirectX::XMMatrixLookAtRH({ pos.x, pos.y, pos.z }, { at.x, at.y, at.z }, { up.x, up.y, up.z });
		DirectX::XMStoreFloat4x4(&view, new_view);
	}

	void Camera::SetFoV(float fov)
	{
		this->fov = fov;
		SetPerspective();
	}

	void Camera::SetAspectRatio(float ratio)
	{
		this->ratio = ratio;
		SetPerspective();
	}

	void Camera::SetPos(fm::vec pos)
	{
		this->pos = pos;
	}

	void Camera::SetEuler(fm::vec euler)
	{
		this->euler = euler;
	}

	float Camera::GetFoV() const
	{
		return fov;
	}

	float Camera::GetAspectRatio() const
	{
		return ratio;
	}

	fm::vec Camera::GetPos() const
	{
		return pos;
	}

	fm::vec Camera::GetEuler() const
	{
		return euler;
	}

	fm::vec Camera::GetForward() const
	{
		return forward;
	}

	DirectX::XMFLOAT4X4 Camera::GetViewMat() const
	{
		return view;
	}

	DirectX::XMFLOAT4X4  Camera::GetProjMat() const
	{
		return projection;
	}

} /* rlr */
