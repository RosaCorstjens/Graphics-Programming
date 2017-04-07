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
: D3DApp(hInstance), mBoxVB(0), mBoxIB(0), mDiffuseMapSRV(0), mEyePosW(0.0f, 0.0f, 0.0f), 
  mTheta(1.3f*MathHelper::Pi), mPhi(0.4f*MathHelper::Pi), mRadius(25.0f)
{
	mMainWndCaption = L"Crate Demo";
	
	mLastMousePos.x = 0;
	mLastMousePos.y = 0;

	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&mTexTransform, I);
	XMStoreFloat4x4(&mView, I);
	XMStoreFloat4x4(&mProj, I);

	// matrix to set the rotation and position of the phone.
	XMMATRIX translationM = XMMatrixTranslation(-1, 0, -3);
	XMMATRIX rotationM = XMMatrixRotationRollPitchYaw(0, 0, 3.14);
	XMStoreFloat4x4(&mBoxWorld, XMMatrixMultiply(rotationM, translationM));

	mDirLights[0].Ambient  = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	mDirLights[0].Diffuse  = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	mDirLights[0].Specular = XMFLOAT4(0.6f, 0.6f, 0.6f, 16.0f);
	mDirLights[0].Direction = XMFLOAT3(0.707f, -0.707f, 0.0f);
 
	mDirLights[1].Ambient  = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	mDirLights[1].Diffuse  = XMFLOAT4(1.4f, 1.4f, 1.4f, 1.0f);
	mDirLights[1].Specular = XMFLOAT4(0.3f, 0.3f, 0.3f, 16.0f);
	mDirLights[1].Direction = XMFLOAT3(-0.707f, 0.0f, 0.707f);

	mBoxMat.Ambient  = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mBoxMat.Diffuse  = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mBoxMat.Specular = XMFLOAT4(0.6f, 0.6f, 0.6f, 16.0f);
}

/// <summary>
/// Finalizes an instance of the <see cref="TexturesApp"/> class.
/// </summary>
TexturesApp::~TexturesApp()
{
	ReleaseCOM(mBoxVB);
	ReleaseCOM(mBoxIB);
	ReleaseCOM(mDiffuseMapSRV);

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

	HR(D3DX11CreateShaderResourceViewFromFile(md3dDevice, 
		L"MyPhone.png", 0, 0, &mDiffuseMapSRV, 0 ));
 
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
	XMStoreFloat4x4(&mProj, P);
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
	XMStoreFloat4x4(&mView, V);
}

/// <summary>
/// Draws the scene.
/// </summary>
void TexturesApp::DrawScene()
{
	md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&Colors::LightSteelBlue));
	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL, 1.0f, 0);

	md3dImmediateContext->IASetInputLayout(InputLayouts::Basic32);
    md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
 
	UINT stride = sizeof(Vertex::Basic32);
    UINT offset = 0;
 
	XMMATRIX view  = XMLoadFloat4x4(&mView);
	XMMATRIX proj  = XMLoadFloat4x4(&mProj);
	XMMATRIX viewProj = view*proj;

	// Set per frame constants.
	Effects::BasicFX->SetDirLights(mDirLights);
	Effects::BasicFX->SetEyePosW(mEyePosW);
 
	ID3DX11EffectTechnique* activeTech = Effects::BasicFX->Light2TexTech;

    D3DX11_TECHNIQUE_DESC techDesc;
	activeTech->GetDesc( &techDesc );
    for(UINT p = 0; p < techDesc.Passes; ++p)
    {
		md3dImmediateContext->IASetVertexBuffers(0, 1, &mBoxVB, &stride, &offset);
		md3dImmediateContext->IASetIndexBuffer(mBoxIB, DXGI_FORMAT_R32_UINT, 0);

		// Draw the box.
		XMMATRIX world = XMLoadFloat4x4(&mBoxWorld);
		XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
		XMMATRIX worldViewProj = world*view*proj;

		Effects::BasicFX->SetWorld(world);
		Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::BasicFX->SetWorldViewProj(worldViewProj);
		Effects::BasicFX->SetTexTransform(XMLoadFloat4x4(&mTexTransform));
		Effects::BasicFX->SetMaterial(mBoxMat);								// sets the material for the phone
		Effects::BasicFX->SetDiffuseMap(mDiffuseMapSRV);					// sets the texture for the phone

		activeTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
		md3dImmediateContext->DrawIndexed(mBoxIndexCount, 0, 0);
    }

	HR(mSwapChain->Present(0, 0));
}

/// <summary>
/// Builds the geometry buffers.
/// </summary>
void TexturesApp::BuildGeometryBuffers()
{
	// read the .txt file with vertex and index data from the phone
	char* filename = "MyPhone.txt";

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
	fin >> mBoxVertexCount;

	mBoxIndexCount = mBoxVertexCount;

	std::vector<Vertex::Basic32> vertices(mBoxVertexCount);

	// Read up to the beginning of the data.
	fin.get(input);
	while (input != ':')
	{
		fin.get(input);
	}
	fin.get(input);
	fin.get(input);

	// Read in the vertex data.
	for (i = 0; i<mBoxVertexCount; i++)
	{
		fin >> vertices[i].Pos.x >> vertices[i].Pos.y >> vertices[i].Pos.z;
		fin >> vertices[i].Tex.x >> vertices[i].Tex.y;
		fin >> vertices[i].Normal.x >> vertices[i].Normal.y >> vertices[i].Normal.z;
	}

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex::Basic32) * mBoxVertexCount;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &vertices[0];
	HR(md3dDevice->CreateBuffer(&vbd, &vinitData, &mBoxVB));

	// Create the index buffer
	std::vector<UINT> indices;
	for (i = 0; i<mBoxVertexCount; i++)
	{
		indices.push_back(i);
	}

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * mBoxIndexCount;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	ibd.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &indices[0];
	HR(md3dDevice->CreateBuffer(&ibd, &iinitData, &mBoxIB));

	// Close the model file.
	fin.close();
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
		mRadius = MathHelper::Clamp(mRadius, 1.0f, 30.0f);
	}

	mLastMousePos.x = x;
	mLastMousePos.y = y;
}