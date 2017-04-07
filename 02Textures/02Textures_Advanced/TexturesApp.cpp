#include "TexturesApp.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
				   PSTR cmdLine, int showCmd)
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

	TexturesApp theApp(hInstance);
	
	if( !theApp.Init() )
		return 0;
	
	return theApp.Run();
}

/// <summary>
/// Initializes a new instance of the <see cref="TexturesApp"/> class.
/// </summary>
/// <param name="hInstance">The h instance.</param>
TexturesApp::TexturesApp(HINSTANCE hInstance)
: D3DApp(hInstance), _vertexBuffer(0), _indexBuffer(0), _phoneMapSRV(0), mEyePosW(0.0f, 0.0f, 0.0f),
  mTheta(1.0f*MathHelper::Pi), mPhi(0.5f*MathHelper::Pi), mRadius(20.0f), _offscreenSRV(0), _renderTargetTexture(0), _offscreenRTV(0)
{
	mMainWndCaption = L"Textures Application";
	
	mLastMousePos.x = 0;
	mLastMousePos.y = 0;

	BuildMatrices();

	_dirLights[0].Ambient  = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	_dirLights[0].Diffuse  = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	_dirLights[0].Specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 16.0f);
	_dirLights[0].Direction = XMFLOAT3(0.707f, -0.707f, 0.0f);
 
	_dirLights[1].Ambient  = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	_dirLights[1].Diffuse  = XMFLOAT4(1.4f, 1.4f, 1.4f, 1.0f);
	_dirLights[1].Specular = XMFLOAT4(0.3f, 0.3f, 0.3f, 16.0f);
	_dirLights[1].Direction = XMFLOAT3(-0.707f, 0.0f, 0.707f);

	SetMaterials();
}

/// <summary>
/// Finalizes an instance of the <see cref="TexturesApp"/> class.
/// </summary>
TexturesApp::~TexturesApp()
{
	ReleaseCOM(_vertexBuffer);
	ReleaseCOM(_indexBuffer);
	ReleaseCOM(_phoneMapSRV);

	ReleaseCOM(_offscreenRTV);
	ReleaseCOM(_offscreenSRV);
	ReleaseCOM(_renderTargetTexture);

	Effects::DestroyAll();
	InputLayouts::DestroyAll();
}

/// <summary>
/// Builds the matrices for positioning and rotating.
/// </summary>
void TexturesApp::BuildMatrices() {
	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&_world, I);
	XMStoreFloat4x4(&_texTransform, I);
	XMStoreFloat4x4(&_view, I);
	XMStoreFloat4x4(&_proj, I);

	XMMATRIX phoneRotation = XMMatrixRotationRollPitchYaw(0, 0, 1.57);
	XMMATRIX phoneOffset = XMMatrixTranslation(0.0f, 1.0f, 0.0f);
	XMStoreFloat4x4(&_phoneWorld, XMMatrixMultiply(phoneRotation, phoneOffset));

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
}

/// <summary>
/// Sets the materials.
/// </summary>
void TexturesApp::SetMaterials() {
	_phoneMaterial.Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	_phoneMaterial.Diffuse = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);
	_phoneMaterial.Specular = XMFLOAT4(0.7f, 0.7f, 0.7f, 16.0f);

	_material.Ambient = XMFLOAT4(0.48f, 0.77f, 0.46f, 1.0f);
	_material.Diffuse = XMFLOAT4(0.48f, 0.77f, 0.46f, 1.0f);
	_material.Specular = XMFLOAT4(0.1f, 0.1f, 0.1f, 16.0f);

	_material2.Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	_material2.Diffuse = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);
	_material2.Specular = XMFLOAT4(0.6f, 0.6f, 0.6f, 16.0f);
}

/// <summary>
/// Initializes this instance.
/// </summary>
/// <returns></returns>
bool TexturesApp::Init()
{
	if(!D3DApp::Init())
		return false;

	// Must init Effects first since InputLayouts depend on shader signatures.
	Effects::InitAll(md3dDevice);
	InputLayouts::InitAll(md3dDevice);

	// only define the texture for the phone itself, since other will be drawn on runtime
	HR(D3DX11CreateShaderResourceViewFromFile(md3dDevice,
		L"MyPhone.png", 0, 0, &_phoneMapSRV, 0));

	BuildGeometryBuffers();

	return true;
}

/// <summary>
/// Called on resize of the screen space.
/// </summary>
void TexturesApp::OnResize()
{
	D3DApp::OnResize();

	BuildOffscreenViews();

	XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f*MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
	XMStoreFloat4x4(&_proj, P);
}

