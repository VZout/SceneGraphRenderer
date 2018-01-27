struct VS_INPUT
{
	float3 pos : POSITION;
	float3 normal : NORMAL;
	float2 texCoord: TEXCOORD;
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
};

cbuffer ConstantBuffer : register(b0)
{
	float4x4 bones[100];
	float4x4 model;
	float instanced;
	float3 shit;
};

cbuffer ConstantBufferPV : register(b1)
{
	float4x4 view;
	float4x4 proj;
};

float4x4 translate(float4x4 mat, float3 pos) {
	float4x4 m = {
    { 1, 0, 0, 0 },
    { 0, 1, 0, 0 },
    { 0, 0, 1, 0 },
    { 0, 0, 0, 1 }
	};

	m[0][3] = pos.x;
	m[1][3] = pos.y;
	m[2][3] = pos.z;

	return m;
}

VS_OUTPUT main(VS_INPUT input)
{
	VS_OUTPUT output;

	float3 pp;
	float4x4 mm;
	if (instanced == 1) {
		mm = mul(translate(model, input.inst_pos), model);
	} else {
		mm = model;
	}

	pp = input.pos;

	float4x4 mvp = mul(mul(proj, view), mm);
	float4x4 vp = mul(view, mm);
 
    output.pos =  mul(mvp, float4(pp, 1.0f));
	output.frag = mul((vp), float4(pp, 1));
	output.texCoord = input.texCoord;
	output.normal = mul((vp), input.normal);

	return output;
}
