#pragma once

#include "d3dApp.h"
#include "d3dx11Effect.h"
#include "GeometryGenerator.h"
#include "MathHelper.h"
#include "LightHelper.h"
#include "Effects.h"
#include "Vertex.h"

class TexturesApp : public D3DApp
{
public:
	TexturesApp(HINSTANCE hInstance);
	~TexturesApp();

	bool Init();
	void OnResize();
	void UpdateScene(float dt);
	void DrawScene();

	void OnMouseDown(WPARAM btnState, int x, int y);
	void OnMouseUp(WPARAM btnState, int x, int y);
	void OnMouseMove(WPARAM btnState, int x, int y);

private:
	void BuildGeometryBuffers();

private:
	ID3D11Buffer* mBoxVB;
	ID3D11Buffer* mBoxIB;

	ID3D11ShaderResourceView* mDiffuseMapSRV;

	DirectionalLight mDirLights[3];
	Material mBoxMat;

	XMFLOAT4X4 mTexTransform;
	XMFLOAT4X4 mBoxWorld;

	XMFLOAT4X4 mView;
	XMFLOAT4X4 mProj;

	int mBoxVertexOffset;
	UINT mBoxIndexOffset;
	UINT mBoxIndexCount;
	UINT mBoxVertexCount;

	XMFLOAT3 mEyePosW;

	float mTheta;
	float mPhi;
	float mRadius;

	POINT mLastMousePos;
};