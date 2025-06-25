#include "DirectXApp.h"
#include <d3dcompiler.h>
#include <stdexcept>
#include <chrono>
#include "../physics/BlackHole.h"
#include "../rendering/Renderer.h"

// Store a pointer to the current application instance for the window procedure
DirectXApp* g_appInstance = nullptr;

DirectXApp::DirectXApp(HINSTANCE hInstance, int nCmdShow) 
    : m_hInstance(hInstance),
      m_windowTitle(L"Black Hole Simulator"),
      m_windowWidth(1280),
      m_windowHeight(720),
      m_fullscreen(false),
      m_deltaTime(0.0),
      m_totalTime(0.0) {
    
    g_appInstance = this;

    if (!InitWindow()) {
        throw std::runtime_error("Failed to initialize window");
    }

    if (!InitDirectX()) {
        throw std::runtime_error("Failed to initialize DirectX");
    }

    if (!InitScene()) {
        throw std::runtime_error("Failed to initialize scene");
    }

    // Show the window
    ShowWindow(m_hWnd, nCmdShow);
    UpdateWindow(m_hWnd);
}

DirectXApp::~DirectXApp() {
    Cleanup();
}

bool DirectXApp::InitWindow() {
    // Register the window class
    WNDCLASSEX wc = {};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = m_hInstance;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.lpszClassName = L"BlackHoleSimulatorClass";

    if (!RegisterClassEx(&wc)) {
        return false;
    }

    // Create the window
    RECT rc = { 0, 0, static_cast<LONG>(m_windowWidth), static_cast<LONG>(m_windowHeight) };
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

    m_hWnd = CreateWindow(
        wc.lpszClassName,
        m_windowTitle.c_str(),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        rc.right - rc.left, rc.bottom - rc.top,
        nullptr,
        nullptr,
        m_hInstance,
        nullptr
    );

    if (!m_hWnd) {
        return false;
    }

    return true;
}

bool DirectXApp::InitDirectX() {
    // Create device and swap chain
    DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
    swapChainDesc.BufferCount = 1;
    swapChainDesc.BufferDesc.Width = m_windowWidth;
    swapChainDesc.BufferDesc.Height = m_windowHeight;
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
    swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.OutputWindow = m_hWnd;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.Windowed = !m_fullscreen;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0
    };
    UINT numFeatureLevels = ARRAYSIZE(featureLevels);

    D3D_FEATURE_LEVEL featureLevel;
    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        createDeviceFlags,
        featureLevels,
        numFeatureLevels,
        D3D11_SDK_VERSION,
        &swapChainDesc,
        &m_swapChain,
        &m_device,
        &featureLevel,
        &m_deviceContext
    );

    if (FAILED(hr)) {
        return false;
    }

    // Create render target view
    Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;
    hr = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(backBuffer.GetAddressOf()));
    if (FAILED(hr)) {
        return false;
    }

    hr = m_device->CreateRenderTargetView(backBuffer.Get(), nullptr, &m_renderTargetView);
    if (FAILED(hr)) {
        return false;
    }

    // Create depth stencil buffer and view
    D3D11_TEXTURE2D_DESC depthStencilDesc = {};
    depthStencilDesc.Width = m_windowWidth;
    depthStencilDesc.Height = m_windowHeight;
    depthStencilDesc.MipLevels = 1;
    depthStencilDesc.ArraySize = 1;
    depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthStencilDesc.SampleDesc.Count = 1;
    depthStencilDesc.SampleDesc.Quality = 0;
    depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
    depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

    hr = m_device->CreateTexture2D(&depthStencilDesc, nullptr, &m_depthStencilBuffer);
    if (FAILED(hr)) {
        return false;
    }

    hr = m_device->CreateDepthStencilView(m_depthStencilBuffer.Get(), nullptr, &m_depthStencilView);
    if (FAILED(hr)) {
        return false;
    }

    // Set render targets
    m_deviceContext->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), m_depthStencilView.Get());

    // Set viewport
    D3D11_VIEWPORT viewport = {};
    viewport.TopLeftX = 0.0f;
    viewport.TopLeftY = 0.0f;
    viewport.Width = static_cast<float>(m_windowWidth);
    viewport.Height = static_cast<float>(m_windowHeight);
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    m_deviceContext->RSSetViewports(1, &viewport);

    return true;
}

bool DirectXApp::InitScene() {
    // Create the camera
    m_camera = std::make_unique<Camera>(
        DirectX::XMFLOAT3(0.0f, 0.0f, -10.0f),  // Position
        DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f),    // Look direction
        DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f),    // Up direction
        DirectX::XM_PIDIV4,                     // Field of view (45 degrees)
        static_cast<float>(m_windowWidth) / static_cast<float>(m_windowHeight), // Aspect ratio
        0.1f,                                   // Near plane
        1000.0f                                 // Far plane
    );

    // Create the black hole
    m_blackHole = std::make_unique<BlackHole>(
        5.0f,                                   // Mass (in solar masses)
        DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f),    // Position
        0.8f                                    // Spin parameter (0 to 1)
    );

    return true;
}

