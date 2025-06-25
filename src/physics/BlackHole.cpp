#include "BlackHole.h"
#include <cmath>

BlackHole::BlackHole(float mass, const DirectX::XMFLOAT3& position, float spinParameter)
    : m_mass(mass),
      m_position(position),
      m_spinParameter(std::max(0.0f, std::min(1.0f, spinParameter))) {
    
    // Calculate the Schwarzschild radius (Rs = 2GM/c²)
    m_schwarzschildRadius = 2.0f * GRAVITATIONAL_CONSTANT * (m_mass * SOLAR_MASS) / (SPEED_OF_LIGHT * SPEED_OF_LIGHT);
    
    // Scale for visualization (convert from meters to simulation units)
    m_schwarzschildRadius *= 1.0e-3f;
    
    // Set accretion disk parameters
    m_accretionDiskInnerRadius = 3.0f * m_schwarzschildRadius; // Innermost stable circular orbit for non-rotating black hole
    m_accretionDiskOuterRadius = 20.0f * m_schwarzschildRadius;
    m_accretionDiskTemperature = 10000.0f; // Kelvin
}

BlackHole::~BlackHole() {
}

void BlackHole::Update(double deltaTime) {
    // Update simulation parameters if needed
    // For now, the black hole is static
}

float BlackHole::GetMass() const {
    return m_mass;
}

float BlackHole::GetSchwarzschildRadius() const {
    return m_schwarzschildRadius;
}

float BlackHole::GetSpinParameter() const {
    return m_spinParameter;
}

DirectX::XMFLOAT3 BlackHole::GetPosition() const {
    return m_position;
}

DirectX::XMFLOAT3 BlackHole::CalculateGravitationalLensing(const DirectX::XMFLOAT3& rayOrigin, const DirectX::XMFLOAT3& rayDirection) const {
    // Simplified gravitational lensing calculation
    // In a real implementation, we would solve the geodesic equation
    
    // Calculate vector from ray origin to black hole
    DirectX::XMVECTOR origin = DirectX::XMLoadFloat3(&rayOrigin);
    DirectX::XMVECTOR direction = DirectX::XMLoadFloat3(&rayDirection);
    DirectX::XMVECTOR blackHolePos = DirectX::XMLoadFloat3(&m_position);
    
    DirectX::XMVECTOR toBlackHole = DirectX::XMVectorSubtract(blackHolePos, origin);
    
    // Calculate closest approach distance
    float d = DirectX::XMVectorGetX(DirectX::XMVector3Length(
        DirectX::XMVectorSubtract(toBlackHole, 
            DirectX::XMVectorScale(direction, 
                DirectX::XMVectorGetX(DirectX::XMVector3Dot(toBlackHole, direction))))));
    
    // Calculate deflection angle (simplified approximation)
    // In general relativity, the deflection angle is approximately:
    // θ = 4GM/(c²d) = 2Rs/d
    float deflectionAngle = 0.0f;
    
    if (d > m_schwarzschildRadius) {
        deflectionAngle = 2.0f * m_schwarzschildRadius / d;
    } else {
        // Inside the critical region, light is strongly bent
        deflectionAngle = DirectX::XM_PI;
    }
    
    // Calculate the deflection direction (perpendicular to both the ray and the vector to the black hole)
    DirectX::XMVECTOR deflectionDir = DirectX::XMVector3Cross(direction, 
        DirectX::XMVector3Cross(toBlackHole, direction));
    deflectionDir = DirectX::XMVector3Normalize(deflectionDir);
    
    // Apply the deflection to the ray direction
    DirectX::XMVECTOR rotationAxis = DirectX::XMVector3Normalize(
        DirectX::XMVector3Cross(direction, deflectionDir));
    DirectX::XMMATRIX rotationMatrix = DirectX::XMMatrixRotationAxis(
        rotationAxis, deflectionAngle);
    
    DirectX::XMVECTOR newDirection = DirectX::XMVector3Transform(direction, rotationMatrix);
    newDirection = DirectX::XMVector3Normalize(newDirection);
    
    // Return the new direction
    DirectX::XMFLOAT3 result;
    DirectX::XMStoreFloat3(&result, newDirection);
    return result;
}

