#pragma once
#include "MathHelper.h"
#include <math.h>
#include <vector>
#include "d3dApp.h"

class PrimitiveGenerator
{
public:
	PrimitiveGenerator();
	//~PrimitiveGenerator();

	// vertex used for primitive generation
	struct Vertex
	{
		Vertex() { Vertex(XMFLOAT3(0, 0, 0)); }
		Vertex(const XMFLOAT3& p) : Position(p) {}

		XMFLOAT3 Position;
	};

	// structure containing all mesh data
	struct MeshData
	{
		std::vector<Vertex> Vertices;
		std::vector<UINT> Indices;
	};

	// TO DO: implement this method with adjustable amount of sides.
	void CreatePyramid(float baseWidth, float height, int sides, MeshData& meshdata);
};
