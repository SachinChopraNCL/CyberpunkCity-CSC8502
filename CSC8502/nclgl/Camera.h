#include "Matrix4.h"
#include "Vector3.h"
#include "OGLRenderer.h"

class Camera
{
public:
	Camera(void) {
		yaw = 0.0f;
		pitch = 0.0f;
		directions[0] = Vector3(1, 0, 0);
		directions[1] = Vector3(0, 0, 1);
		directions[2] = Vector3(-1, 0, 0);
		directions[3] = Vector3(0, 0, -1);
	}

	Camera(float pitch, float yaw, Vector3 position) {
		this->pitch = pitch;
		this->yaw = yaw;
		this->position = position;
		directions[0] = Vector3(1, 0, 0);
		directions[1] = Vector3(0, 0, 1);
		directions[2] = Vector3(-1, 0, 0);
		directions[3] = Vector3(0, 0, -1);
	}

	~Camera(void) {};

	void UpdateCamera(float dt = 1.0f);

	Matrix4 BuildViewMatrix();

	Vector3 GetPosition() const { return position; }
	void    SetPosition(Vector3 val) { position = val; }


	float   GetYaw() const { return yaw; }
	void    SetYaw(float val) { yaw = val; }


	float   GetPitch() const { return pitch; }
	void    SetPitch(float val) { pitch = val; }

	bool    tracked = true;
	float	camera_time = 0;
	int     index = 0;


protected:
	float yaw;
	float pitch;
	Vector3 position;
	Vector3 directions[4];
};
