#include "Building.h"

Building::Building() : SceneNode(){
	colour = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	SetModelScale(Vector3(200.0f, 200.0f, 200.0f));
	SetBoundingRadius(200.0f);
	bumpTexture = SOIL_load_OGL_texture(TEXTUREDIR"buildingbump.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	shaderRef = 4;
}

Building::~Building(void) {}

void Building::Update(float dt) {
	SceneNode::Update(dt);
}

void Building::Rotate(float degree) {
	transform = transform * Matrix4::Rotation(degree, Vector3(0, 1, 0));
}
