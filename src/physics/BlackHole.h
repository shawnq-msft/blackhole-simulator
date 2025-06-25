#pragma once

#include <DirectXMath.h>
#include <vector>

// Constants
constexpr float GRAVITATIONAL_CONSTANT = 6.67430e-11f;  // m^3 kg^-1 s^-2
constexpr float SPEED_OF_LIGHT = 299792458.0f;          // m/s
constexpr float SOLAR_MASS = 1.989e30f;                 // kg

class BlackHole {
public:
    BlackHole(float mass, const DirectX::XMFLOAT3& position, float spinParameter);
    ~BlackHole();

    // Update the black hole simulation
    void Update(double deltaTime);

    // Get black hole properties
    float GetMass() const;
    float GetSchwarzschildRadius() const;
    float GetSpinParameter() const;
    DirectX::XMFLOAT3 GetPosition() const;
    
    // Calculate the gravitational lensing effect
    DirectX::XMFLOAT3 CalculateGravitationalLensing(const DirectX::XMFLOAT3& rayOrigin, const DirectX::XMFLOAT3& rayDirection) const;
    
    // Calculate the path of a photon around the black hole
    std::vector<DirectX::XMFLOAT3> CalculatePhotonPath(const DirectX::XMFLOAT3& startPosition, const DirectX::XMFLOAT3& direction, float stepSize, int maxSteps) const;
    
    // Calculate the accretion disk color at a given point
    DirectX::XMFLOAT4 CalculateAccretionDiskColor(const DirectX::XMFLOAT3& position) const;

private:
    float m_mass;                      // Mass in solar masses
    float m_schwarzschildRadius;       // Schwarzschild radius (event horizon for non-rotating black hole)
    float m_spinParameter;             // Dimensionless spin parameter (0 to 1)
    DirectX::XMFLOAT3 m_position;      // Position in world space
    float m_accretionDiskInnerRadius;  // Inner radius of the accretion disk
    float m_accretionDiskOuterRadius;  // Outer radius of the accretion disk
    float m_accretionDiskTemperature;  // Temperature of the accretion disk

    // Calculate the metric tensor at a given point (Kerr metric for rotating black hole)
    void CalculateMetricTensor(const DirectX::XMFLOAT3& position, float g[][4]) const;
    
    // Calculate the Christoffel symbols at a given point
    void CalculateChristoffelSymbols(const DirectX::XMFLOAT3& position, float gamma[][4][4]) const;
    
    // Convert Cartesian coordinates to Boyer-Lindquist coordinates
    void CartesianToBoyerLindquist(const DirectX::XMFLOAT3& cartesian, float& r, float& theta, float& phi) const;
    
    // Convert Boyer-Lindquist coordinates to Cartesian coordinates
    DirectX::XMFLOAT3 BoyerLindquistToCartesian(float r, float theta, float phi) const;
};