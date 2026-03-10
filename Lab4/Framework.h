#pragma once
#include "Common/d3dApp.h"
#include "Common/MathHelper.h"
#include "Common/UploadBuffer.h"
#include "Scene.h"
#include "Vertex.h"
#include <string>

class Framework : public D3DApp
{
public:
    Framework(HINSTANCE hInstance, const std::string& objFilePath = "");
    Framework(const Framework& rhs) = delete;
    Framework& operator=(const Framework& rhs) = delete;
    ~Framework();

    virtual bool Initialize() override;

private:
    virtual void OnResize() override;
    virtual void Update(const GameTimer& gt) override;
    virtual void Draw(const GameTimer& gt) override;

    virtual void OnMouseDown(WPARAM btnState, int x, int y) override;
    virtual void OnMouseUp(WPARAM btnState, int x, int y) override;
    virtual void OnMouseMove(WPARAM btnState, int x, int y) override;

    virtual LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) override;

    std::string mObjFilePath;

    void BuildDescriptorHeaps();
    void BuildConstantBuffers();
    void BuildRootSignature();
    void BuildShadersAndInputLayout();
    void BuildPSO();
    void CreateGameObjects();

    void UpdateCamera(const GameTimer& gt);
    void ProcessKeyboardInput(const GameTimer& gt);

private:
    Microsoft::WRL::ComPtr<ID3D12RootSignature> mRootSignature = nullptr;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mCbvHeap = nullptr;

    std::unique_ptr<UploadBuffer<ObjectConstants>> mObjectCB = nullptr;

    Microsoft::WRL::ComPtr<ID3DBlob> mvsByteCode = nullptr;
    Microsoft::WRL::ComPtr<ID3DBlob> mpsByteCode = nullptr;

    std::vector<D3D12_INPUT_ELEMENT_DESC> mInputLayout;

    Microsoft::WRL::ComPtr<ID3D12PipelineState> mPSO = nullptr;

    XMFLOAT4X4 mView = MathHelper::Identity4x4();
    XMFLOAT4X4 mProj = MathHelper::Identity4x4();

    float mTheta = 1.5f * XM_PI;
    float mPhi = XM_PIDIV4;
    float mRadius = 5.0f;

    XMFLOAT3 mCameraPosition;
    XMFLOAT3 mCameraTarget;
    XMFLOAT3 mCameraUp;
    float mCameraYaw;
    float mCameraPitch;
    float mMoveSpeed;
    float mRotateSpeed;

    bool mKeyW, mKeyA, mKeyS, mKeyD;
    bool mKeyUp, mKeyDown, mKeyLeft, mKeyRight;

    POINT mLastMousePos;

    std::unique_ptr<Scene> mScene;
    UINT mCurrentCbvIndex;
    float mTimeAccumulator;
};