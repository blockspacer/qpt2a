#include "Cam.h"
#include "cinder/app/AppBasic.h"
#include "../../common/Config.h"

using namespace cinder::app;

Cam::Cam()
{

}

Cam::Cam(int width, int height, float fov, float nearPlane, float farPlane) :
	CameraPersp(width, height, fov, nearPlane, farPlane),
	upDirection(Vec3f::zAxis())
{
}

Cam::~Cam(void)
{
}

Cam & Cam::setPosition(Vec3f & newPosition)
{
	position = newPosition;
	lookAt(position, focus, upDirection);

	return *this;
}

Cam & Cam::setFocus(Vec3f & newFocus)
{
	focus = newFocus;
	lookAt(position, focus, upDirection);

	return *this;
}

Vec3f Cam::screenToWorldPlane(Vec2i screenPoint)
{
	float u = (float)screenPoint.x / (float)getWindowWidth();
	float v = 1.0 - (float)screenPoint.y / (float)getWindowHeight();

	auto ray = generateRay(u, v, getAspectRatio());

	// z-coordinates of both cam position and gazing direction
	// must be non-zero and must have inverse signs
	// so that camera looks onto the world plane (i.e. plane with z == 0) from a distance
	if (ray.getOrigin().z * ray.getDirection().z >= 0)
	{
		throw exception("Cannot calculate the coordinates on the world plane from screen coordinates, because the camera does not look onto the world plane from a distance.");
	}

	// normalize x and y against z (same as stretching direction vector so that z == 1),
	// stretch it against the z-distance of the camera
	// and add camera's x- and y-offsets to the coordinates
	// to get x and y coordinates on the 2D-plane with z == 0, i.e. the world plane
	float x = ray.getDirection().x / abs(ray.getDirection().z) * ray.getOrigin().z + ray.getOrigin().x;
	float y = ray.getDirection().y / abs(ray.getDirection().z) * ray.getOrigin().z + ray.getOrigin().y;

	return Vec3f(x, y, 0);
}