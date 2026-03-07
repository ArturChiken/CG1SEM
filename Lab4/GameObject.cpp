#include "GameObject.h"
#include "Common/d3dUtil.h"

GameObject::GameObject() :
    mPosition(0.0f, 0.0f, 0.0f),
    mRotation(0.0f, 0.0f, 0.0f),
    mScale(1.0f, 1.0f, 1.0f),
    mWorld(MathHelper::Identity4x4()),
    mIsVisible(true),
    mIsInitialized(false),
    mName("GameObject")
{
}

void GameObject::Initialize(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList)
{
    mIsInitialized = true;
}

void GameObject::Update(const GameTimer& gt)
{
    if (!mIsVisible) return;
    UpdateWorldMatrix();
}

void GameObject::Draw(ID3D12GraphicsCommandList* cmdList, UINT cbvIndex)
{
    if (!mIsVisible || !mIsInitialized || !mMeshGeometry) return;

    if (!mMeshGeometry->DrawArgs.empty())
    {
        auto& drawArgs = mMeshGeometry->DrawArgs.begin()->second;

        cmdList->IASetVertexBuffers(0, 1, &mMeshGeometry->VertexBufferView());
        cmdList->IASetIndexBuffer(&mMeshGeometry->IndexBufferView());
        cmdList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        cmdList->DrawIndexedInstanced(drawArgs.IndexCount, 1,
            drawArgs.StartIndexLocation, drawArgs.BaseVertexLocation, 0);
    }
}

void GameObject::SetPosition(float x, float y, float z)
{
    mPosition.x = x;
    mPosition.y = y;
    mPosition.z = z;
    UpdateWorldMatrix();
}

void GameObject::SetRotation(float pitch, float yaw, float roll)
{
    mRotation.x = pitch;
    mRotation.y = yaw;
    mRotation.z = roll;
    UpdateWorldMatrix();
}

void GameObject::SetScale(float x, float y, float z)
{
    mScale.x = x;
    mScale.y = y;
    mScale.z = z;
    UpdateWorldMatrix();
}

void GameObject::UpdateWorldMatrix()
{
    XMMATRIX scale = XMMatrixScaling(mScale.x, mScale.y, mScale.z);
    XMMATRIX rotation = XMMatrixRotationRollPitchYaw(mRotation.x, mRotation.y, mRotation.z);
    XMMATRIX translation = XMMatrixTranslation(mPosition.x, mPosition.y, mPosition.z);

    XMMATRIX world = scale * rotation * translation;
    XMStoreFloat4x4(&mWorld, world);
}

CubeObject::CubeObject(float size) :
    GameObject(),
    mSize(size)
{
    mName = "Cube";
}

void CubeObject::Initialize(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList)
{
    BuildGeometry(device, cmdList);
    mIsInitialized = true;
}

void CubeObject::BuildGeometry(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList)
{
    std::array<Vertex, 8> vertices = {
        Vertex({ XMFLOAT3(-mSize, -mSize, -mSize), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) }),
        Vertex({ XMFLOAT3(-mSize, +mSize, -mSize), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) }),
        Vertex({ XMFLOAT3(+mSize, +mSize, -mSize), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) }),
        Vertex({ XMFLOAT3(+mSize, -mSize, -mSize), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f) }),
        Vertex({ XMFLOAT3(-mSize, -mSize, +mSize), XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f) }),
        Vertex({ XMFLOAT3(-mSize, +mSize, +mSize), XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f) }),
        Vertex({ XMFLOAT3(+mSize, +mSize, +mSize), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f) }),
        Vertex({ XMFLOAT3(+mSize, -mSize, +mSize), XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f) })
    };

    std::array<std::uint16_t, 36> indices = {
        0, 1, 2,
        0, 2, 3,
        4, 6, 5,
        4, 7, 6,
        4, 5, 1,
        4, 1, 0,
        3, 2, 6,
        3, 6, 7,
        1, 5, 6,
        1, 6, 2,
        4, 0, 3,
        4, 3, 7
    };

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

    mMeshGeometry->DrawArgs["box"] = submesh;
}

ObjModelObject::ObjModelObject(const std::string& objFilePath) :
    GameObject(),
    mObjFilePath(objFilePath)
{
    mName = "ObjModel";
}

void ObjModelObject::Initialize(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList)
{
    LoadFromObj(device, cmdList);
    mIsInitialized = true;
}

void ObjModelObject::LoadFromObj(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList)
{
    CubeObject temp(1.0f);
    temp.Initialize(device, cmdList);
    mMeshGeometry = std::move(temp.mMeshGeometry);
}