std::vector<DirectX::XMFLOAT3> BlackHole::CalculatePhotonPath(
    const DirectX::XMFLOAT3& startPosition, 
    const DirectX::XMFLOAT3& direction, 
    float stepSize, 
    int maxSteps) const {
    
    std::vector<DirectX::XMFLOAT3> path;
    path.reserve(maxSteps);
    
    // Add the starting position
    path.push_back(startPosition);
    
    DirectX::XMFLOAT3 currentPos = startPosition;
    DirectX::XMFLOAT3 currentDir = direction;
    
    for (int i = 0; i < maxSteps; ++i) {
        // Calculate the next position
        DirectX::XMVECTOR pos = DirectX::XMLoadFloat3(&currentPos);
        DirectX::XMVECTOR dir = DirectX::XMLoadFloat3(&currentDir);
        
        DirectX::XMVECTOR nextPos = DirectX::XMVectorAdd(pos, DirectX::XMVectorScale(dir, stepSize));
        DirectX::XMStoreFloat3(&currentPos, nextPos);
        
        // Check if we've hit the event horizon
        DirectX::XMVECTOR blackHolePos = DirectX::XMLoadFloat3(&m_position);
        DirectX::XMVECTOR toBlackHole = DirectX::XMVectorSubtract(blackHolePos, nextPos);
        float distance = DirectX::XMVectorGetX(DirectX::XMVector3Length(toBlackHole));
        
        if (distance <= m_schwarzschildRadius) {
            // We've crossed the event horizon, stop the simulation
            break;
        }
        
        // Calculate the new direction due to gravitational lensing
        currentDir = CalculateGravitationalLensing(currentPos, currentDir);
        
        // Add the new position to the path
        path.push_back(currentPos);
    }
    
    return path;
}

DirectX::XMFLOAT4 BlackHole::CalculateAccretionDiskColor(const DirectX::XMFLOAT3& position) const {
    // Calculate distance from the black hole center
    DirectX::XMVECTOR pos = DirectX::XMLoadFloat3(&position);
    DirectX::XMVECTOR blackHolePos = DirectX::XMLoadFloat3(&m_position);
    DirectX::XMVECTOR toBlackHole = DirectX::XMVectorSubtract(pos, blackHolePos);
    
    // Project onto the xy-plane (assuming the accretion disk is in this plane)
    DirectX::XMFLOAT3 projected;
    DirectX::XMStoreFloat3(&projected, toBlackHole);
    projected.z = 0.0f;
    
    float distance = std::sqrt(projected.x * projected.x + projected.y * projected.y);
    
    // Check if the point is within the accretion disk
    if (distance >= m_accretionDiskInnerRadius && distance <= m_accretionDiskOuterRadius) {
        // Calculate temperature based on distance (temperature decreases with distance)
        float temperatureRatio = m_accretionDiskInnerRadius / distance;
        float temperature = m_accretionDiskTemperature * std::pow(temperatureRatio, 0.75f);
        
        // Convert temperature to RGB color (blackbody radiation approximation)
        float r, g, b;
        
        // Simple approximation of blackbody radiation color
        if (temperature < 1000.0f) {
            r = 1.0f;
            g = 0.0f;
            b = 0.0f;
        } else if (temperature < 2000.0f) {
            r = 1.0f;
            g = 0.5f * (temperature - 1000.0f) / 1000.0f;
            b = 0.0f;
        } else if (temperature < 6000.0f) {
            r = 1.0f;
            g = 0.5f + 0.5f * (temperature - 2000.0f) / 4000.0f;
            b = (temperature - 2000.0f) / 4000.0f;
        } else if (temperature < 15000.0f) {
            r = 1.0f;
            g = 1.0f;
            b = 1.0f;
        } else {
            r = 0.7f;
            g = 0.7f;
            b = 1.0f;
        }
        
        // Add some variation based on angle
        float angle = std::atan2(projected.y, projected.x);
        float variation = 0.1f * std::sin(10.0f * angle + distance);
        
        // Calculate alpha (opacity) - higher near the inner edge
        float alpha = 0.8f * (1.0f - (distance - m_accretionDiskInnerRadius) / 
                             (m_accretionDiskOuterRadius - m_accretionDiskInnerRadius));
        
        // Apply relativistic effects (simplified)
        // Doppler shift and beaming
        float relativeVelocity = std::sqrt(GRAVITATIONAL_CONSTANT * m_mass * SOLAR_MASS / distance);
        float doppler = 1.0f / std::sqrt(1.0f - relativeVelocity * relativeVelocity / (SPEED_OF_LIGHT * SPEED_OF_LIGHT));
        
        // Adjust color based on Doppler shift
        if (projected.x > 0) {
            // Approaching side (blueshift)
            b = std::min(1.0f, b * doppler);
            r = r / doppler;
        } else {
            // Receding side (redshift)
            r = std::min(1.0f, r * doppler);
            b = b / doppler;
        }
        
        return DirectX::XMFLOAT4(r + variation, g + variation, b + variation, alpha);
    } else {
        // Outside the accretion disk
        return DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
    }
}

