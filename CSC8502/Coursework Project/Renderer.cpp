#include "Renderer.h"
#include <algorithm>
#define ITERATIONS 14
#define NUMASTEROIDS 10
#define NUMLIGHTS 20
#define POSTPASSES 10
#define SHADOWSIZE 1024
Renderer::Renderer(Window &parent) : OGLRenderer(parent)	{

	InitFBO();
	InitWorld();

	InitLight();
	_root = new SceneNode();
	LoadShaderList();
	_root->AddChild(new Rain());
	GenerateObjects();
	SetOPENGLFunc();
	_camera = new Camera();
	_camera->SetPosition(Vector3(0, 6000.0f, 0));
	_camera->tracked = true;
	_water_rotate = 0.0f;
	_water_cycle = 0.0f;
	init = true;
}

Renderer::~Renderer(void) {
	delete _root;
	delete _camera;
}

void Renderer::SetOPENGLFunc() {
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glEnable(GL_CULL_FACE);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void Renderer::LoadShaderList() {
	/*Blur Shader*/
	Shader* default_shader = new Shader("DefaultVertex.glsl", "DefaultFragment.glsl");
	if (!default_shader->LoadSuccess()) { return; }
	_shader_list.push_back(default_shader);

	/*Skybox Shader*/
	Shader* sky_box_shader = new Shader("SkyBoxVertex.glsl", "SkyBoxFragment.glsl");
	if (!sky_box_shader->LoadSuccess()) { return; }
	_shader_list.push_back(sky_box_shader);

	/*Heightmap Shader*/
	Shader* height_map_shader = new Shader("TexturedVertex.glsl", "TexturedFragment.glsl");
	if (!height_map_shader->LoadSuccess()) { return; }
	_shader_list.push_back(height_map_shader);

	/*Water Shader*/
	Shader* water_shader = new Shader("BumpVertex.glsl", "ReflectFragment.glsl");
	if (!water_shader->LoadSuccess()) { return; }
	_shader_list.push_back(water_shader);

	/* Lighting Shader */
	Shader* light_shader = new Shader("BumpVertex.glsl", "BumpFragment.glsl");
	if (!light_shader->LoadSuccess()) { return; }
	_shader_list.push_back(light_shader);
	
	/* Buffer Shader */
	Shader* scene_shader = new Shader("BumpVertex.glsl", "BufferFragment.glsl");
	if (!scene_shader->LoadSuccess()) { return; }
	_shader_list.push_back(scene_shader);

	Shader* point_light_shader = new Shader("PointLightVertex.glsl", "PointLightFragment.glsl");
	if (!point_light_shader->LoadSuccess()) { return; }
	_shader_list.push_back(point_light_shader);

	Shader* combine_shader = new Shader("CombineVertex.glsl", "CombineFragment.glsl");
	if (!combine_shader->LoadSuccess()) { return; }
	_shader_list.push_back(combine_shader);

	Shader* blur_shader = new Shader("TexturedVertex.glsl", "ProcessFrag.glsl");
	if (!blur_shader->LoadSuccess()) { return; }
	_shader_list.push_back(blur_shader);

	Shader* shadow_shader = new Shader("ShadowVertex.glsl", "ShadowFragment.glsl");
	if (!shadow_shader->LoadSuccess()) { std::cout << "failed" << std::endl; return; }
	_shader_list.push_back(shadow_shader);

}

void Renderer::InitLight() {
	light_sphere = Mesh::LoadFromMeshFile("Sphere.msh");
	screen_quad = Mesh::GenerateQuad();
	final_quad = Mesh::GenerateQuad();
	blur_quad = Mesh::GenerateQuad();
	point_lights = new Light[NUMLIGHTS];
	
	Vector3 height_map_size = _terrain_mesh->GetHeightMapSize();

	_scene_light = new Light();
	_scene_light->SetPosition(Vector3(8000, 4000.0f, 8000));
	_scene_light->SetColour(Vector4(1, 1, 1, 1));
	_scene_light->SetRadius(10000);


	Light&  main_caster = point_lights[0];
	main_caster.SetPosition(Vector3(7500.0f, 1500.0f, 7500.0f));
	main_caster.SetColour(Vector4(1, 1, 1, 1));
	main_caster.SetRadius(15000);

	Light& l = point_lights[1];
	l.SetPosition(Vector3(2500, 2500, 4500));
	l.SetColour(Vector4(0, 1, 0, 1));
	l.SetRadius(10000);

	Light& l2 = point_lights[2];
	l2.SetPosition(Vector3(7500, 3000.0f, 4500));
	l2.SetColour(Vector4(1.2, 0, 1.2, 1));
	l2.SetRadius(12000);

	Light& l3 = point_lights[3];
	l3.SetPosition(Vector3(12500, 3000.0f, 4500));
	l3.SetColour(Vector4(0, 1, 1, 1));
	l3.SetRadius(9000);

	Light& l4 = point_lights[4];
	l4.SetPosition(Vector3(12500, 3000.0f, 12000));
	l4.SetColour(Vector4(1.2, 0, 1.2, 1));
	l4.SetRadius(9000);

	Light& l5 = point_lights[5];
	l5.SetPosition(Vector3(2500, 3000.0f, 12000));
	l5.SetColour(Vector4(0, 1, 1, 1));
	l5.SetRadius(9000);

	Light& l6 = point_lights[6];
	l6.SetPosition(Vector3(7500, 3000.0f, 12000));
	l6.SetColour(Vector4(0, 1, 0, 1));
	l6.SetRadius(9000);

	for (int i = 7; i < NUMLIGHTS; ++i) {
		Light& l = point_lights[i];
		int x = (int)height_map_size.x;
		int z = (int)height_map_size.z;
		Vector3 position = Vector3(rand() % x, 1100, rand() % z);
		l.SetPosition(Vector3(position));
		l.SetColour(Vector4(1,1,1,1));
		l.SetRadius(700.0f + (rand() % 500));
	}
}

void Renderer::InitFBO() {
	
	glGenFramebuffers(1, &buffer_fbo);
	glGenFramebuffers(1, &point_light_fbo);
	glGenFramebuffers(1, &sky_box_fbo);
	glGenFramebuffers(1, &blur_fbo);
	glGenFramebuffers(1, &_shadow_fbo);

	
	GLenum buffers[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};

	GenerateScreenTexture(buffer_depth_tex, true);
	GenerateScreenTexture(buffer_colour_tex);
	GenerateScreenTexture(buffer_normal_tex);
	GenerateScreenTexture(light_diffuse_tex);
	GenerateScreenTexture(light_specular_tex);
	GenerateScreenTexture(light_blur_tex);
	GenerateScreenTexture(sky_depth_tex, true);
	GenerateScreenTexture(sky_colour_tex);
	GenerateScreenTexture(blur_tex);
	GenerateScreenTexture(shadow_colour_tex);
	
	glGenTextures(1, &_shadow_tex);
	glBindTexture(GL_TEXTURE_2D, _shadow_tex);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOWSIZE, SHADOWSIZE, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glBindTexture(GL_TEXTURE_2D, 0);
	
	glBindFramebuffer(GL_FRAMEBUFFER, _shadow_fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, _shadow_tex, 0);
	glDrawBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, buffer_fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, buffer_colour_tex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, buffer_normal_tex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, buffer_depth_tex, 0);
	glDrawBuffers(2, buffers);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) { return; }
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
	glBindFramebuffer(GL_FRAMEBUFFER, point_light_fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, light_diffuse_tex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, light_specular_tex, 0);
	glDrawBuffers(2, buffers);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) { return; }
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, sky_box_fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, sky_colour_tex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, light_blur_tex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,  sky_depth_tex, 0);
	glDrawBuffers(2, buffers);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) { return; }
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	
}

