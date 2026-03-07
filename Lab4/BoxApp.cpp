#include "BoxApp.h"

BoxApp::BoxApp(HINSTANCE hInstance, const std::string& objFilePath)
    : D3DApp(hInstance)
    , mObjFilePath(objFilePath)
    , mCurrentCbvIndex(0)
    , mTimeAccumulator(0.0f)
{
    mMainWndCaption = L"Box Application with GameObjects";
    mScene = std::make_unique<Scene>();
}

BoxApp::~BoxApp()
{
    if (md3dDevice != nullptr)
        FlushCommandQueue();
}

bool BoxApp::Initialize()
{
    if (!D3DApp::Initialize())
        return false;

    ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), nullptr));

    BuildRootSignature();
    BuildShadersAndInputLayout();

    // Создаем игровые объекты
    CreateGameObjects();

    // Инициализируем сцену
    mScene->Initialize(md3dDevice.Get(), mCommandList.Get());

    BuildDescriptorHeaps();
    BuildConstantBuffers();
    BuildPSO();

    ThrowIfFailed(mCommandList->Close());
    ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
    mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

    FlushCommandQueue();

    return true;
}

void BoxApp::CreateGameObjects()
{
    // Создаем несколько кубов на сцене
    auto* cube1 = mScene->CreateObject<CubeObject>(1.0f);
    cube1->SetName("MainCube");
    cube1->SetPosition(0.0f, 0.0f, 0.0f);
    cube1->SetRotation(0.0f, 0.0f, 0.0f);
    cube1->SetScale(1.0f, 1.0f, 1.0f);

    auto* cube2 = mScene->CreateObject<CubeObject>(0.5f);
    cube2->SetName("SmallCube");
    cube2->SetPosition(2.0f, 1.0f, 0.0f);
    cube2->SetRotation(0.0f, 0.0f, 0.0f);
    cube2->SetScale(1.0f, 1.0f, 1.0f);

    auto* cube3 = mScene->CreateObject<CubeObject>(0.75f);
    cube3->SetName("RotatingCube");
    cube3->SetPosition(-2.0f, 0.5f, 0.0f);
    cube3->SetRotation(0.0f, 0.0f, 0.0f);
    cube3->SetScale(1.0f, 1.0f, 1.0f);

    // Если указан OBJ файл, загружаем модель
    if (!mObjFilePath.empty())
    {
        auto* objModel = mScene->CreateObject<ObjModelObject>(mObjFilePath);
        objModel->SetName("ImportedModel");
        objModel->SetPosition(0.0f, 2.0f, 0.0f);
    }
}

void BoxApp::BuildRootSignature()
{
    CD3DX12_DESCRIPTOR_RANGE cbvTable;
    cbvTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);

    CD3DX12_ROOT_PARAMETER slotRootParameter[1];
    slotRootParameter[0].InitAsDescriptorTable(1, &cbvTable);

    CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(1, slotRootParameter, 0, nullptr,
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    Microsoft::WRL::ComPtr<ID3DBlob> serializedRootSig = nullptr;
    Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
    HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
        serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

    if (errorBlob != nullptr)
    {
        ::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
    }
    ThrowIfFailed(hr);

    ThrowIfFailed(md3dDevice->CreateRootSignature(
        0,
        serializedRootSig->GetBufferPointer(),
        serializedRootSig->GetBufferSize(),
        IID_PPV_ARGS(mRootSignature.GetAddressOf())));
}

void BoxApp::BuildShadersAndInputLayout()
{
    HRESULT hr = S_OK;

    mvsByteCode = d3dUtil::CompileShader(L"Shaders\\color.hlsl", nullptr, "VS", "vs_5_0");
    mpsByteCode = d3dUtil::CompileShader(L"Shaders\\color.hlsl", nullptr, "PS", "ps_5_0");

    mInputLayout =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };
}

