#pragma once
#include "GameObject.h"
#include <memory>
#include <vector>
#include <string>

class Scene
{
public:
    Scene();
    ~Scene();

    void Initialize(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList);
    void Update(const GameTimer& gt);
    void Draw(ID3D12GraphicsCommandList* cmdList, ID3D12DescriptorHeap* cbvHeap, UINT& cbvIndex);

    template<typename T, typename... Args>
    T* CreateObject(Args&&... args)
    {
        auto obj = std::make_unique<T>(std::forward<Args>(args)...);
        T* ptr = obj.get();
        mGameObjects.push_back(std::move(obj));
        return ptr;
    }

    GameObject* FindObject(const std::string& name);
    GameObject* GetObjectByIndex(size_t index);
    void RemoveObject(const std::string& name);
    void Clear();

    size_t GetObjectCount() const { return mGameObjects.size(); }

private:
    std::vector<std::unique_ptr<GameObject>> mGameObjects;
};