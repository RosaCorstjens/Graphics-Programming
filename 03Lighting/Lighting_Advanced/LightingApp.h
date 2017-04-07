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
	XMFLOAT2 Texture;
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

private:
	ID3D11Buffer* _vertexBuffer;
	ID3D11Buffer* _indexBuffer;

	UINT gridSize;
	UINT wallHeight;

	DirectionalLight mDirLights[3];
	PointLight pointLight;
	Material _gridMaterial;

	ID3D11ShaderResourceView* _sandMapSRV;
	ID3D11ShaderResourceView* _waterMapSRV;
	float offsetWater;

	ID3DX11Effect* mFX;
	ID3DX11EffectTechnique* mTech;
	ID3DX11EffectMatrixVariable* mfxWorldViewProj;
	ID3DX11EffectMatrixVariable* mfxPointViewProj;

	ID3DX11EffectMatrixVariable* mfxWorld;
	ID3DX11EffectMatrixVariable* mfxWorldInvTranspose;
	ID3DX11EffectMatrixVariable* mfxTexTransform;
	ID3DX11EffectVectorVariable* mfxEyePosW;

	ID3DX11EffectVectorVariable* FogColor;
	ID3DX11EffectScalarVariable* FogStart;
	ID3DX11EffectScalarVariable* FogRange;

	ID3DX11EffectVariable* mfxDirLight;
	ID3DX11EffectVariable* mfxPointLight;
	ID3DX11EffectVariable* mfxSpotLight;
	ID3DX11EffectVariable* mfxMaterial;
	ID3DX11EffectShaderResourceVariable* mfxDiffuseMap;
	ID3DX11EffectShaderResourceVariable* mfxProjectionMap;

	ID3DX11EffectScalarVariable* mfxOffset;

	ID3D11InputLayout* mInputLayout;

	// Define transformations from local spaces to world space.
	XMFLOAT4X4 _world;
	XMFLOAT4X4 mView;
	XMFLOAT4X4 mProj;
	XMFLOAT4X4 _gridsWorld;
	XMFLOAT4X4 _sandTexTransform;

	XMFLOAT4X4 _lightView;
	XMFLOAT4X4 _lightProj;
	UINT _indexCount;
	int _gridsVertexOffset;
	UINT _gridsIndexOffset;
	UINT _gridsIndexCount;
	XMFLOAT2 _waterTexOffset;

	XMFLOAT3 mEyePosW;

	float mTheta;
	float mPhi;
	float mRadius;

	POINT mLastMousePos;
};
