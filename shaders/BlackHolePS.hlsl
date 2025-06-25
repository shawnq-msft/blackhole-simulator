// Constant buffer for black hole properties
cbuffer BlackHoleBuffer : register(b0)
{
    float3 blackHolePosition;
    float schwarzschildRadius;
    float spinParameter;
    float3 padding;
};

// Texture and sampler
Texture2D starfieldTexture : register(t0);
SamplerState samplerState : register(s0);

// Input pixel structure
struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD0;
    float3 worldPosition : TEXCOORD1;
    float3 viewDirection : TEXCOORD2;
};

// Helper function to calculate gravitational lensing
float3 CalculateGravitationalLensing(float3 rayOrigin, float3 rayDirection)
{
    // Vector from ray origin to black hole
    float3 toBlackHole = blackHolePosition - rayOrigin;
    
    // Calculate closest approach distance
    float d = length(toBlackHole - dot(toBlackHole, rayDirection) * rayDirection);
    
    // Calculate deflection angle (simplified approximation)
    float deflectionAngle = 0.0f;
    
    if (d > schwarzschildRadius)
    {
        deflectionAngle = 2.0f * schwarzschildRadius / d;
    }
    else
    {
        // Inside the critical region, light is strongly bent
        deflectionAngle = 3.14159265f; // PI
    }
    
    // Calculate the deflection direction (perpendicular to both the ray and the vector to the black hole)
    float3 deflectionDir = normalize(cross(rayDirection, cross(toBlackHole, rayDirection)));
    
    // Apply the deflection to the ray direction
    float3 rotationAxis = normalize(cross(rayDirection, deflectionDir));
    
    // Simplified rotation calculation
    float s = sin(deflectionAngle);
    float c = cos(deflectionAngle);
    float t = 1.0f - c;
    
    float3x3 rotationMatrix = float3x3(
        t * rotationAxis.x * rotationAxis.x + c,
        t * rotationAxis.x * rotationAxis.y - s * rotationAxis.z,
        t * rotationAxis.x * rotationAxis.z + s * rotationAxis.y,
        
        t * rotationAxis.x * rotationAxis.y + s * rotationAxis.z,
        t * rotationAxis.y * rotationAxis.y + c,
        t * rotationAxis.y * rotationAxis.z - s * rotationAxis.x,
        
        t * rotationAxis.x * rotationAxis.z - s * rotationAxis.y,
        t * rotationAxis.y * rotationAxis.z + s * rotationAxis.x,
        t * rotationAxis.z * rotationAxis.z + c
    );
    
    float3 newDirection = mul(rotationMatrix, rayDirection);
    return normalize(newDirection);
}

// Helper function to calculate accretion disk color
float4 CalculateAccretionDiskColor(float3 position)
{
    // Calculate distance from the black hole center
    float3 toBlackHole = position - blackHolePosition;
    
    // Project onto the xy-plane (assuming the accretion disk is in this plane)
    float3 projected = float3(toBlackHole.x, toBlackHole.y, 0.0f);
    float distance = length(projected);
    
    // Accretion disk parameters
    float innerRadius = 3.0f * schwarzschildRadius;
    float outerRadius = 20.0f * schwarzschildRadius;
    
    // Check if the point is within the accretion disk
    if (distance >= innerRadius && distance <= outerRadius)
    {
        // Calculate temperature based on distance (temperature decreases with distance)
        float temperatureRatio = innerRadius / distance;
        float temperature = 10000.0f * pow(temperatureRatio, 0.75f);
        
        // Convert temperature to RGB color (blackbody radiation approximation)
        float r, g, b;
        
        // Simple approximation of blackbody radiation color
        if (temperature < 1000.0f)
        {
            r = 1.0f;
            g = 0.0f;
            b = 0.0f;
        }
        else if (temperature < 2000.0f)
        {
            r = 1.0f;
            g = 0.5f * (temperature - 1000.0f) / 1000.0f;
            b = 0.0f;
        }
        else if (temperature < 6000.0f)
        {
            r = 1.0f;
            g = 0.5f + 0.5f * (temperature - 2000.0f) / 4000.0f;
            b = (temperature - 2000.0f) / 4000.0f;
        }
        else if (temperature < 15000.0f)
        {
            r = 1.0f;
            g = 1.0f;
            b = 1.0f;
        }
        else
        {
            r = 0.7f;
            g = 0.7f;
            b = 1.0f;
        }
        
        // Add some variation based on angle
        float angle = atan2(projected.y, projected.x);
        float variation = 0.1f * sin(10.0f * angle + distance);
        
        // Calculate alpha (opacity) - higher near the inner edge
        float alpha = 0.8f * (1.0f - (distance - innerRadius) / (outerRadius - innerRadius));
        
        // Apply relativistic effects (simplified)
        // Doppler shift and beaming
        float relativeVelocity = sqrt(6.67430e-11f * 1.989e30f * schwarzschildRadius / distance);
        float doppler = 1.0f / sqrt(1.0f - relativeVelocity * relativeVelocity / (299792458.0f * 299792458.0f));
        
        // Adjust color based on Doppler shift
        if (projected.x > 0)
        {
            // Approaching side (blueshift)
            b = min(1.0f, b * doppler);
            r = r / doppler;
        }
        else
        {
            // Receding side (redshift)
            r = min(1.0f, r * doppler);
            b = b / doppler;
        }
        
        return float4(r + variation, g + variation, b + variation, alpha);
    }
    else
    {
        // Outside the accretion disk
        return float4(0.0f, 0.0f, 0.0f, 0.0f);
    }
}

// Pixel Shader
float4 main(PixelInputType input) : SV_TARGET
{
    // Calculate the ray direction from the camera to the pixel
    float3 rayDirection = normalize(input.viewDirection);
    
    // Calculate the gravitational lensing effect
    float3 lensedDirection = CalculateGravitationalLensing(input.worldPosition, rayDirection);
    
    // Calculate the distance from the pixel to the black hole
    float distanceToBlackHole = length(blackHolePosition - input.worldPosition);
    
    // Check if the pixel is inside the event horizon
    if (distanceToBlackHole < schwarzschildRadius)
    {
        // Inside the event horizon - pure black
        return float4(0.0f, 0.0f, 0.0f, 1.0f);
    }
    
    // Check if the pixel is part of the accretion disk
    float4 accretionColor = CalculateAccretionDiskColor(input.worldPosition);
    
    // Sample the starfield texture using the lensed direction
    float2 texCoord = float2(
        0.5f + atan2(lensedDirection.z, lensedDirection.x) / (2.0f * 3.14159265f),
        0.5f - asin(lensedDirection.y) / 3.14159265f
    );
    
    float4 starfieldColor = starfieldTexture.Sample(samplerState, texCoord);
    
    // Blend the accretion disk color with the starfield color
    return lerp(starfieldColor, accretionColor, accretionColor.a);
}