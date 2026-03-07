#pragma once
#include "Common/d3dUtil.h"
#include "Common/MathHelper.h"
#include "Vertex.h"
#include <DirectXMath.h>
#include <memory>
#include <string>
#include <array>
#include <vector>
#include "Common/GameTimer.h"

using namespace DirectX;

class GameObject
{
public:
    GameObject();
    virtual ~GameObject() = default;

    virtual void Initialize(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList);
    virtual void Update(const GameTimer& gt);
    virtual void Draw(ID3D12GraphicsCommandList* cmdList, UINT cbvIndex);

    void SetPosition(float x, float y, float z);
    void SetRotation(float pitch, float yaw, float roll);
    void SetScale(float x, float y, float z);

    XMFLOAT3 GetPosition() const { return mPosition; }
    XMFLOAT3 GetRotation() const { return mRotation; }
    XMFLOAT3 GetScale() const { return mScale; }

    XMFLOAT4X4 GetWorldMatrix() const { return mWorld; }

    void SetName(const std::string& name) { mName = name; }
    std::string GetName() const { return mName; }

    MeshGeometry* GetMeshGeometry() { return mMeshGeometry.get(); }
    const MeshGeometry* GetMeshGeometry() const { return mMeshGeometry.get(); }

protected:
    virtual void UpdateWorldMatrix();

public:
    XMFLOAT3 mPosition;
    XMFLOAT3 mRotation;
    XMFLOAT3 mScale;
    XMFLOAT4X4 mWorld;

    std::unique_ptr<MeshGeometry> mMeshGeometry;

    std::string mName;

    bool mIsVisible;
    bool mIsInitialized;
};

class CubeObject : public GameObject
{
public:
    CubeObject(float size = 1.0f);
    virtual ~CubeObject() = default;

    virtual void Initialize(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList) override;

private:
    float mSize;
    void BuildGeometry(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList);
};

class ObjModelObject : public GameObject
{
public:
    ObjModelObject(const std::string& objFilePath);
    virtual ~ObjModelObject() = default;

    virtual void Initialize(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList) override;

private:
    std::string mObjFilePath;
    void LoadFromObj(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList);
};