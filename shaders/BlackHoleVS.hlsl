// Constant buffer for matrices
cbuffer MatrixBuffer : register(b0)
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};

// Input vertex structure
struct VertexInputType
{
    float3 position : POSITION;
    float2 texCoord : TEXCOORD0;
};

// Output vertex structure
struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD0;
    float3 worldPosition : TEXCOORD1;
    float3 viewDirection : TEXCOORD2;
};

// Vertex Shader
PixelInputType main(VertexInputType input)
{
    PixelInputType output;
    
    // Change the position vector to be 4 units for proper matrix calculations
    float4 worldPosition = mul(float4(input.position, 1.0f), worldMatrix);
    
    // Calculate the position of the vertex against the world, view, and projection matrices
    output.position = mul(worldPosition, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    
    // Store the texture coordinates for the pixel shader
    output.texCoord = input.texCoord;
    
    // Store the world position for the pixel shader
    output.worldPosition = worldPosition.xyz;
    
    // Calculate the view direction (from camera to vertex)
    float4 cameraPosition = float4(viewMatrix._41, viewMatrix._42, viewMatrix._43, 1.0f);
    output.viewDirection = normalize(worldPosition.xyz - cameraPosition.xyz);
    
    return output;
}