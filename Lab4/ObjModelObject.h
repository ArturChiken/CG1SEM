#pragma once
#include "GameObject.h"
#include "ObjLoader.h"
#include "Vertex.h"

class ObjModelObject : public GameObject
{
public:
    ObjModelObject(const std::string& filename) : mFilename(filename) {}

    virtual bool Initialize(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList) override
    {
        std::vector<XMFLOAT3> positions;
        std::vector<XMFLOAT3> normals;
        std::vector<std::uint32_t> indices;

        if (!ObjLoader::LoadObjFile(mFilename, positions, normals, indices))
            return false;

        std::vector<Vertex> vertices;
        vertices.reserve(positions.size());

        XMVECTOR lightDir = XMVector3Normalize(XMVectorSet(1.0f, 1.0f, 1.0f, 0.0f));

        for (size_t i = 0; i < positions.size(); ++i)
        {
            XMVECTOR normal = XMLoadFloat3(&normals[i]);
            float diffuse = XMVectorGetX(XMVector3Dot(normal, lightDir));
            diffuse = max(0.1f, diffuse);

            XMFLOAT4 color(diffuse, diffuse, diffuse, 1.0f);
            vertices.push_back({ positions[i], color });
        }
        return true;
    }

private:
    std::string mFilename;
    std::unique_ptr<MeshGeometry> mMeshGeometry;
};