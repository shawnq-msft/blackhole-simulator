#include "Renderer.h"
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <vector>
#include <random>

// Initialize static members
Microsoft::WRL::ComPtr<ID3D11VertexShader> Renderer::m_vertexShader;
Microsoft::WRL::ComPtr<ID3D11PixelShader> Renderer::m_pixelShader;
Microsoft::WRL::ComPtr<ID3D11InputLayout> Renderer::m_inputLayout;
Microsoft::WRL::ComPtr<ID3D11Buffer> Renderer::m_vertexBuffer;
Microsoft::WRL::ComPtr<ID3D11Buffer> Renderer::m_indexBuffer;
Microsoft::WRL::ComPtr<ID3D11Buffer> Renderer::m_matrixBuffer;
Microsoft::WRL::ComPtr<ID3D11Buffer> Renderer::m_blackHoleBuffer;
Microsoft::WRL::ComPtr<ID3D11SamplerState> Renderer::m_samplerState;
Microsoft::WRL::ComPtr<ID3D11Texture2D> Renderer::m_starfieldTexture;
Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> Renderer::m_starfieldTextureView;

bool Renderer::Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext) {
    bool result = true;
    
    // Create the shaders
    result = CreateShaders(device);
    if (!result) {
        return false;
    }
    
    // Create the buffers
    result = CreateBuffers(device);
    if (!result) {
        return false;
    }
    
    // Create the sampler state
    result = CreateSamplerState(device);
    if (!result) {
        return false;
    }
    
    // Create the starfield texture
    result = CreateStarfieldTexture(device);
    if (!result) {
        return false;
    }
    
    return true;
}

void Renderer::RenderBlackHole(ID3D11Device* device, ID3D11DeviceContext* deviceContext, 
                              const BlackHole* blackHole, const Camera* camera) {
    // Set the vertex and pixel shaders
    deviceContext->VSSetShader(m_vertexShader.Get(), nullptr, 0);
    deviceContext->PSSetShader(m_pixelShader.Get(), nullptr, 0);
    
    // Set the input layout
    deviceContext->IASetInputLayout(m_inputLayout.Get());
    
    // Set the vertex and index buffers
    UINT stride = sizeof(VertexType);
    UINT offset = 0;
    deviceContext->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);
    deviceContext->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
    
    // Set the primitive topology
    deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    
    // Set the sampler state
    deviceContext->PSSetSamplers(0, 1, m_samplerState.GetAddressOf());
    
    // Set the starfield texture
    deviceContext->PSSetShaderResources(0, 1, m_starfieldTextureView.GetAddressOf());
    
    // Update the matrix constant buffer
    MatrixBufferType matrixBuffer;
    matrixBuffer.world = DirectX::XMMatrixIdentity();
    matrixBuffer.view = camera->GetViewMatrix();
    matrixBuffer.projection = camera->GetProjectionMatrix();
    
    // Transpose the matrices for the shader
    matrixBuffer.world = DirectX::XMMatrixTranspose(matrixBuffer.world);
    matrixBuffer.view = DirectX::XMMatrixTranspose(matrixBuffer.view);
    matrixBuffer.projection = DirectX::XMMatrixTranspose(matrixBuffer.projection);
    
    // Update the matrix constant buffer
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    deviceContext->Map(m_matrixBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    memcpy(mappedResource.pData, &matrixBuffer, sizeof(MatrixBufferType));
    deviceContext->Unmap(m_matrixBuffer.Get(), 0);
    
    // Set the matrix constant buffer
    deviceContext->VSSetConstantBuffers(0, 1, m_matrixBuffer.GetAddressOf());
    
    // Update the black hole constant buffer
    BlackHoleBufferType blackHoleBuffer;
    blackHoleBuffer.position = blackHole->GetPosition();
    blackHoleBuffer.schwarzschildRadius = blackHole->GetSchwarzschildRadius();
    blackHoleBuffer.spinParameter = blackHole->GetSpinParameter();
    blackHoleBuffer.padding = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
    
    // Update the black hole constant buffer
    deviceContext->Map(m_blackHoleBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    memcpy(mappedResource.pData, &blackHoleBuffer, sizeof(BlackHoleBufferType));
    deviceContext->Unmap(m_blackHoleBuffer.Get(), 0);
    
    // Set the black hole constant buffer
    deviceContext->PSSetConstantBuffers(0, 1, m_blackHoleBuffer.GetAddressOf());
    
    // Render the scene
    deviceContext->DrawIndexed(36, 0, 0); // Assuming a cube with 36 indices
    
    // Render the accretion disk
    RenderAccretionDisk(deviceContext, blackHole, camera);
    

