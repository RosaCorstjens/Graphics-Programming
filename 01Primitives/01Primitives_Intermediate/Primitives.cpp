#include "d3dApp.h"
#include "Primitives.h"
#include "MathHelper.h"
#include <math.h>

// main entry point.
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance, PSTR cmdLine, int showCmd)
{
	// enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
	AllocConsole();

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
	_fxWorldViewProj(0), _inputLayout(0), _inputState(0), incrIntParam(1), incrFloatParam(0.1f),
	_minRadius(0.3f), _minHeight(0.3f), _minSides(3), _maxSides(50),
	_bttn1LastFrame(false), _bttn2LastFrame(false), _bttn3LastFrame(false), _bttnleftLastFrame(false), _bttnRightLastFrame(false),
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

	_pyramid.Indices.clear();
	_pyramid.Vertices.clear();
}

/// <summary>
/// Initializes this instance.
/// Build the geometry buffers, effect and input layout.
/// </summary>
/// <returns></returns>
bool Primitives::Init()
{
	if (!D3DApp::Init()) { return false; }

	// array with input parameters.
	_params[0] = 1;
	_params[1] = 1;
	_params[2] = 3;
	std::copy(std::begin(_params), std::end(_params), std::begin(_lastParams));

	InitGeometryBuffers();
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

	ReadInput();

	// if input params changed since last time step
	if (_params[0] != _lastParams[0] || _params[1] != _lastParams[1] || _params[2] != _lastParams[2])
	{
		std::copy(std::begin(_params), std::end(_params), std::begin(_lastParams));

		SetGeometryBuffers();
	}
}

/// <summary>
/// Reads the input.
/// </summary>
void Primitives::ReadInput()
{
	// check for change in input state (1 for height, 2 for radius, 3 for sides)
	if (GetAsyncKeyState('1') & 0x8000)
	{
		if (!_bttn1LastFrame)
		{
			_inputState = 0;
			_bttn1LastFrame = true;
		}
	}
	else if (!GetAsyncKeyState('1'))
	{
		_bttn1LastFrame = false;
	}

	if (GetAsyncKeyState('2') & 0x8000)
	{
		if (!_bttn2LastFrame)
		{
			_inputState = 1;
			_bttn2LastFrame = true;
		}
	}
	else if (!GetAsyncKeyState('2'))
	{
		_bttn2LastFrame = false;
	}

	if (GetAsyncKeyState('3') & 0x8000)
	{
		if (!_bttn3LastFrame)
		{
			_inputState = 2;
			_bttn3LastFrame = true;
		}
	}
	else if (!GetAsyncKeyState('3'))
	{
		_bttn3LastFrame = false;
	}

	// check for increase/decrease in param based on input
	if (GetAsyncKeyState(VK_LEFT) & 0x8000)
	{
		if (!_bttnleftLastFrame)
		{
			// get the selected parameter and substract 1 
			_params[_inputState] -= (_inputState == 2 ? incrIntParam : incrFloatParam);
			_bttnleftLastFrame = true;
		}
	}
	else if (!GetAsyncKeyState(VK_LEFT))
	{
		_bttnleftLastFrame = false;
	}

	if (GetAsyncKeyState(VK_RIGHT) & 0x8000)
	{
		if (!_bttnRightLastFrame)
		{
			// get the selected parameter and add 1 
			_params[_inputState] += (_inputState == 2 ? incrIntParam : incrFloatParam);
			_bttnRightLastFrame = true;
		}
	}
	else if (!GetAsyncKeyState(VK_RIGHT))
	{
		_bttnRightLastFrame = false;
	}

	// make sure the input params are still in bounds. 
	BoundInputParams();
}

/// <summary>
/// Keeps the input parameters in bounds.
/// </summary>
void Primitives::BoundInputParams()
{
	// check first param for being out of range
	if (_params[0] < _minRadius) _params[0] = _minRadius;

	// check sec param for being out of range
	if (_params[1] < _minHeight) _params[1] = _minHeight;

	// check third param for being out of range
	if (_params[2] < _minSides) _params[2] = _minSides;
	if (_params[2] > _maxSides) _params[2] = _maxSides;
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
/// Initializes the vertex and index buffer.
/// </summary>
void Primitives::InitGeometryBuffers()
{
	// first calculate the maximum possible amount of vertices and indices
	// so that the buffers can support every pyramid size that should be supported
	// the first two params don't influence the amount of indices and vertices
	// so just pass the default values for the first two and the max value for the third
	_generator.CreatePyramid(_lastParams[0], _lastParams[1], _maxSides, _pyramid);
	_maxVertexCount = _pyramid.Vertices.size();
	_maxIndexCount = _pyramid.Indices.size();

	// create a pyramid with the default parameters 
	_generator.CreatePyramid(_lastParams[0], _lastParams[1], _lastParams[2], _pyramid);

	// set counts
	_vertexCount = _pyramid.Vertices.size();
	_indexCount = _pyramid.Indices.size();

	// note that I only allocate space, every time step the data will be updated
	// create vertex buffer
	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_DYNAMIC;
	vbd.ByteWidth = sizeof(Vertex) * _maxVertexCount;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vbd.MiscFlags = 0;
	HR(md3dDevice->CreateBuffer(&vbd, 0, &_vertexBuffer));

	// create index buffer
	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_DYNAMIC;
	ibd.ByteWidth = sizeof(UINT) * _maxIndexCount;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	ibd.MiscFlags = 0;
	ibd.StructureByteStride = 0;
	HR(md3dDevice->CreateBuffer(&ibd, 0, &_indexBuffer));

	// build and fill the created buffers
	SetGeometryBuffers();

	return;
}

/// <summary>
/// Sets the data in the vertex buffer.
/// </summary>
void Primitives::SetGeometryBuffers()
{
	// create a new pyramid
	_generator.CreatePyramid(_lastParams[0], _lastParams[1], _lastParams[2], _pyramid);

	// update the vertex buffer with the new input
	D3D11_MAPPED_SUBRESOURCE mappedDataVertex;
	HR(md3dImmediateContext->Map(_vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedDataVertex));

	Vertex* v = reinterpret_cast<Vertex*>(mappedDataVertex.pData);

	XMFLOAT4 black(0.0f, 0.0f, 0.0f, 1.0f);

	for (UINT i = 0; i < _pyramid.Vertices.size(); ++i)
	{
		v[i].Position = _pyramid.Vertices[i].Position;
		v[i].Color = black;
	}

	v[0].Color = XMFLOAT4(0, 1, 0, 1.0f);

	md3dImmediateContext->Unmap(_vertexBuffer, 0);

	// update the index buffers
	D3D11_MAPPED_SUBRESOURCE mappedDataIndex;
	HR(md3dImmediateContext->Map(_indexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedDataIndex));

	UINT* in = reinterpret_cast<UINT*>(mappedDataIndex.pData);

	for (UINT i = 0; i < _pyramid.Indices.size(); ++i)
	{
		in[i] = _pyramid.Indices[i];
	}

	md3dImmediateContext->Unmap(_indexBuffer, 0);

	_vertexCount = _pyramid.Vertices.size();
	_indexCount = _pyramid.Indices.size();

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
/// Builds the input layout depending on vertex struct.
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