void Renderer::GenerateScreenTexture(GLuint& into, bool depth) {
	glGenTextures(1, &into);
	glBindTexture(GL_TEXTURE_2D, into);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	GLuint format = depth ? GL_DEPTH_COMPONENT24 : GL_RGBA8;
	GLuint type = depth ? GL_DEPTH_COMPONENT : GL_RGBA;

	glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, type, GL_UNSIGNED_BYTE, NULL);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Renderer::InitWorld() {
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	_skybox_texture = SOIL_load_OGL_cubemap(TEXTUREDIR"left.png", TEXTUREDIR"right.png", TEXTUREDIR"top.png", TEXTUREDIR"bottom.png", TEXTUREDIR"back.png", TEXTUREDIR"front.png", SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, 0);
	_skybox_mesh = Mesh::GenerateQuad();

	if (!_skybox_texture) {
		std::cout << "ERROR WITH SKYBOX TEXTURE!";
		return;
	}

	_terrain_texture = SOIL_load_OGL_texture(TEXTUREDIR"planetground.JPG", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	_terrain_mesh = new HeightMap(TEXTUREDIR"planetsurface.png", 5);
	_terrain_bump = SOIL_load_OGL_texture(TEXTUREDIR"planetbump.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	if (!_terrain_texture) {
		std::cout << "ERROR WITH TERRAIN TEXTURE!";
		return;
	}

	snow_texture =SOIL_load_OGL_texture(TEXTUREDIR"snow.PNG", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	snow_bump = SOIL_load_OGL_texture(TEXTUREDIR"snowbump.PNG", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	snow_scale_factor = 0;

	_water_texture = SOIL_load_OGL_texture(TEXTUREDIR"water.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	_water_mesh = new HeightMap(TEXTUREDIR"waterheightmap.png", 2);
	_water_bump = SOIL_load_OGL_texture(TEXTUREDIR"waterbump.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	
	SetTextureRepeating(snow_texture, true);
	SetTextureRepeating(snow_bump, true);
	SetTextureRepeating(_water_texture, true);
	SetTextureRepeating(_water_bump, true);
	SetTextureRepeating(_terrain_texture, true);
	SetTextureRepeating(_terrain_bump, true);
}

SceneNode* Renderer::CreateSceneNode(string mesh_file, string mat_file) {
	SceneNode* node = new SceneNode();
	Mesh* mesh = Mesh::LoadFromMeshFile(mesh_file);
	MeshMaterial* node_mat = new MeshMaterial(mat_file);
	std::vector<GLuint> node_textures = LoadTextures(mesh, node_mat);
	node->SetColour(Vector4(1.0f, 1.0f, 1.0f, 1.0f));
	node->SetModelScale(Vector3(100.0f, 100.0f, 100.0f));
	node->SetBoundingRadius(50.0f);
	node->SetMesh(mesh);
	node->matTextures = node_textures;
	node->SetShaderRef(4);
	node->SetTransform(Matrix4::Translation(Vector3(3000.0f, 5000.0f, 3000.0f)));
	_root->AddChild(node);
	return node; 
}

void Renderer::GenerateObjects() {
	 GenerateBuildings();
 	 GenerateShips();
	 GenerateAsteroids();
     GenerateMisc();

}

void Renderer::GenerateMisc() {

	GLuint gas_tex = SOIL_load_OGL_texture(TEXTUREDIR"gastanktexture.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	GLuint gas_bump = SOIL_load_OGL_texture(TEXTUREDIR"gastanknormal.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	Mesh*  gas_mesh = Mesh::LoadFromMeshFile("gastank.msh");

	SceneNode* gas_tank = new SceneNode();
	gas_tank->SetMesh(gas_mesh);
	gas_tank->SetTexture(gas_tex);
	gas_tank->SetBumpTexture(gas_bump);
	gas_tank->SetModelScale(Vector3(200.0f, 200.0f, 200.0f));
	gas_tank->SetTransform(Matrix4::Translation(Vector3(11500.0f, 1200.0f, 10000.0f)));
	_root->AddChild(gas_tank);

	gas_tank = new SceneNode();
	gas_tank->SetMesh(gas_mesh);
	gas_tank->SetTexture(gas_tex);
	gas_tank->SetBumpTexture(gas_bump);
	gas_tank->SetModelScale(Vector3(200.0f, 200.0f, 200.0f));
	gas_tank->SetTransform(Matrix4::Translation(Vector3(12000.0f, 1200.0f, 10000.0f)));
	_root->AddChild(gas_tank);

	gas_tank = new SceneNode();
	gas_tank->SetMesh(gas_mesh);
	gas_tank->SetTexture(gas_tex);
	gas_tank->SetBumpTexture(gas_bump);
	gas_tank->SetModelScale(Vector3(200.0f, 200.0f, 200.0f));
	gas_tank->SetTransform(Matrix4::Translation(Vector3(11000.0f, 1100.0f, 10000.0f)));
	_root->AddChild(gas_tank);

	gas_tank = new SceneNode();
	gas_tank->SetMesh(gas_mesh);
	gas_tank->SetTexture(gas_tex);
	gas_tank->SetBumpTexture(gas_bump);
	gas_tank->SetModelScale(Vector3(200.0f, 200.0f, 200.0f));
	gas_tank->SetTransform(Matrix4::Translation(Vector3(10500.0f, 1100.0f, 10000.0f)));
	_root->AddChild(gas_tank);
	 
	///////////////////////////////////////////////////////////////////////////////////////////

	gas_tank->SetMesh(gas_mesh);
	gas_tank->SetTexture(gas_tex);
	gas_tank->SetBumpTexture(gas_bump);
	gas_tank->SetModelScale(Vector3(200.0f, 200.0f, 200.0f));
	gas_tank->SetTransform(Matrix4::Translation(Vector3(11500.0f, 1200.0f, 11000.0f)));
	_root->AddChild(gas_tank);

	gas_tank = new SceneNode();
	gas_tank->SetMesh(gas_mesh);
	gas_tank->SetTexture(gas_tex);
	gas_tank->SetBumpTexture(gas_bump);
	gas_tank->SetModelScale(Vector3(200.0f, 200.0f, 200.0f));
	gas_tank->SetTransform(Matrix4::Translation(Vector3(12000.0f, 1200.0f, 11000.0f)));
	_root->AddChild(gas_tank);

	gas_tank = new SceneNode();
	gas_tank->SetMesh(gas_mesh);
	gas_tank->SetTexture(gas_tex);
	gas_tank->SetBumpTexture(gas_bump);
	gas_tank->SetModelScale(Vector3(200.0f, 200.0f, 200.0f));
	gas_tank->SetTransform(Matrix4::Translation(Vector3(11000.0f, 1200.0f, 11000.0f)));
	_root->AddChild(gas_tank);

	gas_tank = new SceneNode();
	gas_tank->SetMesh(gas_mesh);
	gas_tank->SetTexture(gas_tex);
	gas_tank->SetBumpTexture(gas_bump);
	gas_tank->SetModelScale(Vector3(200.0f, 200.0f, 200.0f));
	gas_tank->SetTransform(Matrix4::Translation(Vector3(10500.0f, 1200.0f, 11000.0f)));
	_root->AddChild(gas_tank);
}

void Renderer::GenerateShips() {
	/* Generate Ships */
	GLuint ship_tex = SOIL_load_OGL_texture(TEXTUREDIR"shiptex.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	GLuint ship_bump = SOIL_load_OGL_texture(TEXTUREDIR"shipbump.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	Mesh* ship_mesh = Mesh::LoadFromMeshFile("ship.msh");
	Vector3 ship_scale = Vector3(40.0f, 40.0f, 40.0f);

	Ship* ship = new Ship();
	ship->SetMesh(ship_mesh);
	ship->SetModelScale(ship_scale);
	ship->SetTexture(ship_tex);
	ship->SetBumpTexture(ship_bump);
	ship->initialPosition = Vector3(0, 1500.0f, 8350.0f);
	ship->speed = 2000.0f;
	ship->SetTransform(Matrix4::Translation(ship->initialPosition) * Matrix4::Rotation(90, Vector3(0, 1, 0)));
	_root->AddChild(ship);

    ship = new Ship();
	ship->SetMesh(ship_mesh);
	ship->SetModelScale(ship_scale);
	ship->SetTexture(ship_tex);
	ship->SetBumpTexture(ship_bump);
	ship->initialPosition = Vector3(0, 2500.0f, 6350.0f);
	ship->speed = 1500.0f;
	ship->SetTransform(Matrix4::Translation(ship->initialPosition) * Matrix4::Rotation(90, Vector3(0, 1, 0)));
	_root->AddChild(ship);

	ship = new Ship();
	ship->SetMesh(ship_mesh);
	ship->SetModelScale(ship_scale);
	ship->SetTexture(ship_tex);
	ship->SetBumpTexture(ship_bump);
	ship->initialPosition = Vector3(0, 2000.0f, 10350.0f);
	ship->speed = 2200.0f;
	ship->SetTransform(Matrix4::Translation(ship->initialPosition) * Matrix4::Rotation(90, Vector3(0, 1, 0)));
	_root->AddChild(ship);

	ship = new Ship();
	ship->SetMesh(ship_mesh);
	ship->SetModelScale(ship_scale);
	ship->SetTexture(ship_tex);
	ship->SetBumpTexture(ship_bump);
	ship->initialPosition = Vector3(0, 3000.0f, 7350.0f);
	ship->speed = 2500.0f;
	ship->SetTransform(Matrix4::Translation(ship->initialPosition) * Matrix4::Rotation(90, Vector3(0, 1, 0)));
	_root->AddChild(ship);


	ship = new Ship();
	ship->SetMesh(ship_mesh);
	ship->SetModelScale(ship_scale);
	ship->SetTexture(ship_tex);
	ship->SetBumpTexture(ship_bump);
	ship->initialPosition = Vector3(0, 3000.0f, 7350.0f);
	ship->speed = 2500.0f;
	ship->SetTransform(Matrix4::Translation(ship->initialPosition) * Matrix4::Rotation(90, Vector3(0, 1, 0)));
	_root->AddChild(ship);

	ship = new Ship();
	ship->SetMesh(ship_mesh);
	ship->SetModelScale(ship_scale);
	ship->SetTexture(ship_tex);
	ship->SetBumpTexture(ship_bump);
	ship->initialPosition = Vector3(0, 1500.0f, 8350.0f);
	ship->speed = 1000.0f;
	ship->SetTransform(Matrix4::Translation(ship->initialPosition) * Matrix4::Rotation(90, Vector3(0, 1, 0)));
	_root->AddChild(ship);
}

void Renderer::GenerateBuildings() {
	float x_incr = 600;
	float z_incr = 1200;
	int building_choice = 0;
	Vector3 type_1_base = Vector3(1030.0f, 1000.0f, 2150.0f);
	Vector3 type_2_base = Vector3(900.0f, 950.0f, 500.0f);
	Vector3 type_3_base = Vector3(910.0f, 1075.0f, -400.0f);
	Vector3 type_4_base = Vector3(420.0f, 950.0f, 900.0f);

	Vector3 type_1_base_reflect = Vector3(750.0f, 1000.0f, -1100.0f);
	Vector3 type_3_base_reflect = Vector3(910.0f, 1075.0f, 1400.0f);
	Vector3 type_4_base_reflect = Vector3(1350.0f, 950.0f, 0.0f);


	Mesh* building_type_1_mesh = Mesh::LoadFromMeshFile("highrise.msh");
	MeshMaterial* building_type_1_mat = new MeshMaterial("highrise.mat");
	std::vector<GLuint> type_1_textures = LoadTextures(building_type_1_mesh, building_type_1_mat);

	Mesh* building_type_2_mesh = Mesh::LoadFromMeshFile("highrise1.msh");
	MeshMaterial* building_type_2_mat = new MeshMaterial("highrise1.mat");
	std::vector<GLuint> type_2_textures = LoadTextures(building_type_2_mesh, building_type_2_mat);

	Mesh* building_type_3_mesh = Mesh::LoadFromMeshFile("highrise2.msh");
	MeshMaterial* building_type_3_mat = new MeshMaterial("highrise2.mat");
	std::vector<GLuint> type_3_textures = LoadTextures(building_type_3_mesh, building_type_3_mat);

	Mesh* building_type_4_mesh = Mesh::LoadFromMeshFile("highrise3.msh");
	MeshMaterial* building_type_4_mat = new MeshMaterial("highrise3.mat");
	std::vector<GLuint> type_4_textures = LoadTextures(building_type_4_mesh, building_type_4_mat);

	GLuint building_bump = SOIL_load_OGL_texture(TEXTUREDIR"buildingbump.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);

	int layer_sizes[ITERATIONS] = { 25,25,25,25,14,13,12,3,0,0,25,25,25,25};



	bool reflect = false;

	for (int i = 0; i < ITERATIONS; i++) {
		if (i == 8) { reflect = true; }
		for (int j = 0; j < layer_sizes[i]; j++) {
			building_choice = rand() % 4;
			switch (building_choice) {
			case 0: {
				Building* building_type_1 = new Building();
				building_type_1->SetMesh(building_type_1_mesh);
				building_type_1->matTextures = type_1_textures;
				if (reflect) {
					building_type_1->SetTransform(Matrix4::Translation(type_1_base_reflect));
					building_type_1->Rotate(-30);
				}
				else {
					building_type_1->SetTransform(Matrix4::Translation(type_1_base));
					building_type_1->Rotate(150);
				}
				building_type_1->SetBumpTexture(building_bump);
				_root->AddChild(building_type_1);
			} break;
			case 1: {
				Building* building_type_2 = new Building();
				building_type_2->SetMesh(building_type_2_mesh);
				building_type_2->matTextures = type_2_textures;
				building_type_2->SetTransform(Matrix4::Translation(type_2_base));
				building_type_2->SetBumpTexture(building_bump);
				_root->AddChild(building_type_2);
			} break;
			case 2: {
				Building* building_type_3 = new Building();
				building_type_3->SetMesh(building_type_3_mesh);
				building_type_3->matTextures = type_3_textures;
				if (reflect) {
					building_type_3->SetTransform(Matrix4::Translation(type_3_base_reflect));
					building_type_3->Rotate(180);
				}
				else {
					building_type_3->SetTransform(Matrix4::Translation(type_3_base));
				}
				building_type_3->SetBumpTexture(building_bump);
				_root->AddChild(building_type_3);
			} break;
			case 3: {
				Building* building_type_4 = new Building();
				building_type_4->SetMesh(building_type_4_mesh);
				building_type_4->matTextures = type_4_textures;
				if (reflect) {
					building_type_4->SetTransform(Matrix4::Translation(type_4_base_reflect));
				}
				else {
					building_type_4->SetTransform(Matrix4::Translation(type_4_base));
					building_type_4->Rotate(180);
				}
				building_type_4->SetBumpTexture(building_bump);
				_root->AddChild(building_type_4);
			} break;
			}

			type_1_base.x += x_incr;
			type_2_base.x += x_incr;
			type_3_base.x += x_incr;
			type_4_base.x += x_incr;

			type_1_base_reflect.x += x_incr;
			type_3_base_reflect.x += x_incr;
			type_4_base_reflect.x += x_incr;

		}
		


		type_1_base = Vector3(1030.0f, 1000.0f, type_1_base.z);
		type_2_base = Vector3(900.0f, 950.0f, type_2_base.z);
		type_3_base = Vector3(910.0f, 1075.0f, type_3_base.z);
		type_4_base = Vector3(420.0f, 950.0f, type_4_base.z);

		type_1_base_reflect = Vector3(750.0f, 1000.0f, type_1_base_reflect.z);
		type_3_base_reflect = Vector3(910.0f, 1075.0f, type_3_base_reflect.z);
		type_4_base_reflect = Vector3(1350.0f, 950.0f, type_4_base_reflect.z);


		type_1_base.z += z_incr;
		type_2_base.z += z_incr;
		type_3_base.z += z_incr;
		type_4_base.z += z_incr;
		
		type_1_base_reflect.z += z_incr;
		type_3_base_reflect.z += z_incr;
		type_4_base_reflect.z += z_incr;
	}

}

void Renderer::GenerateAsteroids() {

	_asteroid_meshes.push_back(Mesh::LoadFromMeshFile("rock.msh"));
	_asteroid_meshes.push_back(Mesh::LoadFromMeshFile("rock1.msh"));
	_asteroid_meshes.push_back(Mesh::LoadFromMeshFile("rock2.msh"));
	_asteroid_meshes.push_back(Mesh::LoadFromMeshFile("rock3.msh"));
	_asteroid_meshes.push_back(Mesh::LoadFromMeshFile("rock4.msh"));
	_asteroid_meshes.push_back(Mesh::LoadFromMeshFile("rock5.msh"));

	for (int i = 0; i < NUMASTEROIDS; i++) {
		Asteroid* asteroid = new Asteroid();
		int mesh_choice = rand() % 5;
		asteroid->SetMesh(_asteroid_meshes[mesh_choice]);
		float x_pos = (rand() % 9000) - 3000;
		float y_pos = (rand() % 3000) + 2500;
		float z_pos = (rand() % 9000) - 3000;
		asteroid->SetTransform(Matrix4::Translation(Vector3(x_pos, y_pos, z_pos)));
		float orbit_speed = (rand() % 80) + 5;
		asteroid->SetOrbitSpeed(orbit_speed);
		_root->AddChild(asteroid);
	}

}

vector<GLuint> Renderer::LoadTextures(Mesh* mesh, MeshMaterial* mat) {
	vector<GLuint> textures;
	for (int i = 0; i < mesh->GetSubMeshCount(); ++i) {
		const MeshMaterialEntry* matEntry = mat->GetMaterialForLayer(i);
		const string* filename = nullptr;
		matEntry->GetEntry("Diffuse", &filename);
		string path = TEXTUREDIR + *filename;
		GLuint tex_id = SOIL_load_OGL_texture(path.c_str(), SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y);
		textures.emplace_back(tex_id);
	}
	return textures;
}

void Renderer::UpdateScene(float dt) {
	projMatrix = Matrix4::Perspective(1.0f, 30000.0f, (float)(width) / (float)height, 45.0f);
	if (Window::GetKeyboard()->KeyDown(KEYBOARD_F1)) {
		_camera->tracked = true;
		_camera->SetPosition(Vector3(0, 6000.0f, 0));
		_camera->camera_time = 0;
		_camera->index = 0;
		
	}

	viewMatrix = _camera->BuildViewMatrix();
	if (_camera->tracked) {
		projMatrix = Matrix4::Perspective(1.0f, 30000.0f, (float)(width) / (float)height, 45.0f);
		viewMatrix = Matrix4::BuildViewMatrix(_camera->GetPosition(), Vector3(8192, 0, 8192));
	}

	if (Window::GetKeyboard()->KeyDown(KEYBOARD_F2)) {
		_camera->tracked = false;
		_camera->SetPosition(Vector3(8192, 10000, 8192));
		_camera->SetPitch(-90);
	}


	if (Window::GetKeyboard()->KeyDown(KEYBOARD_F3)) {
		draw_blur = !draw_blur;
	}

	if (Window::GetKeyboard()->KeyDown(KEYBOARD_F4)) {
		use_shadow = !use_shadow;
	}
	_camera->UpdateCamera(dt);

	_water_rotate += dt * 2.0f;
	_water_cycle  += dt * 0.25f;
	_default_frustum.FromMatrix(projMatrix * viewMatrix);
	snow_scale_factor += 0.1f;
	_root->Update(dt);
}

void Renderer::RenderScene() {
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	BuildObjectList(_root);
	SortObjectList();
	FillBuffers();
	DrawPointLights();
	CombineBuffers();
	if (draw_blur) {
		bloom_val = 1;
		DrawBlur();
	}
	else {
		bloom_val = 0;
	}

	DrawFinal();


	ClearObjectList();
}


void Renderer::DrawObjectList() {
	for (const auto& obj : _object_list) { DrawObject(obj); }
}

void Renderer::DrawObject(SceneNode* obj) {
	if (obj->GetMesh()) {
		Matrix4 model_matrix = obj->GetWorldTransform() * Matrix4::Scale(obj->GetModelScale());
		Shader* referenced_shader = _shader_list[5];
		TextureShaderUniforms(*referenced_shader, model_matrix, *obj);
	}
}


void Renderer::TextureShaderUniforms(Shader& referenced_shader, const Matrix4 model_matrix, SceneNode& obj) {
	textureMatrix.ToIdentity();
	BindShader(&referenced_shader);
	SetShaderLight(point_lights[0]);
	UpdateShaderMatrices();
	glUniformMatrix4fv(glGetUniformLocation(referenced_shader.GetProgram(), "modelMatrix"), 1, false, model_matrix.values);
	glUniform1i(glGetUniformLocation(referenced_shader.GetProgram(), "diffuseTex"), 0);
	glUniform1i(glGetUniformLocation(referenced_shader.GetProgram(), "bumpTex"), 1);
	GLuint bump_texture = obj.GetBumpTexture();
	if (obj.GetMesh()->GetSubMeshCount() > 1) {
		for (int i = 0; i < obj.GetMesh()->GetSubMeshCount(); ++i) {
			SetTextureRepeating(obj.matTextures[i], true);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, (obj.matTextures[i]));
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, bump_texture);
			BindShadowTex();
			obj.GetMesh()->DrawSubMesh(i);
		}
		return;
	}

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, obj.GetTexture());
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, bump_texture);
	BindShadowTex();
	obj.Draw(*this);
}


void Renderer::BuildObjectList(SceneNode* target) {
	if (_default_frustum.InsideFrustum(*target)) {
		Vector3 dir = target->GetWorldTransform().GetPositionVector() - _camera->GetPosition();
		target->SetCameraDistance(Vector3::Dot(dir, dir));
		_object_list.push_back(target);
	}

	for (vector<SceneNode*>::const_iterator i = target->GetChildIteratorStart(); i != target->GetChildIteratorEnd(); ++i) { BuildObjectList((*i)); }
}

void Renderer::SortObjectList() {
	std::sort(_object_list.begin(), _object_list.end(), SceneNode::CompareByCameraDistance);
}

void Renderer::ClearObjectList() {
	_object_list.clear();
}

void Renderer::DrawShadowScene() {
	Matrix4 model_matrix;
	glDisable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	glBindFramebuffer(GL_FRAMEBUFFER, _shadow_fbo);
	BindShader(_shader_list[9]);
	glClear(GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, SHADOWSIZE, SHADOWSIZE);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

	viewMatrix = Matrix4::BuildViewMatrix(point_lights[0].GetPosition(), Vector3(0, 0, 0));
	projMatrix = Matrix4::Perspective(1, 5000, 1, 45);
	shadowMatrix = projMatrix * viewMatrix;
		
	model_matrix.ToIdentity();
	glUniformMatrix4fv(glGetUniformLocation(_shader_list[9]->GetProgram(), "modelMatrix"), 1, false, model_matrix.values);
	_terrain_mesh->Draw();

	model_matrix.ToIdentity();
	glUniformMatrix4fv(glGetUniformLocation(_shader_list[9]->GetProgram(), "modelMatrix"), 1, false, model_matrix.values);
	_water_mesh->Draw();

	for (const auto& obj : _object_list) {
		if (obj->GetMesh()) {
			model_matrix = obj->GetWorldTransform() * Matrix4::Scale(obj->GetModelScale());
			glUniformMatrix4fv(glGetUniformLocation(_shader_list[9]->GetProgram(), "modelMatrix"), 1, false, model_matrix.values);
			
			if (obj->GetMesh()->GetSubMeshCount() > 1) {
				for (int i = 0; i < obj->GetMesh()->GetSubMeshCount(); ++i) {
					obj->GetMesh()->DrawSubMesh(i);
				}
			}
			else {
				obj->Draw(*this);
			}
		}
	}
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glViewport(0, 0, width, height);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
	viewMatrix = _camera->BuildViewMatrix();
	projMatrix = Matrix4::Perspective(1.0f, 30000.0f, (float)width / (float)height, 45.0f);
}

void Renderer::FillBuffers() {
	glBindFramebuffer(GL_FRAMEBUFFER, buffer_fbo);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	BindShader(_shader_list[5]);
	UpdateShaderMatrices();
	DrawTerrain(_shader_list[5]);
	DrawWater(_shader_list[3]);
	DrawObjectList();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::BindShadowTex() {
	if (currentShader) {
		glUniform1i(glGetUniformLocation(currentShader->GetProgram(), "shadowTex"), 2);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, _shadow_tex);
	}
}

void Renderer::DrawPointLights() {
	/*calculate lighting for each light volume in scene */

	glBindFramebuffer(GL_FRAMEBUFFER, point_light_fbo);
	BindShader(_shader_list[6]);

	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glBlendFunc(GL_ONE, GL_ONE);
	glCullFace(GL_FRONT);
	
	glUniform1i(glGetUniformLocation(_shader_list[6]->GetProgram(), "depthTex"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, buffer_depth_tex);

	glUniform1i(glGetUniformLocation(_shader_list[6]->GetProgram(), "normTex"), 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, buffer_normal_tex);

	glUniform3fv(glGetUniformLocation(_shader_list[6]->GetProgram(), "cameraPos"), 1, (float*)&_camera->GetPosition());
	glUniform2f(glGetUniformLocation(_shader_list[6]->GetProgram(), "pixelSize"), 1.0f / width, 1.0f / height);

	
	Matrix4 invViewProj = (projMatrix * viewMatrix).Inverse();
	glUniformMatrix4fv(glGetUniformLocation(_shader_list[6]->GetProgram(), "inverseProjView"), 1, false, invViewProj.values);

	UpdateShaderMatrices();

	for (int i = 0; i < NUMLIGHTS; ++i) {
		Light& l = point_lights[i];
		Matrix4 position = Matrix4::Translation(l.GetPosition()) * Matrix4::Rotation(10, Vector3(0, 1, 0));
		l.SetPosition(position.GetPositionVector());
		SetShaderLight(l);
		light_sphere->Draw();
	}

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glCullFace(GL_BACK);
	glDepthFunc(GL_LEQUAL);

	glDepthMask(GL_TRUE);
	glClearColor(0.4f, 0.4f, 0.4f, 1);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::CombineBuffers() {

	/* Final lighting calculation */

	glBindFramebuffer(GL_FRAMEBUFFER, sky_box_fbo);
	BindShader(_shader_list[7]);
	UpdateShaderMatrices();
	glUniform1i(glGetUniformLocation(_shader_list[7]->GetProgram(), "diffuseTex"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, buffer_colour_tex);

	glUniform1i(glGetUniformLocation(_shader_list[7]->GetProgram(), "diffuseLight"), 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, light_diffuse_tex);

	glUniform1i(glGetUniformLocation(_shader_list[7]->GetProgram(), "specularLight"), 2);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, light_specular_tex);
	screen_quad->Draw();
	
	/* Now combine skybox */

	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, buffer_fbo);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, sky_box_fbo);
	glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, sky_box_fbo);
	glDepthFunc(GL_EQUAL);
	glDepthMask(GL_FALSE);
	BindShader(_shader_list[1]);
	UpdateShaderMatrices();
	_skybox_mesh->Draw();
	glDepthMask(GL_TRUE);
	glDisable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::DrawBlur() {
	/* Blur and apply a bloom effect to the scene */
	glBindFramebuffer(GL_FRAMEBUFFER, blur_fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, blur_tex, 0);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	BindShader(_shader_list[8]);
	modelMatrix.ToIdentity();
	viewMatrix.ToIdentity();
	projMatrix.ToIdentity();
	textureMatrix.ToIdentity();
	UpdateShaderMatrices();

	glDisable(GL_DEPTH_TEST);

	glActiveTexture(GL_TEXTURE0);
	glUniform1i(glGetUniformLocation(_shader_list[8]->GetProgram(), "sceneTex"), 0);
	for (int i = 0; i < POSTPASSES; ++i) {

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, blur_tex, 0);
		glUniform1i(glGetUniformLocation(_shader_list[8]->GetProgram(), "isVertical"), 0);
		glBindTexture(GL_TEXTURE_2D, light_blur_tex);
		blur_quad->Draw();

		glUniform1i(glGetUniformLocation(_shader_list[8]->GetProgram(), "isVertical"), 1);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, light_blur_tex, 0);
		glBindTexture(GL_TEXTURE_2D, blur_tex);
		blur_quad->Draw();
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glEnable(GL_DEPTH_TEST);

}

void Renderer::DrawFinal() {

	modelMatrix.ToIdentity();
	viewMatrix.ToIdentity();
	projMatrix.ToIdentity();

	glDepthFunc(GL_LEQUAL);
	BindShader(_shader_list[0]);
	UpdateShaderMatrices();
	glUniform1i(glGetUniformLocation(_shader_list[0]->GetProgram(), "diffuseTex"), 0);
	glUniform1i(glGetUniformLocation(_shader_list[0]->GetProgram(), "bloomTex"), 1);
	glUniform1i(glGetUniformLocation(_shader_list[0]->GetProgram(), "drawBloom"), bloom_val);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, sky_colour_tex);
	
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, light_blur_tex);
	
	final_quad->Draw();

}


void Renderer::DrawTerrain(Shader* shader_ref) {
	
	Shader* terrain_shader = shader_ref;
	BindShader(terrain_shader);
	SetShaderLight(point_lights[0]);


	modelMatrix.ToIdentity();
	textureMatrix.ToIdentity();

	glUniform1i(glGetUniformLocation(terrain_shader->GetProgram(), "diffuseTex"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _terrain_texture);

	glUniform1i(glGetUniformLocation(terrain_shader->GetProgram(), "bumpTex"), 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, _terrain_bump);

	glUniform1i(glGetUniformLocation(terrain_shader->GetProgram(), "snowTex"), 2);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, snow_texture);

	glUniform1f(glGetUniformLocation(terrain_shader->GetProgram(), "scaleFactor"), snow_scale_factor);

	UpdateShaderMatrices();
	_terrain_mesh->Draw();
}

void Renderer::DrawWater(Shader* shader_ref) {
	Shader* water_shader = shader_ref;
	BindShader(water_shader);
	
	glUniform3fv(glGetUniformLocation(water_shader->GetProgram(), "cameraPos"), 1, (float*)&_camera->GetPosition());
	glUniform1i(glGetUniformLocation(water_shader->GetProgram(), "diffuseTex"), 0);
	glUniform1i(glGetUniformLocation(water_shader->GetProgram(), "bumpTex"), 1);
	glUniform1i(glGetUniformLocation(water_shader->GetProgram(), "cubeTex"), 2);
	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _water_texture);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, _water_bump);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_CUBE_MAP, _skybox_texture);


	modelMatrix.ToIdentity();
	textureMatrix.ToIdentity();

	modelMatrix = Matrix4::Translation(Vector3(0, 250, 0));
	textureMatrix = Matrix4::Translation(Vector3(_water_cycle, 0.0f, _water_cycle)) * Matrix4::Scale(Vector3(0.5, 0.5, 0.5)) * Matrix4::Rotation(_water_rotate, Vector3(0, 0, 1));

	UpdateShaderMatrices();
	SetShaderLight(*_scene_light);
	_water_mesh->Draw();
}