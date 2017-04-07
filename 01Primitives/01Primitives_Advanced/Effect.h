
#ifndef EFFECTS_H
#define EFFECTS_H

#include "d3dUtil.h"

#pragma region Effect
class Effect
{
public:
	Effect(ID3D11Device* device, const std::wstring& filename);
	virtual ~Effect();

private:
	Effect(const Effect& rhs);
	Effect& operator=(const Effect& rhs);

protected:
	ID3DX11Effect* mFX;
};
#pragma endregion

#pragma region Effects
class Effects
{
public:
	static void InitAll(ID3D11Device* device);
	static void DestroyAll();

	static Effect* TriangleFX;
	static Effect* SqrtFX;
	static Effect* PolyFX;
};
#pragma endregion

#endif // EFFECTS_H