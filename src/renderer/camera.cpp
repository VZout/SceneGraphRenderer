#include "camera.h"

#define MATH_UTIL_USE_FMATH
#include "math/math_util.hpp"

namespace rlr {

Camera::Camera(float aspect_ratio, bool temp) {
	if (temp) {
		_frustum_near = 0.1f;
		_frustum_far = 64.f;
		_fov = 1.74532925;
	}
	else {
		_frustum_near = 0.1f;
		_frustum_far = 64.f;
		_fov = 0.785398163;
	}
	_ratio = aspect_ratio;

	SetPerspective();

	_pos = fm::vec(0, 0, -4);
	_world_up = fm::vec(0, 1, 0);
	_euler = fm::vec(0, 0, 0);
	_forward = fm::vec(0, 0, -1);

	_right = _forward.Cross(_world_up).Normalized();
	_up = _right.Cross(_forward).Normalized();
}

Camera::~Camera() {
}

void Camera::SetPerspective() {
	DirectX::XMMATRIX proj = DirectX::XMMatrixPerspectiveFovRH(_fov, _ratio, _frustum_near, _frustum_far);
	DirectX::XMStoreFloat4x4(&_projection, proj);
}

void Camera::SetOrthographic() {
	//DirectX::XMMATRIX proj = DirectX::XMMatrixOrthographicRH(1280, 720, _frustum_near, _frustum_far);
	//DirectX::XMStoreFloat4x4(&_projection, proj);
}

void Camera::Update() {
	fm::vec new_forward(0, 0, 0);

	new_forward[2] = cos(vik::rads(_euler[1])) * cos(vik::rads(_euler[0]));
	new_forward[1] = sin(vik::rads(_euler[0]));
	new_forward[0] = sin(vik::rads(_euler[1])) * cos(vik::rads(_euler[0]));

	_forward = new_forward.Normalized();
	_right = _forward.Cross(_world_up).Normalized();
	_up = _right.Cross(_forward).Normalized();


	fm::vec3 at = _pos + _forward;
	DirectX::XMMATRIX view = DirectX::XMMatrixLookAtRH({ _pos.x, _pos.y, _pos.z }, { at.x, at.y, at.z }, { _up.x, _up.y, _up.z });
	DirectX::XMStoreFloat4x4(&_view, view);
}

void Camera::SetFoV(float fov) {
	_fov = fov;
	SetPerspective();
}

void Camera::SetAspectRatio(float ratio) {
	_ratio = ratio;
	SetPerspective();
}

void Camera::SetPos(fm::vec pos) {
	_pos = pos;
}

void Camera::SetEuler(fm::vec euler) {
	_euler = euler;
}

float Camera::GetFoV() {
	return _fov;
}

float Camera::GetAspectRatio() {
	return _ratio;
}

fm::vec Camera::GetPos() {
	return _pos;
}

fm::vec Camera::GetEuler() {
	return _euler;
}

fm::vec Camera::GetForward() {
	return _forward;
}

DirectX::XMFLOAT4X4 Camera::GetViewMat() {
	return _view;
}

DirectX::XMFLOAT4X4  Camera::GetProjMat() {
	return _projection;
}

} /* rlr */
