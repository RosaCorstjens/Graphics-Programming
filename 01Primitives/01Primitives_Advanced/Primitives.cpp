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
/// Initializes a new instance of the <see cref="Primitives"/> class.
/// </summary>
/// <param name="hInstance">The h instance.</param>
Primitives::Primitives(HINSTANCE hInstance)
	: D3DApp(hInstance), _vertexBuffer(0), _effect(0), 
	_techniqueThreeSides(0), _techniqueFourSides(0), _techniqueFiveSides(0),
	_fxWorldViewProj(0), _inputLayout(0), _inputState(0), incrIntParam(1), incrFloatParam(0.1f),
	_minRadius(0.3f), _minHeight(0.3f), _minSides(3), _maxSides(6),
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
	ReleaseCOM(_effect);
	ReleaseCOM(_inputLayout);
	ReleaseCOM(_WireframeRS);
}

/// <summary>
/// Initializes this instance.
/// Set the input parameters, build the geometry buffers, effect and input layout.
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
	BuildPyramidVertexLayout();

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

	// read the input
	ReadInput();

	// if input params changed since last time step
	if (_params[0] != _lastParams[0] || _params[1] != _lastParams[1] || _params[2] != _lastParams[2])
	{
		// set the last parameters to new, to enable checking for changes
		std::copy(std::begin(_params), std::end(_params), std::begin(_lastParams));

		// set the geometry buffer with the new input params.
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
	// clear views
	md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&Colors::Silver));
	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	// set constants
	XMMATRIX world = XMLoadFloat4x4(&_World);
	XMMATRIX view = XMLoadFloat4x4(&_View);
	XMMATRIX proj = XMLoadFloat4x4(&_Proj);
	XMMATRIX worldViewProj = world*view*proj;

	// draw pyramid
	md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	md3dImmediateContext->IASetInputLayout(_inputLayout);
	md3dImmediateContext->RSSetState(_WireframeRS);

	UINT stride = sizeof(PyramidVertex);
	UINT offset = 0;

	// bind vertex buffer
	md3dImmediateContext->IASetVertexBuffers(0, 1, &_vertexBuffer, &stride, &offset);

	_fxWorldViewProj->SetMatrix(reinterpret_cast<float*>(&worldViewProj));

	// set the technique for geometry shader based on the 'sides' parameter
	ID3DX11EffectTechnique* pyramidTech;
	int sides = static_cast<int>(_lastParams[2]);
	switch (sides)
	{
	case 3:
		pyramidTech = _techniqueThreeSides;
		break;
	case 4:
		pyramidTech = _techniqueFourSides;
		break;
	case 5:
		pyramidTech = _techniqueFiveSides;
		break;
	case 6:
		pyramidTech = _techniqueSixSides;
		break;
	}

	D3DX11_TECHNIQUE_DESC techDesc;
	pyramidTech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		pyramidTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
		md3dImmediateContext->DrawIndexed(16, 0, 0);
	}

	HR(mSwapChain->Present(0, 0));
}

/// <summary>
/// Initializes the vertex buffer.
/// </summary>
void Primitives::InitGeometryBuffers()
{
	// note that I only allocate space, every time step the data will be updated
	// create vertex buffer
	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_DYNAMIC;
	vbd.ByteWidth = sizeof(PyramidVertex);
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vbd.MiscFlags = 0;
	HR(md3dDevice->CreateBuffer(&vbd, 0, &_vertexBuffer));

	// set the created buffer
	SetGeometryBuffers();

	return;
}

/// <summary>
/// Sets the data in the vertex buffer.
/// </summary>
void Primitives::SetGeometryBuffers()
{
	// create sub resource and map the vertex buffer
	D3D11_MAPPED_SUBRESOURCE mappedDataVertex;
	HR(md3dImmediateContext->Map(_vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedDataVertex));

	// get the mapped data and set the properties
	PyramidVertex* v = reinterpret_cast<PyramidVertex*>(mappedDataVertex.pData);
	v[0].Position = XMFLOAT3(0, 0, 0);
	v[0].Color = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	v[0].Size = XMFLOAT2(_lastParams[0], _lastParams[1]);

	// unmap to enable drawing new pyramid
	md3dImmediateContext->Unmap(_vertexBuffer, 0);

	return;
}

/// <summary>
/// Builds the effect file.
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

	_techniqueThreeSides = _effect->GetTechniqueByName("ThreeSideTech");
	_techniqueFourSides = _effect->GetTechniqueByName("FourSideTech");
	_techniqueFiveSides = _effect->GetTechniqueByName("FiveSideTech");
	_techniqueSixSides = _effect->GetTechniqueByName("SixSideTech");
	_fxWorldViewProj = _effect->GetVariableByName("gWorldViewProj")->AsMatrix();
}

// Builds the input layout to descripe the vertex.
void Primitives::BuildPyramidVertexLayout()
{
	// create the vertex input layout
	D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },		// position
		{ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },	// color
		{ "SIZE",    0, DXGI_FORMAT_R32G32_FLOAT, 0, 28, D3D11_INPUT_PER_VERTEX_DATA, 0 },			// size
	};

	// create the input layout
	D3DX11_PASS_DESC passDesc;							// description for the input signature
	_techniqueThreeSides->GetPassByIndex(0)->GetDesc(&passDesc);	// get description for this technique.
	HR(md3dDevice->CreateInputLayout(vertexDesc, 3, passDesc.pIAInputSignature,
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