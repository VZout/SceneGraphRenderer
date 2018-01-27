Texture2D t0 : register(t0);
Texture2D specular_texture : register(t1);
SamplerState s0 : register(s0);

struct VS_OUTPUT
{
	float4 pos: SV_POSITION;
	float3 frag: POSITION0;
	float3 light_pos: POSITION1;
	float3 normal : NORMAL;
	float2 texCoord: TEXCOORD0;
	float4 color: COLOR;
};

struct PS_OUTPUT
{
	float4 albedo : SV_TARGET0;
	float4 normal : SV_TARGET1;
	float4 pos : SV_TARGET2;
};

float linearDepth(float depth)
{
	float NEAR_PLANE = 0.1f; //todo: specialization const
	float FAR_PLANE = 64.0f; //todo: specialization const 
	float z = depth * 2.0f - 1.0f;
	return (2.0f * NEAR_PLANE * FAR_PLANE) / (FAR_PLANE + NEAR_PLANE - z * (FAR_PLANE - NEAR_PLANE));
}

PS_OUTPUT main(VS_OUTPUT input) : SV_TARGET
{
	float4 dc = t0.Sample(s0, input.texCoord);
	dc.a = specular_texture.Sample(s0, input.texCoord);

	PS_OUTPUT psout;
	psout.albedo = dc;
	psout.normal = float4(normalize(input.normal), 1);
	psout.pos = float4(input.frag, linearDepth(input.pos.z));
	return psout;
}
