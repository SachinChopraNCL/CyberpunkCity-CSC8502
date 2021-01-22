#pragma once
#include <string>
#include "Mesh.h"
#include "Vector3.h"

class HeightMap : public Mesh {
public:
	HeightMap(const std::string& name, int y_height);
	~HeightMap(void) {}

	Vector3 GetHeightMapSize() const { return heightMapSize; }
	float y_height;
protected:
	Vector3 heightMapSize;
};

