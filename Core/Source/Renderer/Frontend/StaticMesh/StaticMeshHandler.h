#ifndef STATICMESHHANDLER_H
#define STATICMESHHANDLER_H
#include <vector>
#include "StaticMesh.h"


class StaticMeshHandler
{
public:
	// We need a list of meshes that we are going to draw
	// We need to represent a "static mesh" in some form of struct/class
	
	std::vector<StaticMesh> m_meshList = {};

	// We need a call to our Vulkan api to draw a mesh from m_meshList
private:

};

#endif

