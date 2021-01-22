#pragma once
#include "../nclgl/SceneNode.h"
class Ship : public SceneNode {
public:
	Ship() {}
	~Ship(void) {}
	void Update(float dt) override;
	Vector3 position;
	Vector3 initialPosition = Vector3(-2000.0f, 1500.0f, 8350.0f);
	float speed;
};

