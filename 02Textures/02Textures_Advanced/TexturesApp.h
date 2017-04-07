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
	void DrawStart();
	void DrawFinish();
	void BuildGeometryBuffers();
	void BuildOffscreenViews();
	void BuildMatrices();
	void SetMaterials();

private:
	// buffers containing geometry
	ID3D11Buffer* _vertexBuffer;
	ID3D11Buffer* _indexBuffer;

	// texture for the phone itself
	ID3D11ShaderResourceView* _phoneMapSRV;

	// render-to-texture variables to render the screen
	ID3D11ShaderResourceView* _offscreenSRV;				
	ID3D11Texture2D* _renderTargetTexture;					
	ID3D11RenderTargetView* _offscreenRTV;

	// lights
	DirectionalLight _dirLights[3];

	// materials
	Material _phoneMaterial;
	Material _material;
	Material _material2;

	// matrices
	XMFLOAT4X4 _texTransform;
	XMFLOAT4X4 _phoneWorld;
	XMFLOAT4X4 _wallsWorld[4];
	XMFLOAT4X4 _gridsWorld[2];
	XMFLOAT4X4 _world;
	XMFLOAT4X4 _view;
	XMFLOAT4X4 _proj;

	UINT _indexCount;
	UINT _vertexCount;

	int _phoneVertexOffset;
	int _wallsVertexOffset;
	int _gridsVertexOffset;

	UINT _phoneIndexOffset;
	UINT _wallsIndexOffset;
	UINT _gridsIndexOffset;

	UINT _phoneIndexCount;
	UINT _wallsIndexCount;
	UINT _gridsIndexCount;

	XMFLOAT3 mEyePosW;

	float mTheta;
	float mPhi;
	float mRadius;

	POINT mLastMousePos;
};