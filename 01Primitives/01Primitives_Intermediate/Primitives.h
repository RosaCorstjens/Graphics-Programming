#pragma once
#include "d3dApp.h"
#include "PyramidGenerator.h"

// structure for a vertex
struct Vertex
{
	XMFLOAT3 Position;		// 3D vector containing a position
	XMFLOAT4 Color;			// 4D vector containing a color
};

// primitives class, the application
// TO DO: move generation code to new class
class Primitives : public D3DApp
{
public:
	Primitives(HINSTANCE hinstance);		// constructor
	~Primitives();							// destructor

	bool Init();							
	void OnResize();
	void UpdateScene(float dt);
	void DrawScene();

	void OnMouseDown(WPARAM btnState, int x, int y);
	void OnMouseUp(WPARAM btnState, int x, int y);
	void OnMouseMove(WPARAM btnState, int x, int y);

private:
	void InitGeometryBuffers();				// initializes dynamic buffers for the vertices and indices.
	void SetGeometryBuffers();			// builds and fills the buffers with a pyramid based on the current input.
	void BuildFX();							// builds effect.
	void BuildVertexLayout();				// builds the input layout for the vertex description.
	void BoundInputParams();
	void ReadInput();

private:
	PrimitiveGenerator::MeshData _pyramid;
	PrimitiveGenerator _generator;

	float _params[3]; // 0 for radius, 1 for height, 2 for sides.
	float _lastParams[3]; 
	float _minRadius, _minHeight;
	int _minSides, _maxSides;

	// the float params (radius & height) will have a different
	// increase value them the int param (sides)
	float incrFloatParam;
	int incrIntParam;

	UINT _inputState; 
	bool _bttn1LastFrame, _bttn2LastFrame, _bttn3LastFrame, _bttnleftLastFrame, _bttnRightLastFrame;

	ID3D11Buffer* _vertexBuffer;
	ID3D11Buffer* _indexBuffer;
	UINT _indexCount;
	UINT _vertexCount;
	UINT _maxIndexCount;
	UINT _maxVertexCount;

	ID3DX11Effect* _effect;
	ID3DX11EffectTechnique* _technique;
	ID3DX11EffectMatrixVariable* _fxWorldViewProj;

	ID3D11InputLayout* _inputLayout;

	ID3D11RasterizerState* _WireframeRS;

	XMFLOAT4X4 _World;
	XMFLOAT4X4 _View;
	XMFLOAT4X4 _Proj;

	float mTheta;
	float mPhi;
	float mRadius;

	POINT mLastMousePos;
};