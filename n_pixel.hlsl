Texture2D t0 : register(t0);
Texture2D specular_texture : register(t1);
Texture2D normal_texture : register(t2);
SamplerState s0 : register(s0);

struct VS_OUTPUT
{
	float4 pos: SV_POSITION;
	float4 frag: POSITION0;
	float3 light_pos: POSITION1;
	float3 normal : NORMAL;
	float2 texCoord: TEXCOORD0;
	float4 color: COLOR;
	float3 tangent: TANGENT;
};

float linearDepth(float depth)
{
	float NEAR_PLANE = 0.1f; //todo: specialization const
	float FAR_PLANE = 64.0f; //todo: specialization const 
	float z = depth * 2.0f - 1.0f; 
	return (2.0f * NEAR_PLANE * FAR_PLANE) / (FAR_PLANE + NEAR_PLANE - z * (FAR_PLANE - NEAR_PLANE));	
}

struct PS_OUTPUT
{
	float4 albedo : SV_TARGET0;
	float4 normal : SV_TARGET1;
	float4 pos : SV_TARGET2;
};

PS_OUTPUT main(VS_OUTPUT input) : SV_TARGET
{
	float4 dc = t0.Sample(s0, input.texCoord);
	dc.a = specular_texture.Sample(s0, input.texCoord);

	PS_OUTPUT psout;
	psout.albedo = dc;
	psout.normal = float4(normalize(input.normal), 1);

	float3 N = normalize(input.normal);
	float3 T = normalize(input.tangent);
	float3 B = cross(N, T);
	
	float3x3 TBN = float3x3(T, B, N);

	float3 tnorm = mul(TBN, normalize(normal_texture.Sample(s0, input.texCoord).rgb * 2.0 - float3(1.0, 1.0, 1.0)));
	psout.normal = float4(tnorm, 1.0);

	//psout.normal = normal_texture.Sample(s0, input.texCoord*2);
	//psout.normal = normalize(psout.normal * 2.0 - 1.0);   
	//psout.normal = float4(normalize(mul(input.tbn, psout.normal)), 1); 
	
	psout.pos = input.frag;
	psout.pos.w = linearDepth(input.pos.z);

	return psout;
}
