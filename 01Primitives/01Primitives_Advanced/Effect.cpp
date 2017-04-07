
#include "Effect.h"

#pragma region Effect
Effect::Effect(ID3D11Device* device, const std::wstring& filename)
	: mFX(0)
{
	std::ifstream fin(filename, std::ios::binary);

	fin.seekg(0, std::ios_base::end);
	int size = (int)fin.tellg();
	fin.seekg(0, std::ios_base::beg);
	std::vector<char> compiledShader(size);

	fin.read(&compiledShader[0], size);
	fin.close();

	HR(D3DX11CreateEffectFromMemory(&compiledShader[0], size,
		0, device, &mFX));
}

Effect::~Effect()
{
	ReleaseCOM(mFX);
}
#pragma endregion

#pragma region Effects

Effect*     Effects::TriangleFX = 0;
Effect*		Effects::SqrtFX = 0;
Effect*		Effects::PolyFX = 0;

void Effects::InitAll(ID3D11Device* device)
{
	TriangleFX = new Effect(device, L"FX/color.fxo");
	SqrtFX = new Effect(device, L"FX/color.fxo");
	PolyFX = new Effect(device, L"FX/color.fxo");
}

void Effects::DestroyAll()
{
	SafeDelete(TriangleFX);
	SafeDelete(SqrtFX);
	SafeDelete(PolyFX);
}
#pragma endregion