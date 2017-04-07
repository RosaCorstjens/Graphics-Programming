#include "LightingApp.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
				   PSTR cmdLine, int showCmd)
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif
	AllocConsole();
	LightingApp theApp(hInstance);
	
	if( !theApp.Init() )
		return 0;
	
	return theApp.Run();
}
 

/// <summary>
/// Initializes a new instance of the <see cref="LightingApp"/> class.
/// </summary>
/// <param name="hInstance">The h instance.</param>
LightingApp::LightingApp(HINSTANCE hInstance)
: D3DApp(hInstance), _vertexBuffer(0), _indexBuffer(0),
  mFX(0), mTech(0), mfxWorld(0), mfxWorldInvTranspose(0), mfxEyePosW(0), 
  mfxDirLight(0), mfxPointLight(0), mfxSpotLight(0), mfxMaterial(0),
  mfxWorldViewProj(0), _waterTexOffset(0.0f, 0.0f),
  mInputLayout(0), mEyePosW(0.0f, 0.0f, 0.0f), mTheta(1.5f*MathHelper::Pi), mPhi(0.45f*MathHelper::Pi), mRadius(30.0f)
{
	mMainWndCaption = L"Laser Light";
	
	mLastMousePos.x = 0;
	mLastMousePos.y = 0;

	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&_world, I);
	XMStoreFloat4x4(&mView, I);
	XMStoreFloat4x4(&mProj, I);
	
	XMMATRIX texScale = XMMatrixScaling(50.0f, 50.0f, 0.0f);
	XMStoreFloat4x4(&_sandTexTransform, texScale);

	XMStoreFloat4x4(&_gridsWorld, XMMatrixTranslation(0.0f, -5.0f, 0.0f));

	XMMATRIX lightRotate = XMMatrixLookAtLH(XMVectorSet(0.1f, 5.0f, 0.0f, 1.0f), XMVectorSet(0.0f, -10.0f, 0.0f, 0.0f), XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
	XMStoreFloat4x4(&_lightView, lightRotate);

	XMMATRIX lightProj = XMMatrixPerspectiveFovLH((float)(MathHelper::Pi / 1.4f), 1.0f, 0.1f, 100.0f);
	XMStoreFloat4x4(&_lightProj, lightProj);

	// Directional light.
	mDirLights[0].Ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	mDirLights[0].Diffuse = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	mDirLights[0].Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	mDirLights[0].Direction = XMFLOAT3(0.57735f, -0.57735f, 0.57735f);

	mDirLights[1].Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	mDirLights[1].Diffuse = XMFLOAT4(0.01f, 0.01f, 0.01f, 1.0f);
	mDirLights[1].Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	mDirLights[1].Direction = XMFLOAT3(-0.57735f, -0.57735f, 0.57735f);

	mDirLights[2].Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	mDirLights[2].Diffuse = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	mDirLights[2].Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	mDirLights[2].Direction = XMFLOAT3(0.0f, -0.707f, -0.707f);

	pointLight.Ambient = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	pointLight.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	pointLight.Specular = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	pointLight.Att = XMFLOAT3(1.0f, 1.0f, 1.0f);
	pointLight.Position = XMFLOAT3(0.1f, 2.0f, 0.0f);
	pointLight.Range = 10.0f;

	_gridMaterial.Ambient  = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	_gridMaterial.Diffuse  = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	_gridMaterial.Specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 16.0f);
}

/// <summary>
/// Finalizes an instance of the <see cref="LightingApp"/> class.
/// </summary>
LightingApp::~LightingApp()
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
bool LightingApp::Init()
{
	if(!D3DApp::Init())
		return false;

	HR(D3DX11CreateShaderResourceViewFromFile(md3dDevice,
		L"sand.png", 0, 0, &_sandMapSRV, 0));

	HR(D3DX11CreateShaderResourceViewFromFile(md3dDevice,
		L"water.png", 0, 0, &_waterMapSRV, 0));

	BuildGeometryBuffers();
	BuildFX();
	BuildVertexLayout();

	return true;
}

/// <summary>
/// Called when [resize].
/// </summary>
void LightingApp::OnResize()
{
	D3DApp::OnResize();

	XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f*MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
	XMStoreFloat4x4(&mProj, P);
}

