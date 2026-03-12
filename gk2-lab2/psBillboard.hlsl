cbuffer cbSurfaceColor : register(b0) //Pixel Shader constant buffer slot 0
{
	float4 surfaceColor;
}

struct PSInput
{
	float4 pos : SV_POSITION;
	float2 tex : TEXCOORD0;
};

float4 main(PSInput i) : SV_TARGET
{
	//TODO : 1.32. Calculate billboard pixel color
	
	return float4(0.0f, 0.0f, 0.0f, 1.0f); //Replace with correct implementation
}