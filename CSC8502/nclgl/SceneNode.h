#pragma once
#include "OGLRenderer.h"
#include "MeshMaterial.h"
class SceneNode
{
public:
	SceneNode(Mesh* m = NULL, Vector4 colour = Vector4(1, 1, 1, 1), int shaderRef = 0);
	~SceneNode(void);

	void           SetTransform(const Matrix4& matrix) { transform = matrix; }
	const Matrix4& GetTransform() const { return transform; }
	Matrix4        GetWorldTransform() const { return worldTransform; }
	void		   SetWorldTransform(const Matrix4& matrix) { worldTransform = matrix; }

	Vector4        GetColour()  const { return colour; }
	void           SetColour(Vector4 c) { colour = c; }

	Vector3        GetModelScale() const { return modelScale; }
	void		   SetModelScale(Vector3 s) { modelScale = s; }

	Mesh* GetMesh()  const { return mesh; }
	void		   SetMesh(Mesh* m) { mesh = m; }

	unsigned int GetShaderRef() const { return shaderRef; }
	void		 SetShaderRef(int i) { shaderRef = i; }

	void           AddChild(SceneNode* s);

	virtual  void  Update(float dt);
	virtual  void  Draw(const OGLRenderer& r);

	std::vector<SceneNode*>::const_iterator GetChildIteratorStart() { return children.begin(); }
	std::vector<SceneNode*>::const_iterator GetChildIteratorEnd()   { return children.end(); }

	float GetBoundingRadius() const  { return boundingRadius; }
	void  SetBoundingRadius(float f) { boundingRadius = f; }

	float GetCameraDistance() const  { return distanceFromCamera; }
	void  SetCameraDistance(float f) { distanceFromCamera = f; }

	void SetTexture(GLuint tex) { texture = tex; }
	GLuint GetTexture() const   { return texture; }

	void SetBumpTexture(GLuint tex) { bumpTexture = tex; }
	GLuint GetBumpTexture() const { return bumpTexture; }

	static bool CompareByCameraDistance(SceneNode* a, SceneNode* b) {
		return (a->distanceFromCamera < b->distanceFromCamera) ? true : false;
	}

	void LoadMeshes(string meshFile, string matFile);

	MeshMaterial* material;
	std::vector<GLuint> matTextures;

protected:
	SceneNode* parent;
	Mesh* mesh;
    int shaderRef;
	Matrix4 worldTransform;
	Matrix4 transform;
	Vector3 modelScale;
	Vector4 colour;
	std::vector<SceneNode*> children;
	float distanceFromCamera;
	float boundingRadius;
	GLuint texture;
	GLuint bumpTexture;

};

