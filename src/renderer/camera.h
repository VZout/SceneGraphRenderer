#pragma once

#include "math/vec.hpp"
#include "math/mat.hpp"

#include <DirectXMath.h>

namespace rlr {
class Camera {
protected:
	float _frustum_near;
	float _frustum_far;
	float _fov;
	float _ratio;

	fm::vec _pos;
	fm::vec _euler;

	fm::vec _forward;
	fm::vec _right;
	fm::vec _up;
	fm::vec _world_up;

	DirectX::XMFLOAT4X4 _view;
	DirectX::XMFLOAT4X4 _projection;

public:
	Camera(float aspect_ratio = 16.0f / 9.0f, bool temp = false);
    virtual ~Camera();

	void SetPerspective();
	void SetOrthographic(float width, float height);
    void Update();

	void SetFoV(float fov);
	void SetAspectRatio(float ratio);
	void SetPos(fm::vec pos);
	void SetEuler(fm::vec euler);
	float GetFoV();
	float GetAspectRatio();
	fm::vec GetPos();
	fm::vec GetEuler();
	fm::vec GetForward();
	DirectX::XMFLOAT4X4 GetViewMat();
	DirectX::XMFLOAT4X4 GetProjMat();
	
};

} /* rlr*/
