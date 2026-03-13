struct Light
{
	float4 position;
	float4 color;
};

struct Lighting
{
	float4 ambient;
	float4 surface;
	Light lights[3];
};

cbuffer cbSurfaceColor : register(b0) //Pixel Shader constant buffer slot 0 - matches slot in psBilboard.hlsl
{
	float4 surfaceColor;
}

cbuffer cbLighting : register(b1) //Pixel Shader constant buffer slot 1
{
	Lighting lighting;
}

//DONE : 0.8. Modify pixel shader input structure to match vertex shader output
struct PSInput
{
    float4 pos : SV_POSITION;
    float4 worldPos : WORLD_POSITION;
    float4 nor : NORMAL;
    float4 viewVector : VIEW_VECTOR;
};

float4 main(PSInput i) : SV_TARGET
{
	//DONE : 0.9. Calculate output color using Phong Illumination Model

    float4 normal = normalize(i.nor);
    //return normal * 0.5f + 0.5f;
    float3 ambient = surfaceColor * 0.1f;
    float3 diffuse = surfaceColor;
    float3 specular = float3(1.0f, 1.0f, 1.0f);
	
    float4 lightPosition = float4(2.0f, 5.5f, 1.0f, 1.0f);
    float4 lightDirection = normalize(i.worldPos - lightPosition);
	
    float diff = max(dot(normal, -lightDirection), 0.0f);
    diffuse *= diff;
	
    float4 reflectDir = reflect(lightDirection, normal);
    float spec = pow(max(dot(i.viewVector, reflectDir), 0.0f), 50.0f);
    specular *= spec;
    //return i.viewVector * 0.5f + 0.5f;
    return float4(min(ambient+diffuse+specular,1.0f), 1.0f);
}