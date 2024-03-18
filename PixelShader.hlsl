#include "Lighting.hlsli"

cbuffer ExternalData : register(b0)
{
	float2 uvScale;
	float2 uvOffset;
	float3 cameraPosition;
	int lightCount;
	Light lights[MAX_LIGHTS];
}

// Struct representing the data we expect to receive from earlier pipeline stages
// - Should match the output of our corresponding vertex shader
// - The name of the struct itself is unimportant
// - The variable names don't have to match other shaders (just the semantics)
// - Each variable must have a semantic, which defines its usage
struct VertexToPixel
{
	// Data type
	//  |
	//  |   Name          Semantic
	//  |    |                |
	//  v    v                v
	float4 screenPosition	: SV_POSITION;
	float3 normal			: NORMAL;
	float2 uv				: TEXCOORD;
	float3 tangent			: TANGENT;
	float3 worldPos			: POSITION;
};

Texture2D AlbedoTexture			: register(t0);
Texture2D NormalMap				: register(t1);
Texture2D RoughnessMap			: register(t2);
Texture2D MetalMap				: register(t3);
SamplerState BasicSampler		: register(s0);

// --------------------------------------------------------
// The entry point (main method) for our pixel shader
// 
// - Input is the data coming down the pipeline (defined by the struct)
// - Output is a single color (float4)
// - Has a special semantic (SV_TARGET), which means 
//    "put the output of this into the current render target"
// - Named "main" because that's the default the shader compiler looks for
// --------------------------------------------------------
float4 main(VertexToPixel input) : SV_TARGET
{
	input.uv = input.uv * uvScale + uvOffset;

	float3 unpackedNormal = NormalMap.Sample(BasicSampler, input.uv).rgb * 2.0f - 1.0f;
	unpackedNormal = normalize(unpackedNormal);	//Don't forget to normalize

	//Feel free to adjust/simplify this code to fit with your existing shader(s)
	//Simplifications include not re-normalizing the same vector more than once!
	float3 N = normalize(input.normal); //Must be normalized here or before
	float3 T = normalize(input.tangent); //Must be normalized here or before
	T = normalize(T - N * dot(T, N)); //Gram-Schmidt assumes T&N are normalized
	float3 B = cross(T, N);
	float3x3 TBN = float3x3(T, B, N);

	//Assumes that input.normal is the normal later in the shader
	input.normal = mul(unpackedNormal, TBN); //Note the multiplication order

	float3 albedoColor = pow(AlbedoTexture.Sample(BasicSampler, input.uv).rgb, 2.2f);

	float roughnessPBR = RoughnessMap.Sample(BasicSampler, input.uv).r;

	float metalness = MetalMap.Sample(BasicSampler, input.uv).r;

	float3 specularColor = lerp(F0_NON_METAL, albedoColor.rgb, metalness);

	float3 lightResult = float3(0.0f, 0.0f, 0.0f);

	for (int i = 0; i < lightCount; i++)
	{
		switch (lights[i].Type)
		{
			case LIGHT_TYPE_DIRECTIONAL:
				//lightResult += DirLight(lights[i], input.normal, cameraPos, input.worldPosition, albedoColor, roughness, specScale);
				lightResult += DirLightPBR(lights[i], input.normal, cameraPosition, input.worldPos, albedoColor, specularColor, roughnessPBR, metalness);
				break;

			case LIGHT_TYPE_POINT:
				//lightResult += PointLight(lights[i], input.normal, cameraPos, input.worldPosition, albedoColor, roughness, specScale);
				lightResult += PointLightPBR(lights[i], input.normal, cameraPosition, input.worldPos, albedoColor, specularColor, roughnessPBR, metalness);
				break;
		}
	}

	return float4(pow(lightResult, 1.0f / 2.2f), 1);
}