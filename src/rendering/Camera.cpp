#include "Camera.h"
#include <algorithm>

Camera::Camera(
    DirectX::XMFLOAT3 position,
    DirectX::XMFLOAT3 lookDirection,
    DirectX::XMFLOAT3 upDirection,
    float fieldOfView,
    float aspectRatio,
    float nearPlane,
    float farPlane
) : m_position(position),
    m_lookDirection(lookDirection),
    m_upDirection(upDirection),
    m_fieldOfView(fieldOfView),
    m_aspectRatio(aspectRatio),
    m_nearPlane(nearPlane),
    m_farPlane(farPlane),
    m_moveSpeed(5.0f),
    m_rotateSpeed(0.005f),
    m_lastMousePosX(0),
    m_lastMousePosY(0),
    m_mouseCaptured(false) {
    
    // Initialize input state
    memset(m_keys, 0, sizeof(m_keys));
    memset(m_mouseButtons, 0, sizeof(m_mouseButtons));

    // Calculate the right direction
    UpdateCameraVectors();
}

void Camera::Update(double deltaTime) {
    float dt = static_cast<float>(deltaTime);

    // Handle keyboard movement
    DirectX::XMVECTOR position = DirectX::XMLoadFloat3(&m_position);
    DirectX::XMVECTOR lookDir = DirectX::XMLoadFloat3(&m_lookDirection);
    DirectX::XMVECTOR upDir = DirectX::XMLoadFloat3(&m_upDirection);
    DirectX::XMVECTOR rightDir = DirectX::XMLoadFloat3(&m_rightDirection);

    // Forward/backward movement
    if (m_keys['W'] || m_keys[VK_UP]) {
        position += lookDir * m_moveSpeed * dt;
    }
    if (m_keys['S'] || m_keys[VK_DOWN]) {
        position -= lookDir * m_moveSpeed * dt;
    }

    // Left/right movement
    if (m_keys['A'] || m_keys[VK_LEFT]) {
        position -= rightDir * m_moveSpeed * dt;
    }
    if (m_keys['D'] || m_keys[VK_RIGHT]) {
        position += rightDir * m_moveSpeed * dt;
    }

    // Up/down movement
    if (m_keys['Q']) {
        position -= upDir * m_moveSpeed * dt;
    }
    if (m_keys['E']) {
        position += upDir * m_moveSpeed * dt;
    }

    // Update position
    DirectX::XMStoreFloat3(&m_position, position);
}

void Camera::HandleInput(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_KEYDOWN:
        if (wParam < 256) {
            m_keys[wParam] = true;
        }
        break;

    case WM_KEYUP:
        if (wParam < 256) {
            m_keys[wParam] = false;
        }
        break;

    case WM_LBUTTONDOWN:
        m_mouseButtons[0] = true;
        m_mouseCaptured = true;
        SetCapture(hWnd);
        m_lastMousePosX = GET_X_LPARAM(lParam);
        m_lastMousePosY = GET_Y_LPARAM(lParam);
        break;

    case WM_LBUTTONUP:
        m_mouseButtons[0] = false;
        if (!m_mouseButtons[1] && !m_mouseButtons[2]) {
            m_mouseCaptured = false;
            ReleaseCapture();
        }
        break;

    case WM_RBUTTONDOWN:
        m_mouseButtons[1] = true;
        m_mouseCaptured = true;
        SetCapture(hWnd);
        m_lastMousePosX = GET_X_LPARAM(lParam);
        m_lastMousePosY = GET_Y_LPARAM(lParam);
        break;

    case WM_RBUTTONUP:
        m_mouseButtons[1] = false;
        if (!m_mouseButtons[0] && !m_mouseButtons[2]) {
            m_mouseCaptured = false;
            ReleaseCapture();
        }
        break;

    case WM_MBUTTONDOWN:
        m_mouseButtons[2] = true;
        m_mouseCaptured = true;
        SetCapture(hWnd);
        m_lastMousePosX = GET_X_LPARAM(lParam);
        m_lastMousePosY = GET_Y_LPARAM(lParam);
        break;

    case WM_MBUTTONUP:
        m_mouseButtons[2] = false;
        if (!m_mouseButtons[0] && !m_mouseButtons[1]) {
            m_mouseCaptured = false;
            ReleaseCapture();
        }
        break;

    case WM_MOUSEMOVE:
        if (m_mouseCaptured) {
            int x = GET_X_LPARAM(lParam);
            int y = GET_Y_LPARAM(lParam);
            int dx = x - m_lastMousePosX;
            int dy = y - m_lastMousePosY;
            m_lastMousePosX = x;
            m_lastMousePosY = y;

            // Handle camera rotation with left mouse button
            if (m_mouseButtons[0]) {
                // Yaw rotation (around up axis)
                DirectX::XMVECTOR lookDir = DirectX::XMLoadFloat3(&m_lookDirection);
                DirectX::XMVECTOR upDir = DirectX::XMLoadFloat3(&m_upDirection);
                DirectX::XMMATRIX rotationMatrix = DirectX::XMMatrixRotationAxis(upDir, -dx * m_rotateSpeed);
                lookDir = DirectX::XMVector3TransformNormal(lookDir, rotationMatrix);
                
                // Pitch rotation (around right axis)
                DirectX::XMVECTOR rightDir = DirectX::XMLoadFloat3(&m_rightDirection);
                rotationMatrix = DirectX::XMMatrixRotationAxis(rightDir, -dy * m_rotateSpeed);
                lookDir = DirectX::XMVector3TransformNormal(lookDir, rotationMatrix);
                
                // Normalize the look direction
                lookDir = DirectX::XMVector3Normalize(lookDir);
                DirectX::XMStoreFloat3(&m_lookDirection, lookDir);
                
                // Update camera vectors
                UpdateCameraVectors();
            }
        }
        break;

    case WM_MOUSEWHEEL:
        {
            int zDelta = GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA;
            m_moveSpeed = std::max(0.1f, m_moveSpeed + zDelta * 0.5f);
        }
        break;
    }
}

