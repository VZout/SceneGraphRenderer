#pragma once

#include "math/vec.hpp"

#include <DirectXMath.h>

namespace rlr {
class Camera {
protected:
	float frustum_near;
	float frustum_far;
	float fov;
	float ratio;

	fm::vec pos;
	fm::vec euler;

	fm::vec forward;
	fm::vec right;
	fm::vec up;
	fm::vec world_up;

	DirectX::XMFLOAT4X4 view;
	DirectX::XMFLOAT4X4 projection;

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
	float GetFoV() const;
	float GetAspectRatio() const;
	fm::vec GetPos() const;
	fm::vec GetEuler() const;
	fm::vec GetForward() const;
	DirectX::XMFLOAT4X4 GetViewMat() const;
	DirectX::XMFLOAT4X4 GetProjMat() const;
	
};

} /* rlr*/
