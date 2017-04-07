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
	void BuildGeometryBuffers();			// builds and fills the buffers with a pyramid based on the current input.
	void BuildFX();							// builds effect.
	void BuildVertexLayout();				// builds the input layout for the vertex description.

private:
	// the float params (radius & height) will have a different
	// increase value them the int param (sides)
	float incrFloatParam;
	int incrIntParam;

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