#include "Sphere.h"
#include "Common/d3dUtil.h"
#include <cmath>
#include <vector>

Sphere::Sphere(float radius, int sliceCount, int stackCount)
    : GameObject()
    , mRadius(radius)
    , mSliceCount(sliceCount)
    , mStackCount(stackCount)
    , mColor(1.0f, 1.0f, 1.0f, 1.0f)
{
    mName = "Sphere";
}

void Sphere::Initialize(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList)
{
    BuildGeometry(device, cmdList);
    mIsInitialized = true;
}

void Sphere::GenerateSphereData(std::vector<Vertex>& vertices, std::vector<std::uint16_t>& indices)
{
    vertices.clear();
    indices.clear();

    float phiStep = XM_PI / mStackCount;
    float thetaStep = 2.0f * XM_PI / mSliceCount;

    for (int i = 0; i <= mStackCount; ++i)
    {
        float phi = i * phiStep;

        for (int j = 0; j <= mSliceCount; ++j)
        {
            float theta = j * thetaStep;

            float x = mRadius * sinf(phi) * cosf(theta);
            float y = mRadius * cosf(phi);
            float z = mRadius * sinf(phi) * sinf(theta);

            Vertex v;
            v.Pos = XMFLOAT3(x, y, z);

            float brightness = 0.5f + 0.5f * cosf(phi);
            v.Color.x = mColor.x * (0.3f + 0.7f * brightness);
            v.Color.y = mColor.y * (0.3f + 0.7f * brightness);
            v.Color.z = mColor.z * (0.3f + 0.7f * brightness);
            v.Color.w = mColor.w;

            vertices.push_back(v);
        }
    }

    for (int i = 0; i < mStackCount; ++i)
    {
        for (int j = 0; j < mSliceCount; ++j)
        {
            int first = i * (mSliceCount + 1) + j;
            int second = first + mSliceCount + 1;

            indices.push_back(first);
            indices.push_back(second);
            indices.push_back(first + 1);
            indices.push_back(second);
            indices.push_back(second + 1);
            indices.push_back(first + 1);
        }
    }
}

void Sphere::BuildGeometry(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList)
{
    std::vector<Vertex> vertices;
    std::vector<std::uint16_t> indices;

    GenerateSphereData(vertices, indices);

    const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
    const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

    mMeshGeometry = std::make_unique<MeshGeometry>();
    mMeshGeometry->Name = mName + "Geometry";

    // Создаем CPU буферы
    ThrowIfFailed(D3DCreateBlob(vbByteSize, &mMeshGeometry->VertexBufferCPU));
    CopyMemory(mMeshGeometry->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

    ThrowIfFailed(D3DCreateBlob(ibByteSize, &mMeshGeometry->IndexBufferCPU));
    CopyMemory(mMeshGeometry->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

    // Создаем GPU буферы
    mMeshGeometry->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(device,
        cmdList, vertices.data(), vbByteSize, mMeshGeometry->VertexBufferUploader);

    mMeshGeometry->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(device,
        cmdList, indices.data(), ibByteSize, mMeshGeometry->IndexBufferUploader);

    mMeshGeometry->VertexByteStride = sizeof(Vertex);
    mMeshGeometry->VertexBufferByteSize = vbByteSize;
    mMeshGeometry->IndexFormat = DXGI_FORMAT_R16_UINT;
    mMeshGeometry->IndexBufferByteSize = ibByteSize;

    SubmeshGeometry submesh;
    submesh.IndexCount = (UINT)indices.size();
    submesh.StartIndexLocation = 0;
    submesh.BaseVertexLocation = 0;

    mMeshGeometry->DrawArgs["sphere"] = submesh;
}