void BoxApp::BuildPSO()
{
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
    ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));

    psoDesc.InputLayout = { mInputLayout.data(), (UINT)mInputLayout.size() };
    psoDesc.pRootSignature = mRootSignature.Get();
    psoDesc.VS =
    {
        reinterpret_cast<BYTE*>(mvsByteCode->GetBufferPointer()),
        mvsByteCode->GetBufferSize()
    };
    psoDesc.PS =
    {
        reinterpret_cast<BYTE*>(mpsByteCode->GetBufferPointer()),
        mpsByteCode->GetBufferSize()
    };
    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = mBackBufferFormat;
    psoDesc.SampleDesc.Count = m4xMsaaState ? 4 : 1;
    psoDesc.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
    psoDesc.DSVFormat = mDepthStencilFormat;

    ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&mPSO)));
}

void BoxApp::BuildDescriptorHeaps()
{
    UINT objectCount = max(1, static_cast<UINT>(mScene->GetObjectCount()));

    D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc;
    cbvHeapDesc.NumDescriptors = objectCount;
    cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    cbvHeapDesc.NodeMask = 0;
    ThrowIfFailed(md3dDevice->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(&mCbvHeap)));
}

void BoxApp::BuildConstantBuffers()
{
    UINT objectCount = max(1, static_cast<UINT>(mScene->GetObjectCount()));
    mObjectCB = std::make_unique<UploadBuffer<ObjectConstants>>(md3dDevice.Get(), objectCount, true);

    UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));

    D3D12_GPU_VIRTUAL_ADDRESS cbAddress = mObjectCB->Resource()->GetGPUVirtualAddress();

    for (UINT i = 0; i < objectCount; i++)
    {
        D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
        cbvDesc.BufferLocation = cbAddress + i * objCBByteSize;
        cbvDesc.SizeInBytes = objCBByteSize;

        auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(mCbvHeap->GetCPUDescriptorHandleForHeapStart());
        handle.Offset(i, mCbvSrvUavDescriptorSize);
        md3dDevice->CreateConstantBufferView(&cbvDesc, handle);
    }
}

void BoxApp::OnResize()
{
    D3DApp::OnResize();

    XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
    XMStoreFloat4x4(&mProj, P);
}

void BoxApp::Update(const GameTimer& gt)
{
    // Обновляем сцену (все объекты)
    mScene->Update(gt);

    // Анимируем вращающийся куб
    GameObject* rotatingCube = mScene->FindObject("RotatingCube");
    if (rotatingCube)
    {
        mTimeAccumulator += gt.DeltaTime();
        rotatingCube->SetRotation(0.0f, mTimeAccumulator * 2.0f, 0.0f);
    }

    // Обновляем камеру
    float x = mRadius * sinf(mPhi) * cosf(mTheta);
    float z = mRadius * sinf(mPhi) * sinf(mTheta);
    float y = mRadius * cosf(mPhi);

    XMVECTOR pos = XMVectorSet(x, y, z, 1.0f);
    XMVECTOR target = XMVectorZero();
    XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

    XMMATRIX view = XMMatrixLookAtLH(pos, target, up);
    XMStoreFloat4x4(&mView, view);
}

