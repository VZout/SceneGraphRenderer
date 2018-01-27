Texture2D t4 : register(t0);
SamplerState s0 : register(s0);

struct VS_OUTPUT
{
	float4 pos: SV_POSITION;
	float2 texCoord: TEXCOORD0;
};

cbuffer ConstantBuffer : register(b0)
{
	float4x4 proj;
	float4 samples[32];
};

float blur5(float2 uv, float2 resolution) {
  float color = 0.0;
  float2 off1 = float2(1.411764705882353, 1.411764705882353);
  color += t4.Sample(s0, uv).r * 0.1964825501511404;
  color += t4.Sample(s0, uv + (off1 / resolution)).r * 0.2969069646728344;
  color += t4.Sample(s0, uv - (off1 / resolution)).r * 0.2969069646728344;
  return color;
}

float blur9(float2 uv, float2 resolution) {
  float color = 0;
  float2 off1 = float2(1.411764705882353, 1.411764705882353);
  float2 off2 = float2(3.2941176470588234, 3.2941176470588234);
  color += t4.Sample(s0, uv) * 0.1964825501511404;
  color += t4.Sample(s0, uv + (off1 / resolution)).r * 0.2969069646728344;
  color += t4.Sample(s0, uv - (off1 / resolution)).r * 0.2969069646728344;
  color += t4.Sample(s0, uv + (off2 / resolution)).r * 0.09447039785044732;
  color += t4.Sample(s0, uv - (off2 / resolution)).r * 0.09447039785044732;
  return color;
}

float blur13(float2 uv, float2 resolution, float2 dir) {
	float color = 0;
	float2 off1 = float2(1.411764705882353, 1.411764705882353) * dir;
	float2 off2 = float2(3.2941176470588234, 3.2941176470588234) * dir;
	float2 off3 = float2(5.176470588235294, 5.176470588235294) * dir;
	color += t4.Sample(s0, uv).r * 0.1964825501511404;
	color += t4.Sample(s0, uv + (off1 / resolution)).r * 0.2969069646728344;
	color += t4.Sample(s0, uv - (off1 / resolution)).r * 0.2969069646728344;
	color += t4.Sample(s0, uv + (off2 / resolution)).r * 0.09447039785044732;
	color += t4.Sample(s0, uv - (off2 / resolution)).r * 0.09447039785044732;
	color += t4.Sample(s0, uv + (off3 / resolution)).r * 0.010381362401148057;
	color += t4.Sample(s0, uv - (off3 / resolution)).r * 0.010381362401148057;
	return color;
}

float4 main(VS_OUTPUT input) : SV_TARGET
{
	float2 res;
	t4.GetDimensions(res.x, res.y);

	float2 dir = float2(1, 1);
	float result = blur13((input.texCoord), res, dir);
	return result;
}


