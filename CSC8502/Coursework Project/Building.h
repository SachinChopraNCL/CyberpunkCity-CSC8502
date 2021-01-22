#pragma once
#include "../nclgl/SceneNode.h"
#include <string.h>
class Building : public SceneNode {
public:
	Building();
	~Building(void);
	void Update(float dt) override;
	void Rotate(float degree);


protected: 
};