/// <summary>
/// Updates the scene.
/// </summary>
/// <param name="dt">The deltatime.</param>
void TexturesApp::UpdateScene(float dt)
{
	// Convert Spherical to Cartesian coordinates.
	float x = mRadius*sinf(mPhi)*cosf(mTheta);
	float z = mRadius*sinf(mPhi)*sinf(mTheta);
	float y = mRadius*cosf(mPhi);

	mEyePosW = XMFLOAT3(x, y, z);

	// Build the view matrix.
	XMVECTOR pos    = XMVectorSet(x, y, z, 1.0f);
	XMVECTOR target = XMVectorZero();
	XMVECTOR up     = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX V = XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat4x4(&_view, V);

	XMMATRIX phoneRotation = XMMatrixRotationRollPitchYaw(3.14, -mTheta, -1.57);
	XMMATRIX phoneOffset = XMMatrixTranslation(0.0f, 1.0f, 0.0f);
	XMStoreFloat4x4(&_phoneWorld, XMMatrixMultiply(phoneOffset,	phoneRotation));
}

/// <summary>
/// Draws the scene.
/// </summary>
void TexturesApp::DrawScene()
{
	// set the SRV for the screen so that I can render to it
	ID3D11RenderTargetView* renderTargets[1] = { _offscreenRTV };
	md3dImmediateContext->OMSetRenderTargets(1, renderTargets, mDepthStencilView);

	// clear views
	md3dImmediateContext->ClearRenderTargetView(_offscreenRTV, reinterpret_cast<const float*>(&Colors::Silver));
	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	// draw everything in the scene except from the phone
	DrawStart();

	// set SRV to the default render target, the back buffer
	renderTargets[0] = mRenderTargetView;
	md3dImmediateContext->OMSetRenderTargets(1, renderTargets, mDepthStencilView);

	// clear views
	md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&Colors::LightSteelBlue));
	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	// draw everything, including phone with screen texture
	DrawFinish();

	HR(mSwapChain->Present(0, 0));
}

/// <summary>
/// Draws the scene without phone.
/// </summary>
void TexturesApp::DrawStart()
{
	md3dImmediateContext->IASetInputLayout(InputLayouts::Basic32);
	md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	UINT stride = sizeof(Vertex::Basic32);
	UINT offset = 0;

	XMMATRIX view = XMLoadFloat4x4(&_view);
	XMMATRIX proj = XMLoadFloat4x4(&_proj);
	XMMATRIX viewProj = view*proj;

	// Set per frame constants.
	Effects::BasicFX->SetDirLights(_dirLights);
	Effects::BasicFX->SetEyePosW(mEyePosW);

	// Draw the box.
	XMMATRIX world = XMLoadFloat4x4(&_world);
	XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
	XMMATRIX worldViewProj = world*view*proj;

	Effects::BasicFX->SetWorld(world);
	Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
	Effects::BasicFX->SetWorldViewProj(worldViewProj);

	ID3DX11EffectTechnique* activeTech = Effects::BasicFX->Light2Tech;

	D3DX11_TECHNIQUE_DESC techDesc;
	activeTech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		md3dImmediateContext->IASetVertexBuffers(0, 1, &_vertexBuffer, &stride, &offset);
		md3dImmediateContext->IASetIndexBuffer(_indexBuffer, DXGI_FORMAT_R32_UINT, 0);

		for (int i = 0; i < 4; ++i) {
			XMMATRIX world = XMLoadFloat4x4(&_wallsWorld[i]);
			XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
			XMMATRIX worldViewProj = world*view*proj;

			Effects::BasicFX->SetWorld(world);
			Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
			Effects::BasicFX->SetWorldViewProj(worldViewProj);
			Effects::BasicFX->SetMaterial(_material2);

			activeTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
			md3dImmediateContext->DrawIndexed(_wallsIndexCount, _wallsIndexOffset, _wallsVertexOffset);
		}

		for (int i = 0; i < 1; ++i) {
			XMMATRIX world = XMLoadFloat4x4(&_gridsWorld[i]);
			XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
			XMMATRIX worldViewProj = world*view*proj;

			Effects::BasicFX->SetWorld(world);
			Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
			Effects::BasicFX->SetWorldViewProj(worldViewProj);
			Effects::BasicFX->SetMaterial(_material);

			activeTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
			md3dImmediateContext->DrawIndexed(_gridsIndexCount, _gridsIndexOffset, _gridsVertexOffset);
		}

	}
}

