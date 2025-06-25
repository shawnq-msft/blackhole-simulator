#pragma once

#include <DirectXMath.h>
#include <Windows.h>

class Camera {
public:
    Camera(
        DirectX::XMFLOAT3 position,
        DirectX::XMFLOAT3 lookDirection,
        DirectX::XMFLOAT3 upDirection,
        float fieldOfView,
        float aspectRatio,
        float nearPlane,
        float farPlane
    );

    // Update the camera state
    void Update(double deltaTime);

    // Handle input for camera movement
    void HandleInput(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

    // Set camera properties
    void SetPosition(const DirectX::XMFLOAT3& position);
    void SetLookDirection(const DirectX::XMFLOAT3& direction);
    void SetAspectRatio(float aspectRatio);

    // Get camera matrices
    DirectX::XMMATRIX GetViewMatrix() const;
    DirectX::XMMATRIX GetProjectionMatrix() const;

    // Get camera properties
    DirectX::XMFLOAT3 GetPosition() const;
    DirectX::XMFLOAT3 GetLookDirection() const;
    DirectX::XMFLOAT3 GetUpDirection() const;

private:
    // Camera position and orientation
    DirectX::XMFLOAT3 m_position;
    DirectX::XMFLOAT3 m_lookDirection;
    DirectX::XMFLOAT3 m_upDirection;
    DirectX::XMFLOAT3 m_rightDirection;

    // Camera projection parameters
    float m_fieldOfView;
    float m_aspectRatio;
    float m_nearPlane;
    float m_farPlane;

    // Camera movement parameters
    float m_moveSpeed;
    float m_rotateSpeed;

    // Input state
    bool m_keys[256];
    bool m_mouseButtons[3];
    int m_lastMousePosX;
    int m_lastMousePosY;
    bool m_mouseCaptured;

    // Update the camera vectors
    void UpdateCameraVectors();
};