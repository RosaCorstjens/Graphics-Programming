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
  mfxWorldViewProj(0), 
  mInputLayout(0), mEyePosW(0.0f, 0.0f, 0.0f), mTheta(1.5f*MathHelper::Pi), mPhi(0.45f*MathHelper::Pi), mRadius(1.0f)
{
	mMainWndCaption = L"Laser Light";
	
	mLastMousePos.x = 0;
	mLastMousePos.y = 0;

	gridSize = 50;
	wallHeight = 100;

	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&_world, I);
	XMStoreFloat4x4(&mView, I);
	XMStoreFloat4x4(&mProj, I);

	XMMATRIX wallRotation = XMMatrixRotationRollPitchYaw(0, -1.57, 0);
	XMMATRIX wallOffset = XMMatrixTranslation(100, 0.0f, 0.0f);
	XMStoreFloat4x4(&_wallsWorld[0], XMMatrixMultiply(wallRotation, wallOffset));
	wallRotation = XMMatrixRotationRollPitchYaw(0, 1.57, 0);
	wallOffset = XMMatrixTranslation(-100, 0.0f, 0.0f);
	XMStoreFloat4x4(&_wallsWorld[1], XMMatrixMultiply(wallRotation, wallOffset));
	wallRotation = XMMatrixRotationRollPitchYaw(0, 3.14, 0);
	wallOffset = XMMatrixTranslation(0.0f, 0.0f, 100);
	XMStoreFloat4x4(&_wallsWorld[2], XMMatrixMultiply(wallRotation, wallOffset));
	XMStoreFloat4x4(&_wallsWorld[3], XMMatrixTranslation(0.0f, 0.0f, -100));

	XMStoreFloat4x4(&_gridsWorld[0], XMMatrixTranslation(0.0f, -10.0f, 0.0f));

	// Directional light.
	_dirLight.Ambient  = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	_dirLight.Diffuse  = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	_dirLight.Specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	_dirLight.Direction = XMFLOAT3(0.57735f, -0.57735f, 0.57735f);

	// Spot light--position and direction changed every frame to animate in UpdateScene function.
	_spotLight.Ambient  = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
	_spotLight.Diffuse  = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
	_spotLight.Specular = XMFLOAT4(0.4f, 0.4f, 0.4f, 5.0f);
	_spotLight.Att      = XMFLOAT3(1.0f, 0.0f, 0.0f);
	_spotLight.Spot     = 5000.0f;
	_spotLight.Range    = 10000.0f;

	_gridMaterial.Ambient  = XMFLOAT4(0.48f, 0.77f, 0.46f, 1.0f);
	_gridMaterial.Diffuse  = XMFLOAT4(0.48f, 0.77f, 0.46f, 1.0f);
	_gridMaterial.Specular = XMFLOAT4(0.1f, 0.1f, 0.1f, 16.0f);

	_wallMaterial.Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	_wallMaterial.Diffuse = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);
	_wallMaterial.Specular = XMFLOAT4(0.6f, 0.6f, 0.6f, 16.0f);

	// set the spot size var to the current size.
	_spotSize = _spotLight.Spot;
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
	ReadInput();

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

	// Change size of spot light.
	_spotLight.Spot = _spotSize;

	// The spotlight takes on the camera position and is aimed in the
	// same direction the camera is looking.  In this way, it looks
	// like we are holding a flashlight.
	_spotLight.Position = mEyePosW;
	XMStoreFloat3(&_spotLight.Direction, XMVector3Normalize(target - pos));
}

