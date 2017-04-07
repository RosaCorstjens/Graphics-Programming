#pragma once
#include "d3dApp.h"

// structure for a vertex
struct Vertex
{
	XMFLOAT3 Position;		// 3D vector containing a position
	XMFLOAT4 Color;			// 4D vector containing a color
};

// structure for a vertex that describes the pyramid
// used for drawing on GPU
struct PyramidVertex
{
	XMFLOAT3 Position;		// 3D vector containing a position
	XMFLOAT4 Color;			// 4D vector containing a color
	XMFLOAT2 Size;			// 4D vector containing a color
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
	void InitGeometryBuffers();				// initializes dynamic buffers for the vertices and indices.
	void SetGeometryBuffers();			// builds and fills the buffers with a pyramid based on the current input.
	void BuildFX();							// builds effect.

	void BuildPyramidVertexLayout();		// builds the input layout for the vertex description.

	void BoundInputParams();				// methods to read input
	void ReadInput();

private:
	// input variables
	float _params[3];						// 0 for radius, 1 for height, 2 for sides.
	float _lastParams[3]; 

	float _minRadius, _minHeight;
	int _minSides, _maxSides;

	float incrFloatParam; 					// the float params (radius & height) will have a different increase value than the int param (sides)
	int incrIntParam;

	UINT _inputState;						// variables to read input
	bool _bttn1LastFrame, _bttn2LastFrame, 
		_bttn3LastFrame, _bttnleftLastFrame, 
		_bttnRightLastFrame;

	ID3D11Buffer* _vertexBuffer;
	UINT _indexCount;
	UINT _vertexCount;
	UINT _maxIndexCount;
	UINT _maxVertexCount;

	ID3DX11Effect* _effect;
	ID3DX11EffectTechnique* _techniqueThreeSides;
	ID3DX11EffectTechnique* _techniqueFourSides;
	ID3DX11EffectTechnique* _techniqueFiveSides;
	ID3DX11EffectTechnique* _techniqueSixSides;
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