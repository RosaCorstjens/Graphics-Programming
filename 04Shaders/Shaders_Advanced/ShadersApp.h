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
	XMFLOAT2 Tex;
};

class ShadersApp : public D3DApp
{
public:
	ShadersApp(HINSTANCE hInstance);
	~ShadersApp();

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
	void BoundInputParams();
	void ReadInput();

private:
	XMFLOAT4 _colors[8];
	int _currentColor;
	bool _bttnleftLastFrame, _bttnRightLastFrame;

	ID3D11Buffer* _vertexBuffer;
	ID3D11Buffer* _indexBuffer;

	DirectionalLight _dirLight;
	Material _material;

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
	ID3DX11EffectVectorVariable* mfxColorPerObject;

	ID3D11InputLayout* mInputLayout;

	// Define transformations from local spaces to world space.
	XMFLOAT4X4 _kittenWorld;
	XMFLOAT4X4 _world;
	XMFLOAT4X4 mView;
	XMFLOAT4X4 mProj;

	UINT _kittenIndexCount;
	UINT _kittenVertexCount;

	XMFLOAT3 mEyePosW;

	float mTheta;
	float mPhi;
	float mRadius;

	POINT mLastMousePos;
};