void BoxApp::Draw(const GameTimer& gt)
{
    // Переиспользуем память для команд
    ThrowIfFailed(mDirectCmdListAlloc->Reset());
    ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), mPSO.Get()));

    mCommandList->RSSetViewports(1, &mScreenViewport);
    mCommandList->RSSetScissorRects(1, &mScissorRect);

    // Переход ресурса в состояние render target
    mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
        D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

    // Очистка
    mCommandList->ClearRenderTargetView(CurrentBackBufferView(), Colors::LightSteelBlue, 0, nullptr);
    mCommandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

    // Установка render targets
    mCommandList->OMSetRenderTargets(1, &CurrentBackBufferView(), true, &DepthStencilView());

    // Установка дескрипторного хипа
    ID3D12DescriptorHeap* descriptorHeaps[] = { mCbvHeap.Get() };
    mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

    // Установка корневой сигнатуры
    mCommandList->SetGraphicsRootSignature(mRootSignature.Get());

    // Установка вершинного и индексного буфера для каждого объекта
    XMMATRIX view = XMLoadFloat4x4(&mView);
    XMMATRIX proj = XMLoadFloat4x4(&mProj);
    XMMATRIX viewProj = view * proj;

    mCurrentCbvIndex = 0;

    // Рисуем все объекты сцены
    UINT objectCount = static_cast<UINT>(mScene->GetObjectCount());

    for (UINT i = 0; i < objectCount; i++)
    {
        GameObject* obj = mScene->GetObjectByIndex(i);
        if (!obj) continue;

        XMMATRIX world = XMLoadFloat4x4(&obj->GetWorldMatrix());
        XMMATRIX worldViewProj = world * viewProj;

        ObjectConstants objConstants;
        XMStoreFloat4x4(&objConstants.WorldViewProj, XMMatrixTranspose(worldViewProj));

        // Обновляем константный буфер для этого объекта
        mObjectCB->CopyData(mCurrentCbvIndex, objConstants);

        // Устанавливаем дескриптор CBV для этого объекта
        auto cbvHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(mCbvHeap->GetGPUDescriptorHandleForHeapStart());
        cbvHandle.Offset(mCurrentCbvIndex, mCbvSrvUavDescriptorSize);
        mCommandList->SetGraphicsRootDescriptorTable(0, cbvHandle);

        // Рисуем объект
        MeshGeometry* meshGeo = obj->GetMeshGeometry();
        if (meshGeo && meshGeo->DrawArgs.find("box") != meshGeo->DrawArgs.end())
        {
            auto& boxDrawArgs = meshGeo->DrawArgs["box"];

            mCommandList->IASetVertexBuffers(0, 1, &meshGeo->VertexBufferView());
            mCommandList->IASetIndexBuffer(&meshGeo->IndexBufferView());
            mCommandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

            mCommandList->DrawIndexedInstanced(boxDrawArgs.IndexCount, 1,
                boxDrawArgs.StartIndexLocation, boxDrawArgs.BaseVertexLocation, 0);
        }

        mCurrentCbvIndex++;
    }

    // Переход ресурса в состояние present
    mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
        D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

    ThrowIfFailed(mCommandList->Close());

    ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
    mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

    // Смена буфера
    ThrowIfFailed(mSwapChain->Present(0, 0));
    mCurrBackBuffer = (mCurrBackBuffer + 1) % SwapChainBufferCount;

    FlushCommandQueue();
}

void BoxApp::OnMouseDown(WPARAM btnState, int x, int y)
{
    mLastMousePos.x = x;
    mLastMousePos.y = y;

    SetCapture(mhMainWnd);
}

void BoxApp::OnMouseUp(WPARAM btnState, int x, int y)
{
    ReleaseCapture();
}

void BoxApp::OnMouseMove(WPARAM btnState, int x, int y)
{
    if ((btnState & MK_LBUTTON) != 0)
    {
        float dx = XMConvertToRadians(0.25f * static_cast<float>(x - mLastMousePos.x));
        float dy = XMConvertToRadians(0.25f * static_cast<float>(y - mLastMousePos.y));

        mTheta += dx;
        mPhi += dy;

        mPhi = MathHelper::Clamp(mPhi, 0.1f, MathHelper::Pi - 0.1f);
    }
    else if ((btnState & MK_RBUTTON) != 0)
    {
        float dx = 0.005f * static_cast<float>(x - mLastMousePos.x);
        float dy = 0.005f * static_cast<float>(y - mLastMousePos.y);

        mRadius += dx - dy;

        mRadius = MathHelper::Clamp(mRadius, 3.0f, 15.0f);
    }

    mLastMousePos.x = x;
    mLastMousePos.y = y;
}