/// <summary>
/// Updates the scene.
/// </summary>
/// <param name="dt">The dt.</param>
void LightingApp::UpdateScene(float dt)
{
	// Convert Spherical to Cartesian coordinates.
	float x = mRadius*sinf(mPhi)*cosf(mTheta);
	float z = mRadius*sinf(mPhi)*sinf(mTheta);
	float y = (mRadius*cosf(mPhi));

	mEyePosW = XMFLOAT3(x, y, z);

	// Build the view matrix.
	XMVECTOR pos    = XMVectorSet(x, y, z, 1.0f);
	XMVECTOR target = XMVectorZero();
	XMVECTOR up     = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX V = XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat4x4(&mView, V);

	// Translate texture over time.	
	offsetWater += 0.05f*dt;
	if (offsetWater > 1.0f) offsetWater = 0;
}

/// <summary>
/// Draws the scene.
/// </summary>
void LightingApp::DrawScene()
{
	md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&XMFLOAT4(0.015f, 0.152f, 0.247f, 1.0f)));
	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL, 1.0f, 0);

	md3dImmediateContext->IASetInputLayout(mInputLayout);
    md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
 
	UINT stride = sizeof(Vertex);
    UINT offset = 0;
	md3dImmediateContext->IASetVertexBuffers(0, 1, &_vertexBuffer, &stride, &offset);
	md3dImmediateContext->IASetIndexBuffer(_indexBuffer, DXGI_FORMAT_R32_UINT, 0);

	XMMATRIX view  = XMLoadFloat4x4(&mView);
	XMMATRIX proj  = XMLoadFloat4x4(&mProj);

	XMMATRIX lightview = XMLoadFloat4x4(&_lightView);
	XMMATRIX lightproj = XMLoadFloat4x4(&_lightProj);

	XMMATRIX viewProj = view*proj;

	// Set per frame constants.
	mfxDirLight->SetRawValue(&mDirLights, 0, sizeof(mDirLights));
	mfxPointLight->SetRawValue(&pointLight, 0, sizeof(pointLight));
	mfxOffset->SetFloat(offsetWater);

	mfxProjectionMap->SetResource(_waterMapSRV);

	XMMATRIX pointViewProj = XMLoadFloat4x4(&_lightView) * XMLoadFloat4x4(&_lightProj);
	mfxPointViewProj->SetMatrix(reinterpret_cast<float*>(&pointViewProj));
	mfxEyePosW->SetRawValue(&mEyePosW, 0, sizeof(mEyePosW));
	
	FogColor->SetFloatVector(reinterpret_cast<const float*>(&XMFLOAT4(0.015f, 0.152f, 0.247f, 1.0f)));
	FogStart->SetFloat(10.0f);
	FogRange->SetFloat(200.0f);

    D3DX11_TECHNIQUE_DESC techDesc;
    mTech->GetDesc( &techDesc );
    for(UINT p = 0; p < techDesc.Passes; ++p)
    {
		// grid 1
		// Set per object constants.
		XMMATRIX world = XMLoadFloat4x4(&_gridsWorld);
		XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
		XMMATRIX worldViewProj = world*view*proj;

		mfxWorld->SetMatrix(reinterpret_cast<float*>(&world));
		mfxWorldInvTranspose->SetMatrix(reinterpret_cast<float*>(&worldInvTranspose));
		mfxWorldViewProj->SetMatrix(reinterpret_cast<float*>(&worldViewProj));
		mfxTexTransform->SetMatrix(reinterpret_cast<float*>(&_sandTexTransform));
		mfxMaterial->SetRawValue(&_gridMaterial, 0, sizeof(_gridMaterial));
		mfxDiffuseMap->SetResource(_sandMapSRV);

		mTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
		md3dImmediateContext->DrawIndexed(_gridsIndexCount, _gridsIndexOffset, _gridsVertexOffset);
    }

	HR(mSwapChain->Present(0, 0));
}

/// <summary>
/// Called when [mouse down].
/// </summary>
/// <param name="btnState">State of the BTN.</param>
/// <param name="x">The x.</param>
/// <param name="y">The y.</param>
void LightingApp::OnMouseDown(WPARAM btnState, int x, int y)
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
void LightingApp::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

/// <summary>
/// Called when [mouse move].
/// </summary>
/// <param name="btnState">State of the BTN.</param>
/// <param name="x">The x.</param>
/// <param name="y">The y.</param>
void LightingApp::OnMouseMove(WPARAM btnState, int x, int y)
{
	if( (btnState & MK_LBUTTON) != 0 )
	{
		// Make each pixel correspond to a quarter of a degree.
		float dx = XMConvertToRadians(0.25f*static_cast<float>(x - mLastMousePos.x));
		float dy = XMConvertToRadians(0.25f*static_cast<float>(y - mLastMousePos.y));

		// Update angles based on input to orbit camera around box.
		mTheta += dx;
		mPhi   += dy;

		// Restrict the angle mPhi.
		mPhi = MathHelper::Clamp(mPhi, 0.1f, MathHelper::Pi-0.1f);
	}

	mLastMousePos.x = x;
	mLastMousePos.y = y;
}

