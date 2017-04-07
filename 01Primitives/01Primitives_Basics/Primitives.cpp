#include "Primitives.h"

// main entry point.
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance, PSTR cmdLine, int showCmd)
{
	// enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
	Primitives app(hInstance);
	if (!app.Init()) { return 0; }

	return app.Run();
}

/// <summary>
/// Initializes a new instance of the <see cref="Primitives"/> .
/// </summary>
/// <param name="hInstance">The h instance.</param>
Primitives::Primitives(HINSTANCE hInstance)
	: D3DApp(hInstance), _vertexBuffer(0), _indexBuffer(0), _effect(0), _technique(0),
	_fxWorldViewProj(0), _inputLayout(0), incrIntParam(1), incrFloatParam(0.1f),
	mTheta(1.5f*MathHelper::Pi), mPhi(0.25f*MathHelper::Pi), mRadius(5.0f)
{
	// main window name.
	mMainWndCaption = L"Primitives Application";
	
	mLastMousePos.x = 0;
	mLastMousePos.y = 0;

	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&_World, I);
	XMStoreFloat4x4(&_View, I);
	XMStoreFloat4x4(&_Proj, I);
}

/// <summary>
/// Finalizes an instance of the <see cref="Primitives"/> class.
/// </summary>
Primitives::~Primitives()
{
	ReleaseCOM(_vertexBuffer);
	ReleaseCOM(_indexBuffer);
	ReleaseCOM(_effect);
	ReleaseCOM(_inputLayout);
	ReleaseCOM(_WireframeRS);
}

/// <summary>
/// Initializes this instance.
/// Build the geometry buffers, effect and input layout.
/// </summary>
/// <returns></returns>
bool Primitives::Init()
{
	if (!D3DApp::Init()) { return false; }

	BuildGeometryBuffers();
	BuildFX();
	BuildVertexLayout();

	D3D11_RASTERIZER_DESC wireframeDesc;
	ZeroMemory(&wireframeDesc, sizeof(D3D11_RASTERIZER_DESC));
	wireframeDesc.FillMode = D3D11_FILL_WIREFRAME;
	wireframeDesc.CullMode = D3D11_CULL_BACK;
	wireframeDesc.FrontCounterClockwise = false;
	wireframeDesc.DepthClipEnable = true;

	HR(md3dDevice->CreateRasterizerState(&wireframeDesc, &_WireframeRS));

	return true;
}

/// <summary>
/// Called on resize of the screen.
/// </summary>
void Primitives::OnResize()
{
	D3DApp::OnResize();

	// The window resized, so update the aspect ratio and recompute the projection matrix.
	XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f*MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
	XMStoreFloat4x4(&_Proj, P);
}

/// <summary>
/// Updates the scene.
/// </summary>
/// <param name="dt">The delta time.</param>
void Primitives::UpdateScene(float dt)
{
	// Convert Spherical to Cartesian coordinates.
	float x = mRadius*sinf(mPhi)*cosf(mTheta);
	float z = mRadius*sinf(mPhi)*sinf(mTheta);
	float y = mRadius*cosf(mPhi);

	// Build the view matrix.
	XMVECTOR pos = XMVectorSet(x, y, z, 1.0f);
	XMVECTOR target = XMVectorZero();
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX V = XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat4x4(&_View, V);
}

/// <summary>
/// Draws the scene.
/// </summary>
void Primitives::DrawScene()
{
	md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&Colors::LightSteelBlue));
	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	md3dImmediateContext->IASetInputLayout(_inputLayout);
	md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	md3dImmediateContext->RSSetState(_WireframeRS);

	UINT stride = sizeof(Vertex);
	UINT offset = 0;

	md3dImmediateContext->IASetVertexBuffers(0, 1, &_vertexBuffer, &stride, &offset);
	md3dImmediateContext->IASetIndexBuffer(_indexBuffer, DXGI_FORMAT_R32_UINT, 0);

	// Set constants
	XMMATRIX world = XMLoadFloat4x4(&_World);
	XMMATRIX view = XMLoadFloat4x4(&_View);
	XMMATRIX proj = XMLoadFloat4x4(&_Proj);
	XMMATRIX worldViewProj = world*view*proj;

	_fxWorldViewProj->SetMatrix(reinterpret_cast<float*>(&worldViewProj));

	D3DX11_TECHNIQUE_DESC techDesc;
	_technique->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		_technique->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
		md3dImmediateContext->DrawIndexed(_indexCount, 0, 0);
	}

	HR(mSwapChain->Present(0, 0));
}

