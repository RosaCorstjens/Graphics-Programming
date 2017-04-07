#include "PyramidGenerator.h"
/// <summary>
/// Initializes a new instance of the <see cref="PrimitiveGenerator"/> .
/// </summary>
PrimitiveGenerator::PrimitiveGenerator() { }

/// <summary>
/// Creates the pyramid.
/// </summary>
/// <param name="radius">The radius.</param>
/// <param name="height">The height.</param>
/// <param name="sides">The sides.</param>
/// <param name="meshdata">The meshdata.</param>
void PrimitiveGenerator::CreatePyramid(float radius, float height, int sides, MeshData& meshdata)
{
	// clear the meshdata member variables. 
	meshdata.Vertices.clear();
	meshdata.Indices.clear();

	// create apex 
	meshdata.Vertices.push_back(Vertex(XMFLOAT3(+0.0f, height, +0.0f)));

	// calculate all other vertices
	for (int i = 0; i < sides; ++i)
	{
		meshdata.Vertices.push_back(Vertex(XMFLOAT3(radius * cosf(2 * MathHelper::Pi * i / sides), +0.0f, radius * sinf(2 * MathHelper::Pi * i / sides))));
	}

	// create center of base
	meshdata.Vertices.push_back(Vertex(XMFLOAT3(+0.0f, +0.0f, +0.0f)));

	int vertexcount = meshdata.Vertices.size() - 1;

	// amount of indices = (sides * 3) * 2
	// since every side needs three indices for the side 
	// and three indices for the bottom. 

	// so for each side a side and bottom triangle will be constructed
	for (int i = 0; i < sides - 1; ++i)
	{
		// first create side triangle
		meshdata.Indices.push_back(0);					// each triangle starts with apex
		meshdata.Indices.push_back(i + 2);
		meshdata.Indices.push_back(i + 1);

		// than create bottom triangle
		meshdata.Indices.push_back(vertexcount);		// each triangle starts with center base
		meshdata.Indices.push_back(i + 1);
		meshdata.Indices.push_back(i + 2);
	}

	// add the last two triangles for the last side seperatly since they attach the last to the first side vertex.
	// first create side triangle
	meshdata.Indices.push_back(0);					// each triangle starts with apex
	meshdata.Indices.push_back(1);
	meshdata.Indices.push_back(vertexcount - 1);

	// than create bottom triangle
	meshdata.Indices.push_back(vertexcount);		// each triangle starts with center base
	meshdata.Indices.push_back(vertexcount - 1);
	meshdata.Indices.push_back(1);

}