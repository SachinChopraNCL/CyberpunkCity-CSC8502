#include "Asteroid.h"

Asteroid::Asteroid(void) {
	texture = SOIL_load_OGL_texture(TEXTUREDIR"asteroidsurface.JPG", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	bumpTexture = SOIL_load_OGL_texture(TEXTUREDIR"asteroidbump.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	colour = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	SetModelScale(Vector3(300.0f, 300.0f, 300.0f));
	SetBoundingRadius(400.0f);
	_radius = 100.0f;
	shaderRef = 4;
}

Asteroid::~Asteroid(void) {
	
}

void Asteroid::Update(float dt) {
	Vector3 to_origin = Vector3(-_origin.x, -_origin.y, -_origin.z);
	transform = transform* Matrix4::Translation(to_origin);
	transform = transform * Matrix4::Rotation(_orbit_speed * dt, Vector3(0, 1, 0));
	transform = transform * Matrix4::Translation(_origin);

	SceneNode::Update(dt);
}