/// <summary>
/// Draws the scene including phone.
/// </summary>
void TexturesApp::DrawFinish() {
	md3dImmediateContext->IASetInputLayout(InputLayouts::Basic32);
	md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	UINT stride = sizeof(Vertex::Basic32);
	UINT offset = 0;

	XMMATRIX view = XMLoadFloat4x4(&_view);
	XMMATRIX proj = XMLoadFloat4x4(&_proj);
	XMMATRIX viewProj = view*proj;

	// Set per frame constants.
	Effects::BasicFX->SetDirLights(_dirLights);
	Effects::BasicFX->SetEyePosW(mEyePosW);

	// Draw the box.
	XMMATRIX world = XMLoadFloat4x4(&_world);
	XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
	XMMATRIX worldViewProj = world*view*proj;

	Effects::BasicFX->SetWorld(world);
	Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
	Effects::BasicFX->SetWorldViewProj(worldViewProj);
	Effects::BasicFX->SetTexTransform(XMLoadFloat4x4(&_texTransform));
	Effects::BasicFX->SetMaterial(_phoneMaterial);
	Effects::BasicFX->SetTexture01(_phoneMapSRV);		// sets first texture
	Effects::BasicFX->SetTexture02(_offscreenSRV);		// sets second texture

	ID3DX11EffectTechnique* activeTech = Effects::BasicFX->Light2TexTech;

	D3DX11_TECHNIQUE_DESC techDesc;
	activeTech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		md3dImmediateContext->IASetVertexBuffers(0, 1, &_vertexBuffer, &stride, &offset);
		md3dImmediateContext->IASetIndexBuffer(_indexBuffer, DXGI_FORMAT_R32_UINT, 0);

		XMMATRIX world = XMLoadFloat4x4(&_phoneWorld);
		XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
		XMMATRIX worldViewProj = world*view*proj;
		
		Effects::BasicFX->SetWorld(world);
		Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::BasicFX->SetWorldViewProj(worldViewProj);

		activeTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
		md3dImmediateContext->DrawIndexed(_phoneIndexCount, _phoneIndexOffset, _phoneVertexOffset);

		// other objects don't have textures. 
		activeTech = Effects::BasicFX->Light2Tech;
		activeTech->GetDesc(&techDesc);

		for (int i = 0; i < 4; ++i) {
			world = XMLoadFloat4x4(&_wallsWorld[i]);
			worldInvTranspose = MathHelper::InverseTranspose(world);
			worldViewProj = world*view*proj;

			Effects::BasicFX->SetWorld(world);
			Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
			Effects::BasicFX->SetWorldViewProj(worldViewProj);
			Effects::BasicFX->SetMaterial(_material2);

			activeTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
			md3dImmediateContext->DrawIndexed(_wallsIndexCount, _wallsIndexOffset, _wallsVertexOffset);
		}

		for (int i = 0; i < 1; ++i) {
			XMMATRIX world = XMLoadFloat4x4(&_gridsWorld[i]);
			XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
			XMMATRIX worldViewProj = world*view*proj;

			Effects::BasicFX->SetWorld(world);
			Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
			Effects::BasicFX->SetWorldViewProj(worldViewProj);
			Effects::BasicFX->SetMaterial(_material);

			activeTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
			md3dImmediateContext->DrawIndexed(_gridsIndexCount, _gridsIndexOffset, _gridsVertexOffset);
		}
	}
}

void TexturesApp::OnMouseDown(WPARAM btnState, int x, int y)
{
	mLastMousePos.x = x;
	mLastMousePos.y = y;

	SetCapture(mhMainWnd);
}

void TexturesApp::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void TexturesApp::OnMouseMove(WPARAM btnState, int x, int y)
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
	else if( (btnState & MK_RBUTTON) != 0 )
	{
		// Make each pixel correspond to 0.01 unit in the scene.
		float dx = 0.01f*static_cast<float>(x - mLastMousePos.x);
		float dy = 0.01f*static_cast<float>(y - mLastMousePos.y);

		// Update the camera radius based on input.
		mRadius += dx - dy;

		// Restrict the radius.
		mRadius = MathHelper::Clamp(mRadius, 1.0f, 1000.0f);
	}

	mLastMousePos.x = x;
	mLastMousePos.y = y;
}

