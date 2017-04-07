#include "LightingApp.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
				   PSTR cmdLine, int showCmd)
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

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
	: D3DApp(hInstance), _vertexBuffer(0), _indexBuffer(0), _effect(0), _technique(0),
	_fxWorldViewProj(0), _inputLayout(0), dx(0), dy(0), _lightOn(true),
  _theta(1.5f*MathHelper::Pi), _phi(0.45f*MathHelper::Pi), _radius(10.0f)
{
	mMainWndCaption = L"Lumos Wand";
	
	_lastMousePos.x = 0;
	_lastMousePos.y = 0;

	// define materials for the wand and wall
	_wandMaterial.Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	_wandMaterial.Diffuse = XMFLOAT4(0.43f, 0.3f, 0.1f, 1.0f);
	_wandMaterial.Specular = XMFLOAT4(0.6f, 0.6f, 0.6f, 16.0f);	
	
	_wallMaterial.Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	_wallMaterial.Diffuse = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);
	_wallMaterial.Specular = XMFLOAT4(0.6f, 0.6f, 0.6f, 16.0f);

	// Set matrices
	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&_gridWorld, I);
	XMStoreFloat4x4(&_world, I);
	XMStoreFloat4x4(&_view, I);
	XMStoreFloat4x4(&_proj, I);

	XMMATRIX wandRotation = XMMatrixRotationRollPitchYaw(0, 0, 20);
	XMMATRIX wandOffset = XMMatrixTranslation(0.0f, 1.0f, 0.0f);
	XMStoreFloat4x4(&_wandWorld, XMMatrixMultiply(wandRotation, wandOffset));

	XMStoreFloat4x4(&_wallWorld, XMMatrixTranslation(0.0f, 0.0f, 1.5f));

	// Direction light
	_dirLight.Ambient = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	_dirLight.Diffuse = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	_dirLight.Specular = XMFLOAT4(0.3f, 0.1f, 0.1f, 16.0f);
	_dirLight.Direction = XMFLOAT3(0.707f, -0.707f, 0.0f);

	// Point light
	_lightPos = XMFLOAT3(0.0f, 3.0f, 0.0f);
	_pointLight.Ambient = XMFLOAT4(0.9f, 0.9f, 0.2f, 1.0f);
	_pointLight.Diffuse = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
	_pointLight.Specular = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	_pointLight.Att = XMFLOAT3(1.0f, 1.0f, 1.0f);
	_pointLight.Range = 2.0f;
	_pointLight.Position = XMFLOAT3(0, 1.8f, 0);

	// set the position of the point light to match the wand
	XMVECTOR poslight = XMLoadFloat3(&_pointLight.Position);
	XMStoreFloat3(&_pointLight.Position, XMVector3TransformCoord(poslight, XMLoadFloat4x4(&_wandWorld)));
	_lightPos = _pointLight.Position;

	// Build the view matrix.
	float x = _radius*sinf(_phi)*cosf(_theta);
	float z = _radius*sinf(_phi)*sinf(_theta);
	float y = _radius*cosf(_phi);

	XMVECTOR pos    = XMVectorSet(x, y, z-3, 1.0f);
	XMVECTOR target = XMVectorZero();
	XMVECTOR up     = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX V = XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat4x4(&_view, V);
}

/// <summary>
/// Finalizes an instance of the <see cref="LightingApp"/> class.
/// </summary>
LightingApp::~LightingApp()
{
	ReleaseCOM(_vertexBuffer);
	ReleaseCOM(_indexBuffer);
	ReleaseCOM(_effect);
	ReleaseCOM(_inputLayout);

	_wand.Indices.clear();
	_wand.Vertices.clear();
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

	// The window resized, so update the aspect ratio and recompute the projection matrix.
	XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f*MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
	XMStoreFloat4x4(&_proj, P);
}

/// <summary>
/// Updates the scene.
/// </summary>
/// <param name="dt">The deltatime.</param>
void LightingApp::UpdateScene(float dt)
{
	_pointLight.Position = _lightPos;
}