/// <summary>
/// Builds the geometry buffers.
/// </summary>
void LightingApp::BuildGeometryBuffers()
{
	GeometryGenerator::MeshData grid;
	GeometryGenerator geoGen;

	geoGen.CreateGrid(1000, 1000, 1000, 1000, grid);

	// Cache the vertex offsets to each object in the concatenated vertex buffer.
	_gridsVertexOffset = 0;

	// Cache the index count of each object.
	_gridsIndexCount = grid.Indices.size();

	// Cache the starting index for each object in the concatenated index buffer.
	_gridsIndexOffset = 0;

	UINT totalVertexCount = grid.Vertices.size();

	_indexCount = _gridsIndexCount;

	//
	// Extract the vertex elements we are interested and apply the height function to
	// each vertex.  
	//
	std::vector<Vertex> vertices(totalVertexCount);

	UINT k = 0;
	for(size_t i = 0; i < grid.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = grid.Vertices[i].Position;
		vertices[k].Normal = XMFLOAT3(0,1,0);
		vertices[k].Texture = grid.Vertices[i].TexC;
	}

    D3D11_BUFFER_DESC vbd;
    vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex) * totalVertexCount;
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = 0;
    vbd.MiscFlags = 0;
    D3D11_SUBRESOURCE_DATA vinitData;
    vinitData.pSysMem = &vertices[0];
    HR(md3dDevice->CreateBuffer(&vbd, &vinitData, &_vertexBuffer));

	//
	// Pack the indices of all the meshes into one index buffer.
	//
	std::vector<UINT> indices;
	indices.insert(indices.end(), grid.Indices.begin(), grid.Indices.end());

	D3D11_BUFFER_DESC ibd;
    ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * _indexCount;
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibd.CPUAccessFlags = 0;
    ibd.MiscFlags = 0;
    D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &indices[0];
    HR(md3dDevice->CreateBuffer(&ibd, &iinitData, &_indexBuffer));
}

/// <summary>
/// Builds the fx.
/// </summary>
void LightingApp::BuildFX()
{
	DWORD shaderFlags = 0;
#if defined( DEBUG ) || defined( _DEBUG )
	shaderFlags |= D3D10_SHADER_DEBUG;
	shaderFlags |= D3D10_SHADER_SKIP_OPTIMIZATION;
#endif

	ID3D10Blob* compiledShader = 0;
	ID3D10Blob* compilationMsgs = 0;
	HRESULT hr = D3DX11CompileFromFile(L"FX/Basic.fx", 0, 0, 0, "fx_5_0", shaderFlags,
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

	mTech                = mFX->GetTechniqueByName("Light3Tex");
	mfxWorldViewProj     = mFX->GetVariableByName("gWorldViewProj")->AsMatrix();
	mfxPointViewProj	 = mFX->GetVariableByName("gWorldViewProj2")->AsMatrix();

	mfxWorld             = mFX->GetVariableByName("gWorld")->AsMatrix();
	mfxWorldInvTranspose = mFX->GetVariableByName("gWorldInvTranspose")->AsMatrix();
	mfxTexTransform      = mFX->GetVariableByName("gTexTransform")->AsMatrix();

	FogColor = mFX->GetVariableByName("gFogColor")->AsVector();
	FogStart = mFX->GetVariableByName("gFogStart")->AsScalar();
	FogRange = mFX->GetVariableByName("gFogRange")->AsScalar();

	mfxEyePosW           = mFX->GetVariableByName("gEyePosW")->AsVector();
	mfxDirLight          = mFX->GetVariableByName("gDirLights");
	mfxPointLight		 = mFX->GetVariableByName("gPointLight");

	mfxMaterial          = mFX->GetVariableByName("gMaterial");
	mfxDiffuseMap		 = mFX->GetVariableByName("gDiffuseMap")->AsShaderResource();
	mfxProjectionMap     = mFX->GetVariableByName("gProjectionTexture")->AsShaderResource();

	mfxOffset			 = mFX->GetVariableByName("offset")->AsScalar();
}

/// <summary>
/// Builds the vertex layout.
/// </summary>
void LightingApp::BuildVertexLayout()
{
	// Create the vertex input layout.
	D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0,  DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	// Create the input layout
    D3DX11_PASS_DESC passDesc;
    mTech->GetPassByIndex(0)->GetDesc(&passDesc);
	HR(md3dDevice->CreateInputLayout(vertexDesc, 3, passDesc.pIAInputSignature, 
		passDesc.IAInputSignatureSize, &mInputLayout));
}