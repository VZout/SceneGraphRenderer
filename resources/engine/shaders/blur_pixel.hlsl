Texture2D overdose_texture : register(t1);
Texture2D self_texture : register(t2);
SamplerState s0 : register(s0);

struct VS_OUTPUT
{
	float4 pos: SV_POSITION;
	float2 texCoord: TEXCOORD0;
};

float4 blur13(Texture2D t, float2 uv, float2 resolution, float2 dir) {
	float4 color = float4(0, 0, 0, 0);
	float2 off1 = float2(1.411764705882353, 1.411764705882353) * dir;
	float2 off2 = float2(3.2941176470588234, 3.2941176470588234) * dir;
	float2 off3 = float2(5.176470588235294, 5.176470588235294) * dir;
	color += t.Sample(s0, uv) * 0.1964825501511404;
	color += t.Sample(s0, uv + (off1 / resolution)) * 0.2969069646728344;
	color += t.Sample(s0, uv - (off1 / resolution)) * 0.2969069646728344;
	color += t.Sample(s0, uv + (off2 / resolution)) * 0.09447039785044732;
	color += t.Sample(s0, uv - (off2 / resolution)) * 0.09447039785044732;
	color += t.Sample(s0, uv + (off3 / resolution)) * 0.010381362401148057;
	color += t.Sample(s0, uv - (off3 / resolution)) * 0.010381362401148057;
	return color;
}

float4 main(VS_OUTPUT input) : SV_TARGET
{
	const float gamma = 1.0;
	const float exposure = 1.0;

	float4 bloom = float4(0, 0, 0, 0);
	float2 res = float2(1280.f, 720.f);

	float2 uv = float2(input.texCoord) * 2.f;
	float w = self_texture.Sample(s0, uv).w;

	if (w > 0.60 && w < 0.7) {
		bloom = blur13(self_texture, uv, res, float2(1, 0));
		w+= 0.01;
	} else if (w > 0.70) {
		bloom = blur13(self_texture, uv, res, float2(0, 1));
		w+= 0.01;
	} else {
		bloom = blur13(overdose_texture, uv, res, float2(1, 0));
		w = 0.65;
	}

	return float4(bloom.xyz, w);
}
