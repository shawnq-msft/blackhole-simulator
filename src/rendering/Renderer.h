#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include <wrl/client.h>
#include <vector>
#include <memory>
#include "../physics/BlackHole.h"
#include "Camera.h"

class Renderer {
public:
    // Initialize the renderer
    static bool Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext);
    
    // Render the black hole
    static void RenderBlackHole(ID3D11Device* device, ID3D11DeviceContext* deviceContext, 
                               const BlackHole* blackHole, const Camera* camera);
    
    // Cleanup resources
    static void Cleanup();

private:
    // Shader structures
    struct VertexType {
        DirectX::XMFLOAT3 position;
        DirectX::XMFLOAT2 texCoord;
    };
    
    struct MatrixBufferType {
        DirectX::XMMATRIX world;
        DirectX::XMMATRIX view;
        DirectX::XMMATRIX projection;
    };
    
    struct BlackHoleBufferType {
        DirectX::XMFLOAT3 position;
        float schwarzschildRadius;
        float spinParameter;
        DirectX::XMFLOAT3 padding;
    };
    
    // DirectX resources
    static Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;
    static Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pixelShader;
    static Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;
    static Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;
    static Microsoft::WRL::ComPtr<ID3D11Buffer> m_indexBuffer;
    static Microsoft::WRL::ComPtr<ID3D11Buffer> m_matrixBuffer;
    static Microsoft::WRL::ComPtr<ID3D11Buffer> m_blackHoleBuffer;
    static Microsoft::WRL::ComPtr<ID3D11SamplerState> m_samplerState;
    static Microsoft::WRL::ComPtr<ID3D11Texture2D> m_starfieldTexture;
    static Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_starfieldTextureView;
    
    // Helper methods
    static bool CreateShaders(ID3D11Device* device);
    static bool CreateBuffers(ID3D11Device* device);
    static bool CreateSamplerState(ID3D11Device* device);
    static bool CreateStarfieldTexture(ID3D11Device* device);
    
    // Rendering methods
    static void RenderAccretionDisk(ID3D11DeviceContext* deviceContext, const BlackHole* blackHole, const Camera* camera);
    static void RenderStarfield(ID3D11DeviceContext* deviceContext, const BlackHole* blackHole, const Camera* camera);
};