#include "Camera.h"
#include "../utility/Math/MyMath.h"

using namespace MyMath;

Camera::Camera()
	: transform_({1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f})
	, fovY_(0.45f)
	, aspectRatio_(float(1280) / float(720))// 数値ベタ打ちはあとで直す
	, nearClip_(0.1f)
	, farClip_(100.0f)
	, worldMatrix_(MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate))
	, viewMatrix_(Inverse(worldMatrix_))
	, projectionMatrix_(MakePerspectiveFovMatrix(fovY_, aspectRatio_, nearClip_, farClip_))
	, viewProjectionMatrix_(Multiply(viewMatrix_, projectionMatrix_))
{}

void Camera::Update()
{
	worldMatrix_ = MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);
	viewMatrix_ = Inverse(worldMatrix_);
	projectionMatrix_ = MakePerspectiveFovMatrix(fovY_, aspectRatio_, nearClip_, farClip_);
	viewProjectionMatrix_ = Multiply(viewMatrix_, projectionMatrix_);
}
