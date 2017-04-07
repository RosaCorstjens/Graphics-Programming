#pragma once

#include "GeometryGenerator.h"
#include "MathHelper.h"
#include "LightHelper.h"
#include "Waves.h"
#include "d3dApp.h"

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
	void ReadInput();
	void BoundInputParams();

private:
	void BuildGeometryBuffers();
	void BuildFX();
	void BuildVertexLayout();

private:
	ID3D11Buffer* _vertexBuffer;
	ID3D11Buffer* _indexBuffer;

	UINT gridSize;
	UINT wallHeight;

	DirectionalLight _dirLight;
	SpotLight _spotLight;
	Material _gridMaterial;
	Material _wallMaterial;

	float _spotSize; 

	ID3DX11Effect* mFX;
	ID3DX11EffectTechnique* mTech;
	ID3DX11EffectMatrixVariable* mfxWorldViewProj;
	ID3DX11EffectMatrixVariable* mfxWorld;
	ID3DX11EffectMatrixVariable* mfxWorldInvTranspose;
	ID3DX11EffectVectorVariable* mfxEyePosW;
	ID3DX11EffectVariable* mfxDirLight;
	ID3DX11EffectVariable* mfxPointLight;
	ID3DX11EffectVariable* mfxSpotLight;
	ID3DX11EffectVariable* mfxMaterial;

	ID3D11InputLayout* mInputLayout;

	// Define transformations from local spaces to world space.
	XMFLOAT4X4 _world;
	XMFLOAT4X4 mView;
	XMFLOAT4X4 mProj;
	XMFLOAT4X4 _wallsWorld[4];
	XMFLOAT4X4 _gridsWorld[2];

	UINT _indexCount;
	int _wallsVertexOffset;
	int _gridsVertexOffset;
	UINT _wallsIndexOffset;
	UINT _gridsIndexOffset;
	UINT _wallsIndexCount;
	UINT _gridsIndexCount;

	XMFLOAT3 mEyePosW;

	float mTheta;
	float mPhi;
	float mRadius;

	POINT mLastMousePos;
};
