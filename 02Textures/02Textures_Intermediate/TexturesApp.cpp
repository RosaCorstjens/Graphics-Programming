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
  mTheta(1.3f*MathHelper::Pi), mPhi(0.4f*MathHelper::Pi), mRadius(25.0f)
{
	mMainWndCaption = L"Textures Application";
	
	mLastMousePos.x = 0;
	mLastMousePos.y = 0;

	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&_world, I);
	XMStoreFloat4x4(&_texTransform, I);
	XMStoreFloat4x4(&_view, I);
	XMStoreFloat4x4(&_proj, I);

	_dirLights[0].Ambient  = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	_dirLights[0].Diffuse  = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	_dirLights[0].Specular = XMFLOAT4(0.6f, 0.6f, 0.6f, 16.0f);
	_dirLights[0].Direction = XMFLOAT3(0.707f, -0.707f, 0.0f);
 
	_dirLights[1].Ambient  = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	_dirLights[1].Diffuse  = XMFLOAT4(1.4f, 1.4f, 1.4f, 1.0f);
	_dirLights[1].Specular = XMFLOAT4(0.3f, 0.3f, 0.3f, 16.0f);
	_dirLights[1].Direction = XMFLOAT3(-0.707f, 0.0f, 0.707f);

	_material.Ambient  = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	_material.Diffuse  = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	_material.Specular = XMFLOAT4(0.6f, 0.6f, 0.6f, 16.0f);
}

/// <summary>
/// Finalizes an instance of the <see cref="TexturesApp"/> class.
/// </summary>
TexturesApp::~TexturesApp()
{
	ReleaseCOM(_vertexBuffer);
	ReleaseCOM(_indexBuffer);
	ReleaseCOM(_phoneMapSRV);

	Effects::DestroyAll();
	InputLayouts::DestroyAll();
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

	// build vector with all file names used as textures
	std::vector<std::wstring> phoneFileNames;
	phoneFileNames.push_back(L"MyPhone.png");
	phoneFileNames.push_back(L"MyScreen.png");

	// create texture array from the filenames
	_phoneMapSRV = d3dHelper::CreateTexture2DArraySRV(
		md3dDevice, md3dImmediateContext, phoneFileNames, DXGI_FORMAT_R8G8B8A8_UNORM);

	BuildGeometryBuffers();

	return true;
}

/// <summary>
/// Called on resize of the screen space.
/// </summary>
void TexturesApp::OnResize()
{
	D3DApp::OnResize();

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
}

void TexturesApp::DrawScene()
{
	md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&Colors::LightSteelBlue));
	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL, 1.0f, 0);

	md3dImmediateContext->IASetInputLayout(InputLayouts::Basic32);
    md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
 
	UINT stride = sizeof(Vertex::Basic32);
    UINT offset = 0;
 
	XMMATRIX view  = XMLoadFloat4x4(&_view);
	XMMATRIX proj  = XMLoadFloat4x4(&_proj);
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
	Effects::BasicFX->SetMaterial(_material);
	Effects::BasicFX->SetTextureMapArray(_phoneMapSRV);					// set the texture array

	ID3DX11EffectTechnique* activeTech = Effects::BasicFX->Light2TexTech;

    D3DX11_TECHNIQUE_DESC techDesc;
	activeTech->GetDesc( &techDesc );
    for(UINT p = 0; p < techDesc.Passes; ++p)
    {
		md3dImmediateContext->IASetVertexBuffers(0, 1, &_vertexBuffer, &stride, &offset);
		md3dImmediateContext->IASetIndexBuffer(_indexBuffer, DXGI_FORMAT_R32_UINT, 0);

		activeTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
		md3dImmediateContext->DrawIndexed(_indexCount, 0, 0);
    }

	HR(mSwapChain->Present(0, 0));
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
		mRadius = MathHelper::Clamp(mRadius, 1.0f, 30.0f);
	}

	mLastMousePos.x = x;
	mLastMousePos.y = y;
}

/// <summary>
/// Builds the geometry buffers.
/// </summary>
void TexturesApp::BuildGeometryBuffers()
{
	// vertices for the phone with position, normal and two textures
	Vertex::Basic32 vertices[] =
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

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex::Basic32) * 36;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = vertices;
	HR(md3dDevice->CreateBuffer(&vbd, &vinitData, &_vertexBuffer));

	// Create the index buffer
	UINT indices[] =
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

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * 36;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	ibd.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = indices;
	HR(md3dDevice->CreateBuffer(&ibd, &iinitData, &_indexBuffer));

	_indexCount = 36;
	_vertexCount = 36;
}
 
