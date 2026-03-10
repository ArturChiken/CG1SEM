#pragma once
#include <DirectXMath.h>
#include "Common/MathHelper.h"

using namespace DirectX;

struct Vertex
{
    XMFLOAT3 Pos;
    XMFLOAT4 Color;
    XMFLOAT3 Normal;     // ƒќЅј¬Ћя≈ћ нормаль
    XMFLOAT2 TexC;       // ƒќЅј¬Ћя≈ћ текстурные координаты
};

struct ObjectConstants
{
    XMFLOAT4X4 WorldViewProj = MathHelper::Identity4x4();
};