/// <summary>
/// Draws the scene.
/// </summary>
void LightingApp::DrawScene()
{
	md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&Colors::LightSteelBlue));
	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL, 1.0f, 0);

	md3dImmediateContext->IASetInputLayout(mInputLayout);
    md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
 
	UINT stride = sizeof(Vertex);
    UINT offset = 0;
	md3dImmediateContext->IASetVertexBuffers(0, 1, &_vertexBuffer, &stride, &offset);
	md3dImmediateContext->IASetIndexBuffer(_indexBuffer, DXGI_FORMAT_R32_UINT, 0);

	XMMATRIX view  = XMLoadFloat4x4(&mView);
	XMMATRIX proj  = XMLoadFloat4x4(&mProj);
	XMMATRIX viewProj = view*proj;

	// Set per frame constants.
	mfxDirLight->SetRawValue(&_dirLight, 0, sizeof(_dirLight));
	mfxSpotLight->SetRawValue(&_spotLight, 0, sizeof(_spotLight));
	mfxEyePosW->SetRawValue(&mEyePosW, 0, sizeof(mEyePosW));
 
    D3DX11_TECHNIQUE_DESC techDesc;
    mTech->GetDesc( &techDesc );
    for(UINT p = 0; p < techDesc.Passes; ++p)
    {
		// grid 1
		// Set per object constants.
		for(int i = 0; i < 1; ++i) {
			XMMATRIX world = XMLoadFloat4x4(&_gridsWorld[i]);
			XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
			XMMATRIX worldViewProj = world*view*proj;

			mfxWorld->SetMatrix(reinterpret_cast<float*>(&world));
			mfxWorldInvTranspose->SetMatrix(reinterpret_cast<float*>(&worldInvTranspose));
			mfxWorldViewProj->SetMatrix(reinterpret_cast<float*>(&worldViewProj));
			mfxMaterial->SetRawValue(&_gridMaterial, 0, sizeof(_gridMaterial));

			mTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
			md3dImmediateContext->DrawIndexed(_gridsIndexCount, _gridsIndexOffset, _gridsVertexOffset);
		}
		
		for (int i = 0; i < 4; ++i) {
			XMMATRIX world = XMLoadFloat4x4(&_wallsWorld[i]);
			XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
			XMMATRIX worldViewProj = world*view*proj;

			mfxWorld->SetMatrix(reinterpret_cast<float*>(&world));
			mfxWorldInvTranspose->SetMatrix(reinterpret_cast<float*>(&worldInvTranspose));
			mfxWorldViewProj->SetMatrix(reinterpret_cast<float*>(&worldViewProj));
			mfxMaterial->SetRawValue(&_wallMaterial, 0, sizeof(_wallMaterial));

			mTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
			md3dImmediateContext->DrawIndexed(_wallsIndexCount, 0, 0);
		}
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
/// Reads the input.
/// </summary>
void LightingApp::ReadInput()
{
	// check for increase/decrease in param based on input
	if (GetAsyncKeyState(VK_RIGHT) & 0x8000)
	{
		// get the selected parameter and substract 1 
		_spotSize -= 1;
	}

	if (GetAsyncKeyState(VK_LEFT) & 0x8000)
	{
		_spotSize += 1;
	}

	// make sure the input params are still in bounds. 
	BoundInputParams();
}

/// <summary>
/// Bounds the input parameters.
/// </summary>
void LightingApp::BoundInputParams()
{
	// check param for being out of range
	if (_spotSize < 100) _spotSize = 100;
	else if (_spotSize > 10000) _spotSize = 10000;
}

/// <summary>
/// Builds the geometry buffers.
/// </summary>
void LightingApp::BuildGeometryBuffers()
{
	GeometryGenerator::MeshData wall;
	GeometryGenerator::MeshData grid;
	GeometryGenerator geoGen;

	geoGen.CreateBox(200, wallHeight, 10.0f, wall);
	geoGen.CreateGrid(200, 200, 200, 200, grid);

	// Cache the vertex offsets to each object in the concatenated vertex buffer.
	_wallsVertexOffset = 0;
	_gridsVertexOffset = wall.Vertices.size();

	// Cache the index count of each object.
	_wallsIndexCount = wall.Indices.size();
	_gridsIndexCount = grid.Indices.size();

	// Cache the starting index for each object in the concatenated index buffer.
	_wallsIndexOffset = 0;
	_gridsIndexOffset = _wallsIndexCount;

	UINT totalVertexCount =
		wall.Vertices.size() +
		grid.Vertices.size();

	_indexCount = _wallsIndexCount + _gridsIndexCount;

	//
	// Extract the vertex elements we are interested and apply the height function to
	// each vertex.  
	//
	std::vector<Vertex> vertices(totalVertexCount);

	UINT k = 0;
	for (size_t i = 0; i < wall.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = wall.Vertices[i].Position;
		vertices[k].Normal = XMFLOAT3(0, 0, 1);
	}
	for(size_t i = 0; i < grid.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = grid.Vertices[i].Position;
		vertices[k].Normal = XMFLOAT3(0,1,0);
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
	indices.insert(indices.end(), wall.Indices.begin(), wall.Indices.end());
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
	std::ifstream fin("fx/Lighting.fxo", std::ios::binary);

	fin.seekg(0, std::ios_base::end);
	int size = (int)fin.tellg();
	fin.seekg(0, std::ios_base::beg);
	std::vector<char> compiledShader(size);

	fin.read(&compiledShader[0], size);
	fin.close();
	
	HR(D3DX11CreateEffectFromMemory(&compiledShader[0], size, 
		0, md3dDevice, &mFX));

	mTech                = mFX->GetTechniqueByName("LightTech");
	mfxWorldViewProj     = mFX->GetVariableByName("gWorldViewProj")->AsMatrix();
	mfxWorld             = mFX->GetVariableByName("gWorld")->AsMatrix();
	mfxWorldInvTranspose = mFX->GetVariableByName("gWorldInvTranspose")->AsMatrix();
	mfxEyePosW           = mFX->GetVariableByName("gEyePosW")->AsVector();
	mfxDirLight          = mFX->GetVariableByName("gDirLight");
	mfxSpotLight         = mFX->GetVariableByName("gSpotLight");
	mfxMaterial          = mFX->GetVariableByName("gMaterial");
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
		{"NORMAL",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	// Create the input layout
    D3DX11_PASS_DESC passDesc;
    mTech->GetPassByIndex(0)->GetDesc(&passDesc);
	HR(md3dDevice->CreateInputLayout(vertexDesc, 2, passDesc.pIAInputSignature, 
		passDesc.IAInputSignatureSize, &mInputLayout));
}