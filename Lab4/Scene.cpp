#include "Scene.h"

Scene::Scene()
{
}

Scene::~Scene()
{
    Clear();
}

void Scene::Initialize(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList)
{
    for (auto& obj : mGameObjects)
    {
        obj->Initialize(device, cmdList);
    }
}

void Scene::Update(const GameTimer& gt)
{
    for (auto& obj : mGameObjects)
    {
        obj->Update(gt);
    }
}

void Scene::Draw(ID3D12GraphicsCommandList* cmdList, ID3D12DescriptorHeap* cbvHeap, UINT& cbvIndex)
{
    for (auto& obj : mGameObjects)
    {
        obj->Draw(cmdList, cbvIndex);
        cbvIndex++; // Каждый объект использует свой CBV
    }
}

GameObject* Scene::FindObject(const std::string& name)
{
    for (auto& obj : mGameObjects)
    {
        if (obj->GetName() == name)
            return obj.get();
    }
    return nullptr;
}

GameObject* Scene::GetObjectByIndex(size_t index)
{
    if (index < mGameObjects.size())
        return mGameObjects[index].get();
    return nullptr;
}

void Scene::RemoveObject(const std::string& name)
{
    auto it = std::remove_if(mGameObjects.begin(), mGameObjects.end(),
        [&name](const std::unique_ptr<GameObject>& obj) {
            return obj->GetName() == name;
        });
    mGameObjects.erase(it, mGameObjects.end());
}

void Scene::Clear()
{
    mGameObjects.clear();
}