/// <summary>
/// Draws the scene.
/// </summary>
void LightingApp::DrawScene()
{
	md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&Colors::LightSteelBlue));
	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	md3dImmediateContext->IASetInputLayout(_inputLayout);
	md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	md3dImmediateContext->IASetVertexBuffers(0, 1, &_vertexBuffer, &stride, &offset);
	md3dImmediateContext->IASetIndexBuffer(_indexBuffer, DXGI_FORMAT_R32_UINT, 0);

	XMMATRIX view = XMLoadFloat4x4(&_view);
	XMMATRIX proj = XMLoadFloat4x4(&_proj);
	XMMATRIX viewProj = view*proj;

	// Set per frame constants.
	_fxDirLight->SetRawValue(&_dirLight, 0, sizeof(_dirLight));
	_fxPointLight->SetRawValue(&_pointLight, 0, sizeof(_pointLight));
	_fxEyePosW->SetRawValue(&_eyePosW, 0, sizeof(_eyePosW));

	D3DX11_TECHNIQUE_DESC techDesc;
	_technique->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		// Draw wand
		// Set per object constants.
		XMMATRIX world = XMLoadFloat4x4(&_wandWorld);
		XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
		XMMATRIX worldViewProj = world*view*proj;

		_fxWorld->SetMatrix(reinterpret_cast<float*>(&(world)));
		_fxWorldInvTranspose->SetMatrix(reinterpret_cast<float*>(&worldInvTranspose));
		_fxWorldViewProj->SetMatrix(reinterpret_cast<float*>(&worldViewProj));
		_fxMaterial->SetRawValue(&_wandMaterial, 0, sizeof(_wandMaterial));

		_technique->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
		md3dImmediateContext->DrawIndexed(_indexCountWand, _wandIndexOffset, _wandVertexOffset);

		// Draw wall
		world = XMLoadFloat4x4(&_wallWorld);
		worldInvTranspose = MathHelper::InverseTranspose(world);
		worldViewProj = world*view*proj;

		_fxWorld->SetMatrix(reinterpret_cast<float*>(&(world)));
		_fxWorldInvTranspose->SetMatrix(reinterpret_cast<float*>(&worldInvTranspose));
		_fxWorldViewProj->SetMatrix(reinterpret_cast<float*>(&worldViewProj));
		_fxMaterial->SetRawValue(&_wallMaterial, 0, sizeof(_wallMaterial));

		_technique->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
		md3dImmediateContext->DrawIndexed(_indexCountWall, _wallIndexOffset, _wallVertexOffset);

		// Draw grid
		world = XMLoadFloat4x4(&_gridWorld);
		worldInvTranspose = MathHelper::InverseTranspose(world);
		worldViewProj = world*view*proj;
		
		_fxWorld->SetMatrix(reinterpret_cast<float*>(&(world)));
		_fxWorldInvTranspose->SetMatrix(reinterpret_cast<float*>(&worldInvTranspose));
		_fxWorldViewProj->SetMatrix(reinterpret_cast<float*>(&worldViewProj));
		_fxMaterial->SetRawValue(&_wallMaterial, 0, sizeof(_wallMaterial));

		_technique->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
		md3dImmediateContext->DrawIndexed(_indexCountGrid, _gridIndexOffset, _gridVertexOffset);
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
	_lastMousePos.x = x;
	_lastMousePos.y = y;

	if (btnState & MK_RBUTTON) {
		_lightOn = !_lightOn;
		if (!_lightOn) _pointLight.Range = 0.0f;
		if (_lightOn) _pointLight.Range = 2.0f;
	}
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
		dx = (x - _lastMousePos.x) * 0.01f;
		dy = (y - _lastMousePos.y) * -0.01f;

		// calculate the new light position
		XMFLOAT3 _newLightPos = _lightPos;
		_newLightPos.x += dx;
		_newLightPos.y += dy;
		_newLightPos.x = MathHelper::Clamp(_newLightPos.x, -6.0f, 6.0f);
		_newLightPos.y = MathHelper::Clamp(_newLightPos.y, 1.5f, 5.0f);

		// get the change in light pos
		dx = _lightPos.x - _newLightPos.x;
		dy = _lightPos.y - _newLightPos.y;

		// update the light position
		_lightPos = _newLightPos;

		// calculate the new wand matrix with the change in light position.
		offsetMatrix = XMMatrixTranslation(-dx, -dy, 0.0f);
		XMMATRIX wandMatrix = XMLoadFloat4x4(&_wandWorld);
		XMStoreFloat4x4(&_wandWorld, XMMatrixMultiply(wandMatrix, offsetMatrix));
	}

	_lastMousePos.x = x;
	_lastMousePos.y = y;
}

/// <summary>
/// Gets the normal vector.
/// </summary>
/// <param name="x">The x.</param>
/// <param name="z">The z.</param>
/// <returns></returns>
XMFLOAT3 LightingApp::GetNormal(float x, float z)const
{
	XMFLOAT3 n(
		-0.03f*z*cosf(0.1f*x) - 0.3f*cosf(0.1f*z),
		1.0f,
		-0.3f*sinf(0.1f*x) + 0.03f*x*sinf(0.1f*z));

	XMVECTOR unitNormal = XMVector3Normalize(XMLoadFloat3(&n));
	XMStoreFloat3(&n, unitNormal);

	return n;
}

