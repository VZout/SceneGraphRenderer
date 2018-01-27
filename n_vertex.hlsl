struct VS_INPUT
{
	float3 pos : POSITION;
	float3 normal : NORMAL;
	float2 texCoord: TEXCOORD;
	float3 tangent: TANGENT0;
	float3 bitangent: TANGENT1;
	float4 weights: BLENDWEIGHT;
	uint4 ids: BLENDINDICES;
	float3 inst_pos : TEXCOORD1;
};

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

cbuffer ConstantBuffer : register(b0)
{
	float4x4 bones[100];
	float4x4 model;
	float instanced;
};

cbuffer ConstantBufferPV : register(b1)
{
	float4x4 view;
	float4x4 proj;
};

VS_OUTPUT main(VS_INPUT input)
{
	VS_OUTPUT output;

	float4x4 tmodel = transpose(model);

	float3 pp;
	if (instanced == 1) {
		pp = input.pos + input.inst_pos;
	} else {
		pp = input.pos;
	}

	float4x4 mvp = mul(mul(proj, view), model);
	float4x4 vp = mul(view, model);
 
    output.pos =  mul(mvp, float4(pp, 1.0f));
	output.color = float4(1, 0, 0, 1);
	output.frag = mul((vp), float4(pp, 1));
	output.texCoord = input.texCoord;
	output.normal = mul((vp), input.normal);

	output.normal = mul(transpose(vp), normalize(float4(input.normal, 1))).xyz;
	output.tangent = mul(transpose(vp), normalize(float4(input.tangent, 1))).xyz;	

	return output;
}
