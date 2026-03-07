#include "Pyramid.h"
#include "Common/d3dUtil.h"
#include <cmath>

Pyramid::Pyramid(float size, float height)
    : GameObject()
    , mSize(size)
    , mHeight(height)
    , mColor(1.0f, 1.0f, 0.0f, 1.0f)
{
    mName = "Pyramid";
}

void Pyramid::Initialize(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList)
{
    BuildGeometry(device, cmdList);
    mIsInitialized = true;
}

void Pyramid::GeneratePyramidData(std::vector<Vertex>& vertices, std::vector<std::uint16_t>& indices)
{
    vertices.clear();
    indices.clear();

    float halfSize = mSize * 0.5f;

    Vertex v0, v1, v2, v3, v4;

    v0.Pos = XMFLOAT3(-halfSize, 0.0f, -halfSize);
    v0.Color = XMFLOAT4(mColor.x * 0.8f, mColor.y * 0.8f, mColor.z * 0.8f, mColor.w);

    v1.Pos = XMFLOAT3(halfSize, 0.0f, -halfSize);
    v1.Color = XMFLOAT4(mColor.x * 0.9f, mColor.y * 0.9f, mColor.z * 0.9f, mColor.w);

    v2.Pos = XMFLOAT3(halfSize, 0.0f, halfSize);
    v2.Color = XMFLOAT4(mColor.x * 0.7f, mColor.y * 0.7f, mColor.z * 0.7f, mColor.w);

    v3.Pos = XMFLOAT3(-halfSize, 0.0f, halfSize);
    v3.Color = XMFLOAT4(mColor.x * 0.6f, mColor.y * 0.6f, mColor.z * 0.6f, mColor.w);

    v4.Pos = XMFLOAT3(0.0f, mHeight, 0.0f);
    v4.Color = mColor;

    vertices.push_back(v0);
    vertices.push_back(v1);
    vertices.push_back(v2);
    vertices.push_back(v3);
    vertices.push_back(v4);

    indices.push_back(4); indices.push_back(0); indices.push_back(1);
    indices.push_back(4); indices.push_back(1); indices.push_back(2);
    indices.push_back(4); indices.push_back(2); indices.push_back(3);
    indices.push_back(4); indices.push_back(3); indices.push_back(0);
    indices.push_back(0); indices.push_back(2); indices.push_back(1);
    indices.push_back(0); indices.push_back(3); indices.push_back(2);
}

void Pyramid::BuildGeometry(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList)
{
    std::vector<Vertex> vertices;
    std::vector<std::uint16_t> indices;

    GeneratePyramidData(vertices, indices);

    const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
    const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

    mMeshGeometry = std::make_unique<MeshGeometry>();
    mMeshGeometry->Name = mName + "Geometry";

    ThrowIfFailed(D3DCreateBlob(vbByteSize, &mMeshGeometry->VertexBufferCPU));
    CopyMemory(mMeshGeometry->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

    ThrowIfFailed(D3DCreateBlob(ibByteSize, &mMeshGeometry->IndexBufferCPU));
    CopyMemory(mMeshGeometry->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

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

    mMeshGeometry->DrawArgs["pyramid"] = submesh;
}