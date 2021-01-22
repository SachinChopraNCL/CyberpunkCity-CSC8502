#pragma once
#include "../nclgl/SceneNode.h"
class Asteroid : public SceneNode {
public:
	Asteroid(void);
	~Asteroid(void);
	void Update(float dt) override;

	void SetOrbitSpeed(float o) { _orbit_speed = o; }

protected:
	float   _radius; 
	Vector3 _origin = Vector3(-8192.f, 4000.f, -8192.f);
	float   _orbit_speed;
};

