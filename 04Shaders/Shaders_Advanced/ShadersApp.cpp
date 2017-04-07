#include "d3dApp.h"
#include "ShadersApp.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
	PSTR cmdLine, int showCmd)
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
	ShadersApp theApp(hInstance);

	if (!theApp.Init())
		return 0;

	return theApp.Run();
}


/// <summary>
/// Initializes a new instance of the <see cref="ShadersApp"/> class.
/// </summary>
/// <param name="hInstance">The h instance.</param>
ShadersApp::ShadersApp(HINSTANCE hInstance)
	: D3DApp(hInstance), _vertexBuffer(0), _indexBuffer(0),
	mFX(0), mTech(0), mfxWorld(0), mfxWorldInvTranspose(0), mfxEyePosW(0),
	mfxDirLight(0), mfxPointLight(0), mfxSpotLight(0), mfxMaterial(0),
	mfxWorldViewProj(0), _currentColor(0), _bttnleftLastFrame(false), _bttnRightLastFrame(false),
	mInputLayout(0), mEyePosW(0.0f, 0.0f, 0.0f), mTheta(1.5f*MathHelper::Pi), mPhi(0.45f*MathHelper::Pi), mRadius(500.0f)
{
	mMainWndCaption = L"Fresnel Kitten";

	mLastMousePos.x = 0;
	mLastMousePos.y = 0;

	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&_world, I);
	XMStoreFloat4x4(&mView, I);
	XMStoreFloat4x4(&mProj, I);

	// matrix to set the rotation and position of the phone.
	XMMATRIX translationM = XMMatrixTranslation(0, -50, 0);
	XMMATRIX scaleM = XMMatrixScaling(0.5f, 0.5f, 0.5f);
	XMStoreFloat4x4(&_kittenWorld, XMMatrixMultiply(scaleM, translationM));

	// Directional light.
	_dirLight.Ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	_dirLight.Diffuse = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	_dirLight.Specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	_dirLight.Direction = XMFLOAT3(0.57735f, -0.57735f, 0.57735f);

	_material.Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	_material.Diffuse = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);
	_material.Specular = XMFLOAT4(0.6f, 0.6f, 0.6f, 16.0f);

	_colors[0] = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f); // black
	_colors[1] = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f); // red
	_colors[2] = XMFLOAT4(1.0f, 0.4f, 0.0f, 1.0f); // orange
	_colors[3] = XMFLOAT4(1.0f, 0.8f, 0.0f, 1.0f); // yellow
	_colors[4] = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f); // green
	_colors[5] = XMFLOAT4(0.0f, 0.6f, 1.0f, 1.0f); // blue
	_colors[6] = XMFLOAT4(0.5f, 0.1f, 1.0f, 1.0f); // purple
	_colors[7] = XMFLOAT4(1.0f, 0.1f, 0.7f, 1.0f); // pink
}

/// <summary>
/// Finalizes an instance of the <see cref="ShadersApp"/> class.
/// </summary>
ShadersApp::~ShadersApp()
{
	ReleaseCOM(_vertexBuffer);
	ReleaseCOM(_indexBuffer);

	ReleaseCOM(mFX);
	ReleaseCOM(mInputLayout);
}

/// <summary>
/// Initializes this instance.
/// </summary>
/// <returns></returns>
bool ShadersApp::Init()
{
	if (!D3DApp::Init())
		return false;

	BuildGeometryBuffers();
	BuildFX();
	BuildVertexLayout();

	return true;
}

/// <summary>
/// Called on resize of the screen space.
/// </summary>
void ShadersApp::OnResize()
{
	D3DApp::OnResize();
	XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f*MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
	XMStoreFloat4x4(&mProj, P);
}

/// <summary>
/// Updates the scene.
/// </summary>
/// <param name="dt">The deltatime.</param>
void ShadersApp::UpdateScene(float dt)
{
	// Convert Spherical to Cartesian coordinates.
	float x = mRadius*sinf(mPhi)*cosf(mTheta);
	float z = mRadius*sinf(mPhi)*sinf(mTheta);
	float y = (mRadius*cosf(mPhi));

	mEyePosW = XMFLOAT3(x, y, z);

	// Build the view matrix.
	XMVECTOR pos = XMVectorSet(x, y, z, 1.0f);
	XMVECTOR target = XMVectorZero();
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX V = XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat4x4(&mView, V);

	ReadInput();
}

/// <summary>
/// Draws the scene.
/// </summary>
void ShadersApp::DrawScene()
{
	md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&Colors::LightSteelBlue));
	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	md3dImmediateContext->IASetInputLayout(mInputLayout);
	md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	md3dImmediateContext->IASetVertexBuffers(0, 1, &_vertexBuffer, &stride, &offset);
	md3dImmediateContext->IASetIndexBuffer(_indexBuffer, DXGI_FORMAT_R32_UINT, 0);

	XMMATRIX view = XMLoadFloat4x4(&mView);
	XMMATRIX proj = XMLoadFloat4x4(&mProj);
	XMMATRIX viewProj = view*proj;

	// Set per frame constants.
	mfxDirLight->SetRawValue(&_dirLight, 0, sizeof(_dirLight));
	mfxEyePosW->SetRawValue(&mEyePosW, 0, sizeof(mEyePosW));
	mfxColorPerObject->SetRawValue(&_colors[_currentColor], 0, sizeof(_colors[_currentColor]));

	D3DX11_TECHNIQUE_DESC techDesc;
	mTech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		XMMATRIX world = XMLoadFloat4x4(&_kittenWorld);
		XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
		XMMATRIX worldViewProj = world*view*proj;
		mfxWorld->SetMatrix(reinterpret_cast<float*>(&world));
		mfxWorldInvTranspose->SetMatrix(reinterpret_cast<float*>(&worldInvTranspose));
		mfxWorldViewProj->SetMatrix(reinterpret_cast<float*>(&worldViewProj));
		mfxMaterial->SetRawValue(&_material, 0, sizeof(_material));
		mTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
		md3dImmediateContext->DrawIndexed(_kittenIndexCount, 0, 0);
	}

	HR(mSwapChain->Present(0, 0));
}

