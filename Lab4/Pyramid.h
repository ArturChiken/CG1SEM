#pragma once
#include "GameObject.h"
#include <vector>

class Pyramid : public GameObject
{
public:
    Pyramid(float size = 1.0f, float height = 1.5f);
    virtual ~Pyramid() = default;

    virtual void Initialize(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList) override;

    void SetSize(float size) { mSize = size; }
    void SetHeight(float height) { mHeight = height; }
    void SetColor(const XMFLOAT4& color) { mColor = color; }
    void SetColor(float r, float g, float b, float a) {
        mColor = XMFLOAT4(r, g, b, a);
    }

private:
    void BuildGeometry(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList);
    void GeneratePyramidData(std::vector<Vertex>& vertices, std::vector<std::uint16_t>& indices);

private:
    float mSize;
    float mHeight;
    XMFLOAT4 mColor;
};