void BlackHole::CalculateMetricTensor(const DirectX::XMFLOAT3& position, float g[][4]) const {
    // Convert to Boyer-Lindquist coordinates
    float r, theta, phi;
    CartesianToBoyerLindquist(position, r, theta, phi);
    
    // Initialize the metric tensor to zero
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            g[i][j] = 0.0f;
        }
    }
    
    // Kerr metric in Boyer-Lindquist coordinates
    float a = m_spinParameter * m_schwarzschildRadius / 2.0f;
    float rs = m_schwarzschildRadius;
    float rho2 = r * r + a * a * std::cos(theta) * std::cos(theta);
    float delta = r * r - rs * r + a * a;
    
    // Components of the metric tensor
    g[0][0] = -(1.0f - rs * r / rho2);
    g[0][3] = -rs * r * a * std::sin(theta) * std::sin(theta) / rho2;
    g[1][1] = rho2 / delta;
    g[2][2] = rho2;
    g[3][0] = g[0][3];
    g[3][3] = (r * r + a * a + rs * r * a * a * std::sin(theta) * std::sin(theta) / rho2) * 
              std::sin(theta) * std::sin(theta);
}

void BlackHole::CalculateChristoffelSymbols(const DirectX::XMFLOAT3& position, float gamma[][4][4]) const {
    // This is a complex calculation that involves derivatives of the metric tensor
    // For a full simulation, we would implement this, but it's beyond the scope of this example
    
    // Initialize all Christoffel symbols to zero
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            for (int k = 0; k < 4; ++k) {
                gamma[i][j][k] = 0.0f;
            }
        }
    }
}

void BlackHole::CartesianToBoyerLindquist(const DirectX::XMFLOAT3& cartesian, float& r, float& theta, float& phi) const {
    // Shift coordinates so the black hole is at the origin
    float x = cartesian.x - m_position.x;
    float y = cartesian.y - m_position.y;
    float z = cartesian.z - m_position.z;
    
    // Calculate cylindrical coordinates
    float rho = std::sqrt(x * x + y * y);
    phi = std::atan2(y, x);
    
    // Calculate r and theta (simplified for non-rotating black hole)
    r = std::sqrt(rho * rho + z * z);
    theta = std::acos(z / r);
}

DirectX::XMFLOAT3 BlackHole::BoyerLindquistToCartesian(float r, float theta, float phi) const {
    // Convert Boyer-Lindquist coordinates to Cartesian
    float x = r * std::sin(theta) * std::cos(phi);
    float y = r * std::sin(theta) * std::sin(phi);
    float z = r * std::cos(theta);
    
    // Shift back to world coordinates
    return DirectX::XMFLOAT3(
        x + m_position.x,
        y + m_position.y,
        z + m_position.z
    );
}