/// <summary>
/// Called when [mouse down].
/// </summary>
/// <param name="btnState">State of the BTN.</param>
/// <param name="x">The x.</param>
/// <param name="y">The y.</param>
void ShadersApp::OnMouseDown(WPARAM btnState, int x, int y)
{
	mLastMousePos.x = x;
	mLastMousePos.y = y;

	SetCapture(mhMainWnd);
}

/// <summary>
/// Called when [mouse up].
/// </summary>
/// <param name="btnState">State of the BTN.</param>
/// <param name="x">The x.</param>
/// <param name="y">The y.</param>
void ShadersApp::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

/// <summary>
/// Called when [mouse move].
/// </summary>
/// <param name="btnState">State of the BTN.</param>
/// <param name="x">The x.</param>
/// <param name="y">The y.</param>
void ShadersApp::OnMouseMove(WPARAM btnState, int x, int y)
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
		mRadius = MathHelper::Clamp(mRadius, 3.0f, 1000.0f);
	}
	mLastMousePos.x = x;
	mLastMousePos.y = y;
}

/// <summary>
/// Reads the input.
/// </summary>
void ShadersApp::ReadInput()
{
	// check for increase/decrease in param based on input
	if (GetAsyncKeyState(VK_LEFT) & 0x8000)
	{
		if (!_bttnleftLastFrame)
		{
			// get the selected parameter and substract 1 
			_currentColor--;
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
			_currentColor++;
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
void ShadersApp::BoundInputParams()
{
	if (_currentColor < 0) _currentColor = 7;
	if (_currentColor > 7) _currentColor = 0;
}

/// <summary>
/// Builds the geometry buffers.
/// </summary>
void ShadersApp::BuildGeometryBuffers()
{
	// read the .txt file with vertex and index data from the phone
	char* filename = "kitten.txt";

	std::ifstream fin;
	char input;
	int i;

	// open file
	fin.open(filename);

	// Read up to the value of vertex count.
	fin.get(input);
	while (input != ':')
	{
		fin.get(input);
	}

	// Read in the vertex count.
	fin >> _kittenVertexCount;

	_kittenIndexCount = _kittenVertexCount;

	std::vector<Vertex> vertices(_kittenVertexCount);

	// Read up to the beginning of the data.
	fin.get(input);
	while (input != ':')
	{
		fin.get(input);
	}
	fin.get(input);
	fin.get(input);

	// Read in the vertex data.
	for (i = 0; i<_kittenVertexCount; i++)
	{
		fin >> vertices[i].Pos.x >> vertices[i].Pos.y >> vertices[i].Pos.z;
		fin >> vertices[i].Tex.x >> vertices[i].Tex.y;
		fin >> vertices[i].Normal.x >> vertices[i].Normal.y >> vertices[i].Normal.z;
	}

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex) * _kittenVertexCount;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &vertices[0];
	HR(md3dDevice->CreateBuffer(&vbd, &vinitData, &_vertexBuffer));

	// Create the index buffer
	std::vector<UINT> indices;
	for (i = 0; i<_kittenIndexCount; i++)
	{
		indices.push_back(i);
	}

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * _kittenIndexCount;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	ibd.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &indices[0];
	HR(md3dDevice->CreateBuffer(&ibd, &iinitData, &_indexBuffer));

	// Close the model file.
	fin.close();
}

/// <summary>
/// Builds the fx.
/// </summary>
void ShadersApp::BuildFX()
{
	DWORD shaderFlags = 0;
#if defined( DEBUG ) || defined( _DEBUG )
	shaderFlags |= D3D10_SHADER_DEBUG;
	shaderFlags |= D3D10_SHADER_SKIP_OPTIMIZATION;
#endif

	ID3D10Blob* compiledShader = 0;
	ID3D10Blob* compilationMsgs = 0;
	HRESULT hr = D3DX11CompileFromFile(L"FX/Lighting.fx", 0, 0, 0, "fx_5_0", shaderFlags,
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
		0, md3dDevice, &mFX));

	// Done with compiled shader.
	ReleaseCOM(compiledShader);

	// get the fresnel technique
	mTech = mFX->GetTechniqueByName("FresnelTech");
	mfxWorldViewProj = mFX->GetVariableByName("gWorldViewProj")->AsMatrix();
	mfxWorld = mFX->GetVariableByName("gWorld")->AsMatrix();
	mfxWorldInvTranspose = mFX->GetVariableByName("gWorldInvTranspose")->AsMatrix();
	mfxEyePosW = mFX->GetVariableByName("gEyePosW")->AsVector();
	mfxDirLight = mFX->GetVariableByName("gDirLight");
	mfxSpotLight = mFX->GetVariableByName("gSpotLight");
	mfxMaterial = mFX->GetVariableByName("gMaterial");
	mfxColorPerObject = mFX->GetVariableByName("gColor")->AsVector();
}

/// <summary>
/// Builds the vertex layout.
/// </summary>
void ShadersApp::BuildVertexLayout()
{
	// Create the vertex input layout.
	D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	// Create the input layout
	D3DX11_PASS_DESC passDesc;
	mTech->GetPassByIndex(0)->GetDesc(&passDesc);
	HR(md3dDevice->CreateInputLayout(vertexDesc, 3, passDesc.pIAInputSignature,
		passDesc.IAInputSignatureSize, &mInputLayout));
}