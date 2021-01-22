#pragma once
#include "../nclgl/OGLRenderer.h"
#include "../nclgl/SceneNode.h"
#include "../nclgl/Frustum.h"
#include "../nclgl/Camera.h"
#include "../nclgl/Mesh.h"
#include "../nclgl/Shader.h"
#include "../nclgl/HeightMap.h"
#include "../nclgl/Light.h"
#include "Asteroid.h"
#include "Ship.h"
#include "Building.h"
#include "Rain.h"

class Renderer : public OGLRenderer	{
public:
	 Renderer(Window &parent);
	 ~Renderer(void);
	 void RenderScene()				override;
	 void UpdateScene(float dt)	    override;

protected:
	void DrawObjectList();
	void DrawObject(SceneNode* obj);
	void SortObjectList();
	void BuildObjectList(SceneNode* target);
	void ClearObjectList();
	void GenerateObjects();
	void GenerateBuildings();
	void GenerateAsteroids();
	void GenerateShips();
	void GenerateMisc();
	void LoadShaderList();
	void GenerateScreenTexture(GLuint& into, bool depth = false);
	void InitWorld();
	void InitLight();
	void InitFBO();
	void FillBuffers();
	void DrawPointLights();
	void CombineBuffers();
	void DrawBlur();
	void DrawFinal();
	void DrawShadowScene();
	void BindShadowTex();
	void SetOPENGLFunc();
	SceneNode* CreateSceneNode(string mesh_file, string mat_file);

	void DrawTerrain(Shader* shader_ref);
	void DrawWater(Shader* shader_ref);

	void TextureShaderUniforms(Shader& referenced_shader, const Matrix4 model_matrix, SceneNode& obj);

	std::vector<GLuint> LoadTextures(Mesh* mesh, MeshMaterial* mat);

	SceneNode* _root;
	Camera*    _camera;
	Frustum    _default_frustum;
	
	Mesh*	   _skybox_mesh;
	GLuint     _skybox_texture;
	
	HeightMap* _terrain_mesh;
	GLuint     _terrain_texture;
	GLuint     _terrain_bump;
	
	HeightMap* _water_mesh;
	GLuint     _water_texture;
	GLuint	   _water_bump;
	GLuint	   _water_FBO;
	float      _water_rotate;
	float      _water_cycle;


	GLuint snow_texture;
	GLuint snow_bump;
	float  snow_scale_factor;

	Light*	   _scene_light; 

	
	vector<SceneNode*> _object_list;
	vector<Shader*> _shader_list;
	vector<Mesh*> _asteroid_meshes;
	
	bool  _tracked = false; 

	GLuint buffer_fbo;
	GLuint buffer_colour_tex;
	GLuint buffer_depth_tex;
	GLuint buffer_normal_tex;

	GLuint point_light_fbo;
	GLuint light_diffuse_tex;
	GLuint light_specular_tex;
	Light* point_lights;
	Mesh*  light_sphere;
	Mesh*  screen_quad; 
	Mesh*  final_quad;
	GLuint sky_box_fbo;
	GLuint sky_colour_tex;
	GLuint light_blur_tex;
	GLuint sky_depth_tex;

	GLuint blur_fbo;
	GLuint blur_tex;
	Mesh*  blur_quad;

	bool   draw_blur = false;
	int    bloom_val = 0;

	GLuint _shadow_tex;
	GLuint _depth_test;
	GLuint _shadow_fbo;
	GLuint shadow_colour_tex;
	bool    use_shadow = 0;

};