/// <summary>
/// Builds the geometry buffers.
/// </summary>
void LightingApp::BuildGeometryBuffers()
{
	// Create vertex buffer
	GeometryGenerator::MeshData wall;
	GeometryGenerator::MeshData grid;

	GeometryGenerator geoGen;
	geoGen.CreateCylinder(0.1f, 0.02f, 3.0f, 20, 20, _wand);
	geoGen.CreateBox(20.0f, 20.0f, 0.1f, wall);
	geoGen.CreateGrid(20.0f, 30.0f, 60, 40, grid);

	// Cache the vertex offsets to each object in the concatenated vertex buffer.
	_wandVertexOffset = 0;
	_wallVertexOffset = _wand.Vertices.size();
	_gridVertexOffset = _wallVertexOffset + wall.Vertices.size();

	_vertexCountWand = _wand.Vertices.size();
	_indexCountWand = _wand.Indices.size();
	_vertexCountWall = wall.Vertices.size();
	_indexCountWall = wall.Indices.size();
	_vertexCountGrid = grid.Vertices.size();
	_indexCountGrid = grid.Indices.size();

	_wandIndexOffset = 0;
	_wallIndexOffset = _indexCountWand;
	_gridIndexOffset = _wallIndexOffset + wall.Indices.size();

	UINT totalVertexCount =
		_vertexCountWand +
		_vertexCountWall +
		_vertexCountGrid;


	UINT totalIndexCount =
		_indexCountWand +
		_indexCountWall +
		_indexCountGrid;


	std::vector<Vertex> vertices(totalVertexCount);
	XMFLOAT4 black(0.0f, 0.0f, 0.0f, 1.0f);

	UINT k = 0;
	for (size_t i = 0; i < _vertexCountWand; ++i, ++k)
	{
		vertices[k].Pos = _wand.Vertices[i].Position;
		vertices[k].Normal = GetNormal(vertices[k].Pos.x, vertices[k].Pos.z);
	}

	for (size_t i = 0; i < _vertexCountWall; ++i, ++k)
	{
		vertices[k].Pos = wall.Vertices[i].Position;
		vertices[k].Normal = GetNormal(vertices[k].Pos.x, vertices[k].Pos.z);
	}

	for (size_t i = 0; i < _vertexCountGrid; ++i, ++k) {
		vertices[k].Pos = grid.Vertices[i].Position;
		vertices[k].Normal = GetNormal(vertices[k].Pos.x, vertices[k].Pos.z);
	}

    D3D11_BUFFER_DESC vbd;
    vbd.Usage = D3D11_USAGE_IMMUTABLE;
    vbd.ByteWidth = sizeof(Vertex) * totalVertexCount;
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = 0;
    vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;
    D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &vertices[0];
    HR(md3dDevice->CreateBuffer(&vbd, &vinitData, &_vertexBuffer));

	// Create the index buffer
	std::vector<UINT> indices;
	indices.insert(indices.end(), _wand.Indices.begin(), _wand.Indices.end());
	indices.insert(indices.end(), wall.Indices.begin(), wall.Indices.end());
	indices.insert(indices.end(), grid.Indices.begin(), grid.Indices.end());

	D3D11_BUFFER_DESC ibd;
    ibd.Usage = D3D11_USAGE_IMMUTABLE;
    ibd.ByteWidth = sizeof(UINT) * totalIndexCount;
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibd.CPUAccessFlags = 0;
    ibd.MiscFlags = 0;
	ibd.StructureByteStride = 0;
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
		0, md3dDevice, &_effect));

	_technique = _effect->GetTechniqueByName("LightTech");
	_fxWorldViewProj = _effect->GetVariableByName("gWorldViewProj")->AsMatrix();
	_fxWorld = _effect->GetVariableByName("gWorld")->AsMatrix();
	_fxWorldInvTranspose = _effect->GetVariableByName("gWorldInvTranspose")->AsMatrix();
	_fxEyePosW = _effect->GetVariableByName("gEyePosW")->AsVector();
	_fxDirLight = _effect->GetVariableByName("gDirLight");
	_fxPointLight = _effect->GetVariableByName("gPointLight");
	_fxMaterial = _effect->GetVariableByName("gMaterial");
}

/// <summary>
/// Builds the vertex layout.
/// </summary>
void LightingApp::BuildVertexLayout()
{
	// Create the vertex input layout.
	D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	// Create the input layout
	D3DX11_PASS_DESC passDesc;
	_technique->GetPassByIndex(0)->GetDesc(&passDesc);
	HR(md3dDevice->CreateInputLayout(vertexDesc, 2, passDesc.pIAInputSignature,
		passDesc.IAInputSignatureSize, &_inputLayout));
}
 