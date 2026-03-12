cbuffer cbWorld : register(b0) //Vertex Shader constant buffer slot 0 - matches slot in vsBilboard.hlsl
{
	matrix worldMatrix;
};

cbuffer cbView : register(b1) //Vertex Shader constant buffer slot 1 - matches slot in vsBilboard.hlsl
{
	matrix viewMatrix;
	matrix invViewMatrix;
};

cbuffer cbProj : register(b2) //Vertex Shader constant buffer slot 2 - matches slot in vsBilboard.hlsl
{
	matrix projMatrix;
};

//DONE : 0.4. Change vertex shader input structure to match new vertex type
struct VSInput
{
	float3 pos : POSITION;
	float3 nor : NORMAL;
};

//DONE : 0.5. Change vertex shader output structure to store position, normal and view vectors in global coordinates
struct PSInput
{
	float4 pos : SV_POSITION;
    float4 worldPos : WORLD_POSITION;
    float4 nor : NORMAL;
    float4 viewVector : VIEW_VECTOR;
};
PSInput main(VSInput i)
{
	PSInput o;
    o.worldPos = mul(worldMatrix, float4(i.pos, 1.0f));

	//DONE : 0.6. Store global (world) position in the output

    float4 pos = mul(viewMatrix, o.worldPos);
    o.pos = mul(projMatrix, pos);

	//DONE : 0.7. Calculate output normal and view vectors in global coordinates frame
	//Hint: you can calculate camera position from inverted view matrix

    o.nor = mul(worldMatrix, float4(i.nor, 0.0f));
    float4 cameraPos = mul(invViewMatrix, float4(0.0f, 0.0f, 0.0f, 1.0f));
    o.viewVector = normalize(cameraPos - o.worldPos);
	
	return o;
}