void Camera::SetPosition(const DirectX::XMFLOAT3& position) {
    m_position = position;
}

void Camera::SetLookDirection(const DirectX::XMFLOAT3& direction) {
    m_lookDirection = direction;
    UpdateCameraVectors();
}

void Camera::SetAspectRatio(float aspectRatio) {
    m_aspectRatio = aspectRatio;
}

DirectX::XMMATRIX Camera::GetViewMatrix() const {
    DirectX::XMVECTOR eyePosition = DirectX::XMLoadFloat3(&m_position);
    DirectX::XMVECTOR lookDirection = DirectX::XMLoadFloat3(&m_lookDirection);
    DirectX::XMVECTOR upDirection = DirectX::XMLoadFloat3(&m_upDirection);
    
    DirectX::XMVECTOR focusPoint = DirectX::XMVectorAdd(eyePosition, lookDirection);
    
    return DirectX::XMMatrixLookAtLH(eyePosition, focusPoint, upDirection);
}

DirectX::XMMATRIX Camera::GetProjectionMatrix() const {
    return DirectX::XMMatrixPerspectiveFovLH(m_fieldOfView, m_aspectRatio, m_nearPlane, m_farPlane);
}

DirectX::XMFLOAT3 Camera::GetPosition() const {
    return m_position;
}

DirectX::XMFLOAT3 Camera::GetLookDirection() const {
    return m_lookDirection;
}

DirectX::XMFLOAT3 Camera::GetUpDirection() const {
    return m_upDirection;
}

void Camera::UpdateCameraVectors() {
    // Normalize the look direction
    DirectX::XMVECTOR lookDir = DirectX::XMLoadFloat3(&m_lookDirection);
    lookDir = DirectX::XMVector3Normalize(lookDir);
    DirectX::XMStoreFloat3(&m_lookDirection, lookDir);

    // Calculate the right vector
    DirectX::XMVECTOR upDir = DirectX::XMLoadFloat3(&m_upDirection);
    DirectX::XMVECTOR rightDir = DirectX::XMVector3Cross(lookDir, upDir);
    rightDir = DirectX::XMVector3Normalize(rightDir);
    DirectX::XMStoreFloat3(&m_rightDirection, rightDir);

    // Recalculate the up vector to ensure orthogonality
    upDir = DirectX::XMVector3Cross(rightDir, lookDir);
    upDir = DirectX::XMVector3Normalize(upDir);
    DirectX::XMStoreFloat3(&m_upDirection, upDir);
}