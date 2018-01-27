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
	float3 frag: POSITION0;
	float3 light_pos: POSITION1;
	float3 normal : NORMAL;
	float2 texCoord: TEXCOORD0;
	float4 color: COLOR;
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

	
	float4x4 bm = mul(bones[input.ids[0]], input.weights[0]);
          bm += mul(bones[input.ids[1]], input.weights[1]);
          bm += mul(bones[input.ids[2]], input.weights[2]);
          bm += mul(bones[input.ids[3]], input.weights[3]);

	float4x4 mvp = mul(proj, mul(view, mul(model, bm)));
	float4x4 vp = mul(view, mul(model, bm));

    output.pos =  mul(mvp, float4(input.pos, 1.0f));
	output.color = float4(1, 0, 0, 1);
	output.frag = mul(vp, float4(input.pos, 1)).xyz;
	output.texCoord = input.texCoord;
	output.normal = mul(vp, input.normal);

	return output;
}
