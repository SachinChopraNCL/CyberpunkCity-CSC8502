#include "Ship.h"

void Ship::Update(float dt){

	
	if (transform.GetPositionVector().x > 19384) {
		transform = Matrix4::Translation(initialPosition) * Matrix4::Rotation(90, Vector3(0, 1, 0));
	}


	Vector3 movement = Vector3(0, 0, speed) * dt;
	transform = transform * Matrix4::Translation(movement);

	SceneNode::Update(dt);
}