#pragma once

#include <Windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <DirectXMath.h>
#include <wrl/client.h>
#include <string>
#include <memory>
#include "../physics/BlackHole.h"
#include "../rendering/Camera.h"

// Link the necessary DirectX libraries
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

class DirectXApp {
public:
    DirectXApp(HINSTANCE hInstance, int nCmdShow);
    ~DirectXApp();

    int Run();

private:
    // Window handling
    HINSTANCE m_hInstance;
    HWND m_hWnd;
    std::wstring m_windowTitle;
    int m_windowWidth;
    int m_windowHeight;
    bool m_fullscreen;

    // DirectX objects
    Microsoft::WRL::ComPtr<ID3D11Device> m_device;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_deviceContext;
    Microsoft::WRL::ComPtr<IDXGISwapChain> m_swapChain;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_renderTargetView;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_depthStencilView;
    Microsoft::WRL::ComPtr<ID3D11Texture2D> m_depthStencilBuffer;

    // Simulation objects
    std::unique_ptr<BlackHole> m_blackHole;
    std::unique_ptr<Camera> m_camera;

    // Timing
    double m_deltaTime;
    double m_totalTime;

    // Methods
    bool InitWindow();
    bool InitDirectX();
    bool InitScene();
    void Update();
    void Render();
    void Cleanup();

    // Window procedure
    static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
};