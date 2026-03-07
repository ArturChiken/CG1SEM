#pragma once
#include "GameObject.h"
#include <vector>

class Sphere : public GameObject
{
public:
    Sphere(float radius = 1.0f, int sliceCount = 20, int stackCount = 20);
    virtual ~Sphere() = default;

    virtual void Initialize(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList) override;

    void SetRadius(float radius) { mRadius = radius; }
    float GetRadius() const { return mRadius; }

    void SetColor(const XMFLOAT4& color) { mColor = color; }
    void SetColor(float r, float g, float b, float a) {
        mColor = XMFLOAT4(r, g, b, a);
    }

private:
    void BuildGeometry(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList);

    void GenerateSphereData(std::vector<Vertex>& vertices, std::vector<std::uint16_t>& indices);

private:
    float mRadius;
    int mSliceCount; 
    int mStackCount;
    XMFLOAT4 mColor;
};