/// <summary>
/// Builds the geometry buffers.
/// </summary>
void TexturesApp::BuildGeometryBuffers()
{
	Vertex::Basic32 phoneVertices[] =
	{
		// back
		{ XMFLOAT3(3.65f, 0.0f, 7.35f), XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(0.501f , 0.05f), XMFLOAT2(0,0) },
		{ XMFLOAT3(-3.65f, 0.0f, 7.35f), XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(0.8894f , 0.05f), XMFLOAT2(0,0) },
		{ XMFLOAT3(-3.65f, 0.0f, -7.35f), XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(0.8894f , 0.9348f), XMFLOAT2(0,0) },
		{ XMFLOAT3(-3.65f, 0.0f, -7.35f), XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(0.8894f , 0.9348f), XMFLOAT2(0,0) },
		{ XMFLOAT3(3.65f, 0.0f, -7.35f), XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(0.501f, 0.9348f), XMFLOAT2(0,0) },
		{ XMFLOAT3(3.65f, 0.0f, 7.35f), XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(0.501f , 0.05f), XMFLOAT2(0,0) },

		// front
		{ XMFLOAT3(3.65f, 0.8f, 7.35f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.432f, 0.0513f), XMFLOAT2(-0.093f, -0.0227f) }, // RB
		{ XMFLOAT3(3.65f, 0.8f, -7.35f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.432f, 0.9277f), XMFLOAT2(1.12f, -0.0227f) }, // RO
		{ XMFLOAT3(-3.65f, 0.8f, -7.35f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0135f, 0.9277f), XMFLOAT2(1.12f, 1.0227f) }, // LO
		{ XMFLOAT3(-3.65f, 0.8f, -7.35f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0135f, 0.9277f), XMFLOAT2(1.12f, 1.0227f) }, // LO
		{ XMFLOAT3(-3.65f, 0.8f, 7.35f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0135f, 0.0513f), XMFLOAT2(-0.093f, 1.0227f) }, // LB
		{ XMFLOAT3(3.65f, 0.8f, 7.35f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.432f, 0.0513f), XMFLOAT2(-0.093f, -0.0227f) }, // RB

		// bottom
		{ XMFLOAT3(3.65f, 0.8f, -7.35f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.8931f, 0.9605f), XMFLOAT2(0,0) },
		{ XMFLOAT3(3.65f, 0.0f, -7.35f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.8931f, 0.9921f), XMFLOAT2(0,0) },
		{ XMFLOAT3(-3.65f, 0.0f, -7.35f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.4987f, 0.9921f), XMFLOAT2(0,0) },
		{ XMFLOAT3(-3.65f, 0.0f, -7.35f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.4987f, 0.9921f), XMFLOAT2(0,0) },
		{ XMFLOAT3(-3.65f, 0.8f, -7.35f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.4987f, 0.9605f), XMFLOAT2(0,0) },
		{ XMFLOAT3(3.65f, 0.8f, -7.35f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.8931f, 0.9605f), XMFLOAT2(0,0) },

		// right side
		{ XMFLOAT3(3.65f, 0.8f, 7.35f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(0.4508f, 0.0489f), XMFLOAT2(0,0) },
		{ XMFLOAT3(3.65f, 0.0f, 7.35f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(0.4868f, 0.0489f), XMFLOAT2(0,0) },
		{ XMFLOAT3(3.65f, 0.0f, -7.35f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(0.4868f, 0.9436f), XMFLOAT2(0,0) },
		{ XMFLOAT3(3.65f, 0.0f, -7.35f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(0.4868f, 0.9436f), XMFLOAT2(0,0) },
		{ XMFLOAT3(3.65f, 0.8f, -7.35f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(0.4508f, 0.9436f), XMFLOAT2(0,0) },
		{ XMFLOAT3(3.65f, 0.8f, 7.35f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(0.4508f, 0.0489f), XMFLOAT2(0,0) },

		// top
		{ XMFLOAT3(-3.65f, 0.8f, 7.35f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(0.8949f, 0.00330001f), XMFLOAT2(0,0) },
		{ XMFLOAT3(-3.65f, 0.0f, 7.35f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(0.8949f, 0.0319f), XMFLOAT2(0,0) },
		{ XMFLOAT3(3.65f, 0.0f, 7.35f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(0.5027f, 0.0319f), XMFLOAT2(0,0) },
		{ XMFLOAT3(3.65f, 0.0f, 7.35f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(0.5027f, 0.0319f), XMFLOAT2(0,0) },
		{ XMFLOAT3(3.65f, 0.8f, 7.35f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(0.5027f, 0.00330001f), XMFLOAT2(0,0) },
		{ XMFLOAT3(-3.65f, 0.8f, 7.35f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(0.8949f, 0.00330001f), XMFLOAT2(0,0) },

		// left side
		{ XMFLOAT3(-3.65f, 0.8f, -7.35f), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(0.944f, 0.943f), XMFLOAT2(0,0) },
		{ XMFLOAT3(-3.65f, 0.0f, -7.35f), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(0.9047f, 0.943f), XMFLOAT2(0,0) },
		{ XMFLOAT3(-3.65f, 0.0f, 7.35f), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(0.9047f, 0.0561f), XMFLOAT2(0,0) },
		{ XMFLOAT3(-3.65f, 0.0f, 7.35f), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(0.9047f, 0.0561f), XMFLOAT2(0,0) },
		{ XMFLOAT3(-3.65f, 0.8f, 7.35f), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(0.944f, 0.0561f), XMFLOAT2(0,0) },
		{ XMFLOAT3(-3.65f, 0.8f, -7.35f), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(0.944f, 0.943f), XMFLOAT2(0,0) }
	};

	GeometryGenerator::MeshData wall;
	GeometryGenerator::MeshData grid;
	GeometryGenerator geoGen;

	geoGen.CreateBox(200, 100, 10.0f, wall);
	geoGen.CreateGrid(200, 200, 10, 10, grid);

	_phoneVertexOffset = 0;
	_wallsVertexOffset = 36;
	_gridsVertexOffset = wall.Vertices.size() + _wallsVertexOffset;

	_phoneIndexCount = 36;
	_wallsIndexCount = wall.Indices.size();
	_gridsIndexCount = grid.Indices.size();

	_phoneIndexOffset = 0;
	_wallsIndexOffset = _phoneIndexCount;
	_gridsIndexOffset = _wallsIndexCount + _wallsIndexOffset;

	_vertexCount = 36 + wall.Vertices.size() + grid.Vertices.size();

	_indexCount = _phoneIndexCount +_wallsIndexCount + _gridsIndexCount;

	std::vector<Vertex::Basic32> vertices(_vertexCount);

	UINT k = 0;
	for (size_t i = 0; i < 36; ++i, ++k)
	{
		vertices[k] = phoneVertices[i];
	}
	for (size_t i = 0; i < wall.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = wall.Vertices[i].Position;
		vertices[k].Normal = XMFLOAT3(0, 0, 1);
	}
	for (size_t i = 0; i < grid.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = grid.Vertices[i].Position;
		vertices[k].Normal = XMFLOAT3(0, 1, 0);
	}

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex::Basic32) * _vertexCount;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &vertices[0];
	HR(md3dDevice->CreateBuffer(&vbd, &vinitData, &_vertexBuffer));

	// Create the index buffer
	UINT phoneIndices[] =
	{
		// back
		0, 1, 2,
		3, 4, 5,

		// front
		6, 7, 8,
		9, 10, 11,

		// bottom
		12, 13, 14,
		15, 16, 17,

		// right side
		18, 19, 20,
		21, 22, 23,

		// top
		24, 25, 26,
		27, 28, 29,

		// left side
		30, 31, 32,
		33, 34, 35
	};

	std::vector<UINT> indices;
	// the convertion trick from array to vector didn't seem to work for the indices.. ? it did for the vertices.
	for (int i = 0; i < 36; ++i) {
		indices.push_back(phoneIndices[i]);
	}
	indices.insert(indices.end(), wall.Indices.begin(), wall.Indices.end());
	indices.insert(indices.end(), grid.Indices.begin(), grid.Indices.end());

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * _indexCount;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	ibd.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &indices[0];
	HR(md3dDevice->CreateBuffer(&ibd, &iinitData, &_indexBuffer));
}
 
/// <summary>
/// Builds the resources for the screen rendering.
/// </summary>
void TexturesApp::BuildOffscreenViews()
{
	ReleaseCOM(_offscreenSRV);
	ReleaseCOM(_offscreenRTV);
	ReleaseCOM(_renderTargetTexture);

	// describe the texture to render to
	D3D11_TEXTURE2D_DESC texDesc;
	ZeroMemory(&texDesc, sizeof(texDesc));
	texDesc.Width = mClientWidth;
	texDesc.Height = mClientHeight;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;

	// create the texture 
	HR(md3dDevice->CreateTexture2D(&texDesc, NULL, &_renderTargetTexture));

	// describe render target view
	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
	renderTargetViewDesc.Format = texDesc.Format;
	renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	renderTargetViewDesc.Texture2D.MipSlice = 0;
	
	// create the render target view
	HR(md3dDevice->CreateRenderTargetView(_renderTargetTexture, &renderTargetViewDesc, &_offscreenRTV));

	// describe shader resource view
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
	shaderResourceViewDesc.Format = texDesc.Format;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;

	// create shader resource view
	HR(md3dDevice->CreateShaderResourceView(_renderTargetTexture, &shaderResourceViewDesc, &_offscreenSRV));
}