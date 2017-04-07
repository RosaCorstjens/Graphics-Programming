#pragma once
#include "d3dApp.h"
#include "d3dx11Effect.h"
#include "MathHelper.h"
#include "LightHelper.h"
#include "GeometryGenerator.h"

struct Vertex
{
	XMFLOAT3 Pos;
	XMFLOAT3 Normal;
};

class LightingApp : public D3DApp
{
public:
	LightingApp(HINSTANCE hInstance);
	~LightingApp();

	bool Init();
	void OnResize();
	void UpdateScene(float dt);
	void DrawScene();

	void OnMouseDown(WPARAM btnState, int x, int y);
	void OnMouseUp(WPARAM btnState, int x, int y);
	void OnMouseMove(WPARAM btnState, int x, int y);

private:
	void BuildGeometryBuffers();
	void BuildFX();
	void BuildVertexLayout();
	XMFLOAT3 GetNormal(float x, float z)const;

private:
	GeometryGenerator::MeshData _wand;
	XMFLOAT3 _lightPos;
	float dx;
	float dy;
	XMMATRIX offsetMatrix;
	bool _lightOn;

	ID3D11Buffer* _vertexBuffer;
	ID3D11Buffer* _indexBuffer;

	Material _wandMaterial;
	Material _wallMaterial;
	DirectionalLight _dirLight;
	PointLight _pointLight;

	ID3DX11Effect* _effect;
	ID3DX11EffectTechnique* _technique;
	ID3DX11EffectMatrixVariable* _fxWorldViewProj;
	ID3DX11EffectMatrixVariable* _fxWorld;
	ID3DX11EffectMatrixVariable* _fxWorldPointLight;
	ID3DX11EffectMatrixVariable* _fxWorldInvTranspose;
	ID3DX11EffectVectorVariable* _fxEyePosW;
	ID3DX11EffectVariable* _fxDirLight;
	ID3DX11EffectVariable* _fxPointLight;
	ID3DX11EffectVariable* _fxMaterial;
	ID3D11InputLayout* _inputLayout;

	// tranformations from local to world.
	XMFLOAT4X4 _wandWorld;
	XMFLOAT4X4 _wallWorld;
	XMFLOAT4X4 _gridWorld;

	XMFLOAT4X4 _world;
	XMFLOAT4X4 _view;
	XMFLOAT4X4 _proj;
	XMFLOAT3 _eyePosW;

	UINT _wandVertexOffset;
	UINT _wallVertexOffset;
	UINT _gridVertexOffset;

	UINT _wandIndexOffset;
	UINT _wallIndexOffset;
	UINT _gridIndexOffset;

	UINT _vertexCountWand;
	UINT _indexCountWand;
	UINT _vertexCountWall;
	UINT _indexCountWall;
	UINT _vertexCountGrid;
	UINT _indexCountGrid;

	float _theta;
	float _phi;
	float _radius;

	POINT _lastMousePos;
};