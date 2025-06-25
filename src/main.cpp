#include "rendering/DirectXApp.h"
#include <Windows.h>
#include <memory>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    try {
        // Create and initialize the DirectX application
        auto app = std::make_unique<DirectXApp>(hInstance, nCmdShow);
        
        // Run the main application loop
        return app->Run();
    }
    catch (const std::exception& e) {
        MessageBoxA(nullptr, e.what(), "Error", MB_OK | MB_ICONERROR);
        return 1;
    }
}