#include "camera_free.h"
#include "coordinatesystem.h"

CAMERA_FREE::CAMERA_FREE(const std::string & name) :
	CAMERA(name),
	offset(direction::Up * 2 - direction::Forward * 8),
	leftright_rotation(0),
	updown_rotation(0)
{
	rotation.LoadIdentity();
}

void CAMERA_FREE::SetOffset(const MATHVECTOR <float, 3> & value)
{
	offset = value;
	if (offset.dot(direction::Forward) < 0.001)
	{
		offset = offset - direction::Forward;
	}
}

void CAMERA_FREE::Reset(const MATHVECTOR <float, 3> & newpos, const QUATERNION <float> &)
{
	leftright_rotation = 0;
	updown_rotation = 0;
	Rotate(0, 0);
	position = newpos + offset;
}

void CAMERA_FREE::Rotate(float up, float left)
{
	updown_rotation += up;
	if (updown_rotation > 1.0) updown_rotation = 1.0;
	if (updown_rotation <-1.0) updown_rotation =-1.0;
	leftright_rotation += left;

	rotation.LoadIdentity();
	rotation.Rotate(updown_rotation, direction::Right);
	rotation.Rotate(leftright_rotation, direction::Up);
}

void CAMERA_FREE::Move(float dx, float dy, float dz)
{
	MATHVECTOR <float, 3> move(dx, dy, dz);
	MATHVECTOR <float, 3> forward = direction::Forward * direction::Forward.dot(move);
	rotation.RotateVector(forward);
	position = position + forward;
}