void DirectXApp::Update() {
    // Calculate delta time
    static auto lastTime = std::chrono::high_resolution_clock::now();
    auto currentTime = std::chrono::high_resolution_clock::now();
    m_deltaTime = std::chrono::duration<double>(currentTime - lastTime).count();
    m_totalTime += m_deltaTime;
    lastTime = currentTime;

    // Update camera
    m_camera->Update(m_deltaTime);

    // Update black hole simulation
    m_blackHole->Update(m_deltaTime);
}

void DirectXApp::Render() {
    // Clear the render target and depth stencil
    float clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f }; // Black background
    m_deviceContext->ClearRenderTargetView(m_renderTargetView.Get(), clearColor);
    m_deviceContext->ClearDepthStencilView(m_depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    // Render the black hole
    Renderer::RenderBlackHole(m_device.Get(), m_deviceContext.Get(), m_blackHole.get(), m_camera.get());

    // Present the back buffer to the screen
    m_swapChain->Present(1, 0);
}

void DirectXApp::Cleanup() {
    // Release DirectX resources
    if (m_deviceContext) m_deviceContext->ClearState();
}

int DirectXApp::Run() {
    MSG msg = {};

    // Main message loop
    while (msg.message != WM_QUIT) {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        } else {
            // Update and render the scene
            Update();
            Render();
        }
    }

    return static_cast<int>(msg.wParam);
}

LRESULT CALLBACK DirectXApp::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    if (g_appInstance) {
        // Handle camera input
        if (g_appInstance->m_camera) {
            g_appInstance->m_camera->HandleInput(hWnd, message, wParam, lParam);
        }
    }

    switch (message) {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    case WM_SIZE:
        // Handle window resizing
        if (g_appInstance && g_appInstance->m_device) {
            g_appInstance->m_windowWidth = LOWORD(lParam);
            g_appInstance->m_windowHeight = HIWORD(lParam);

            // Release the old views
            g_appInstance->m_renderTargetView.Reset();
            g_appInstance->m_depthStencilView.Reset();
            g_appInstance->m_depthStencilBuffer.Reset();

            // Resize the swap chain
            g_appInstance->m_swapChain->ResizeBuffers(
                1,
                g_appInstance->m_windowWidth,
                g_appInstance->m_windowHeight,
                DXGI_FORMAT_R8G8B8A8_UNORM,
                0
            );

            // Recreate the render target view
            Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;
            g_appInstance->m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(backBuffer.GetAddressOf()));
            g_appInstance->m_device->CreateRenderTargetView(backBuffer.Get(), nullptr, g_appInstance->m_renderTargetView.GetAddressOf());

            // Recreate the depth stencil buffer and view
            D3D11_TEXTURE2D_DESC depthStencilDesc = {};
            depthStencilDesc.Width = g_appInstance->m_windowWidth;
            depthStencilDesc.Height = g_appInstance->m_windowHeight;
            depthStencilDesc.MipLevels = 1;
            depthStencilDesc.ArraySize = 1;
            depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
            depthStencilDesc.SampleDesc.Count = 1;
            depthStencilDesc.SampleDesc.Quality = 0;
            depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
            depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

            g_appInstance->m_device->CreateTexture2D(&depthStencilDesc, nullptr, g_appInstance->m_depthStencilBuffer.GetAddressOf());
            g_appInstance->m_device->CreateDepthStencilView(g_appInstance->m_depthStencilBuffer.Get(), nullptr, g_appInstance->m_depthStencilView.GetAddressOf());

            // Set the render targets
            g_appInstance->m_deviceContext->OMSetRenderTargets(1, g_appInstance->m_renderTargetView.GetAddressOf(), g_appInstance->m_depthStencilView.Get());

            // Update the viewport
            D3D11_VIEWPORT viewport = {};
            viewport.TopLeftX = 0.0f;
            viewport.TopLeftY = 0.0f;
            viewport.Width = static_cast<float>(g_appInstance->m_windowWidth);
            viewport.Height = static_cast<float>(g_appInstance->m_windowHeight);
            viewport.MinDepth = 0.0f;
            viewport.MaxDepth = 1.0f;
            g_appInstance->m_deviceContext->RSSetViewports(1, &viewport);

            // Update the camera's aspect ratio
            if (g_appInstance->m_camera) {
                g_appInstance->m_camera->SetAspectRatio(
                    static_cast<float>(g_appInstance->m_windowWidth) / 
                    static_cast<float>(g_appInstance->m_windowHeight)
                );
            }
        }
        return 0;

    case WM_KEYDOWN:
        if (wParam == VK_ESCAPE) {
            PostQuitMessage(0);
            return 0;
        }
        break;
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}