/// <summary>
/// Builds the geometry buffers (index and vertex).
/// </summary>
void Primitives::BuildGeometryBuffers()
{
	// create instance of generator and of meshdata
	PrimitiveGenerator generator;
	PrimitiveGenerator::MeshData pyramid;

	// create a new pyramid
	generator.CreatePyramid(1.5f, 1.5f, 10, pyramid);

	// set the counts for drawing and looping
	_vertexCount = pyramid.Vertices.size();
	_indexCount = pyramid.Indices.size();

	// create vector for vertices and fill the vector with the vertex data 
	// from the generator
	std::vector<Vertex> v(_vertexCount);
	XMFLOAT4 black(0.0f, 0.0f, 0.0f, 1.0f);
	for (UINT i = 0; i < pyramid.Vertices.size(); ++i)
	{
		v[i].Position = pyramid.Vertices[i].Position;
		v[i].Color = black;
	}

	// give the top vertex a different color for giggles
	v[0].Color = XMFLOAT4(0, 1, 0, 1.0f);

	// create the vertex buffer description and give it the vector with vertices
	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex) * _vertexCount;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &v[0];
	HR(md3dDevice->CreateBuffer(&vbd, &vinitData, &_vertexBuffer));

	// create a vector for index data.
	std::vector<UINT> in;
	for (UINT i = 0; i < pyramid.Indices.size(); ++i)
	{
		in.push_back(pyramid.Indices[i]);
	}

	// create the index buffer description and give it the vector with indices
	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * _indexCount;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	ibd.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &in[0];
	HR(md3dDevice->CreateBuffer(&ibd, &iinitData, &_indexBuffer));

	return;
}

/// <summary>
/// Builds the fx.
/// </summary>
void Primitives::BuildFX()
{
	DWORD shaderFlags = 0;
#if defined( DEBUG ) || defined( _DEBUG )
	shaderFlags |= D3D10_SHADER_DEBUG;
	shaderFlags |= D3D10_SHADER_SKIP_OPTIMIZATION;
#endif

	ID3D10Blob* compiledShader = 0;
	ID3D10Blob* compilationMsgs = 0;
	HRESULT hr = D3DX11CompileFromFile(L"FX/color.fx", 0, 0, 0, "fx_5_0", shaderFlags,
		0, 0, &compiledShader, &compilationMsgs, 0);

	// compilationMsgs can store errors or warnings.
	if (compilationMsgs != 0)
	{
		MessageBoxA(0, (char*)compilationMsgs->GetBufferPointer(), 0, 0);
		ReleaseCOM(compilationMsgs);
	}

	// Even if there are no compilationMsgs, check to make sure there were no other errors.
	if (FAILED(hr))
	{
		DXTrace(__FILE__, (DWORD)__LINE__, hr, L"D3DX11CompileFromFile", true);
	}

	HR(D3DX11CreateEffectFromMemory(compiledShader->GetBufferPointer(), compiledShader->GetBufferSize(),
		0, md3dDevice, &_effect));

	// Done with compiled shader.
	ReleaseCOM(compiledShader);

	_technique = _effect->GetTechniqueByName("ColorTech");
	_fxWorldViewProj = _effect->GetVariableByName("gWorldViewProj")->AsMatrix();
}

/// <summary>
/// Builds the input layout for vertex.
/// </summary>
void Primitives::BuildVertexLayout()
{
	// create the vertex input layout
	D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },		// position
		{ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }	// color
	};

	// create the input layout
	D3DX11_PASS_DESC passDesc;							// description for the input signature
	_technique->GetPassByIndex(0)->GetDesc(&passDesc);	// get description for this technique.
	HR(md3dDevice->CreateInputLayout(vertexDesc, 2, passDesc.pIAInputSignature,
		passDesc.IAInputSignatureSize, &_inputLayout));
}

void Primitives::OnMouseDown(WPARAM btnState, int x, int y)
{
	mLastMousePos.x = x;
	mLastMousePos.y = y;

	SetCapture(mhMainWnd);
}

void Primitives::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void Primitives::OnMouseMove(WPARAM btnState, int x, int y)
{
	if ((btnState & MK_LBUTTON) != 0)
	{
		// Make each pixel correspond to a quarter of a degree.
		float dx = XMConvertToRadians(0.25f*static_cast<float>(x - mLastMousePos.x));
		float dy = XMConvertToRadians(0.25f*static_cast<float>(y - mLastMousePos.y));

		// Update angles based on input to orbit camera around box.
		mTheta += dx;
		mPhi += dy;

		// Restrict the angle mPhi.
		mPhi = MathHelper::Clamp(mPhi, 0.1f, MathHelper::Pi - 0.1f);
	}
	else if ((btnState & MK_RBUTTON) != 0)
	{
		// Make each pixel correspond to 0.01 unit in the scene.
		float dx = 0.01f*static_cast<float>(x - mLastMousePos.x);
		float dy = 0.01f*static_cast<float>(y - mLastMousePos.y);

		// Update the camera radius based on input.
		mRadius += dx - dy;

		// Restrict the radius.
		mRadius = MathHelper::Clamp(mRadius, 3.0f, 200.0f);
	}

	mLastMousePos.x = x;
	mLastMousePos.y = y;
}