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
	ID3D11Buffer* _vertexBuffer;
	ID3D11Buffer* _indexBuffer;

	ID3D11ShaderResourceView* _phoneMapSRV;

	DirectionalLight _dirLights[3];
	Material _material;

	XMFLOAT4X4 _texTransform;
	XMFLOAT4X4 _world;
	XMFLOAT4X4 _view;
	XMFLOAT4X4 _proj;

	UINT _vertexOffset;
	UINT _indexOffset;
	UINT _indexCount;
	UINT _vertexCount;

	XMFLOAT3 mEyePosW;

	float mTheta;
	float mPhi;
	float mRadius;

	POINT mLastMousePos;
};