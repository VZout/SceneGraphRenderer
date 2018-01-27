Texture2D t1 : register(t1);
Texture2D t2 : register(t2);
Texture2D t5 : register(t4);
SamplerState s0 : register(s0);

struct VS_OUTPUT
{
	float4 pos: SV_POSITION;
	float2 texCoord: TEXCOORD0;
};

cbuffer ConstantBuffer : register(b0)
{
	float4x4 view;
	float4x4 proj;
	float4 samples[32];
};

// removed normalizing of normal and random vec. Also removed * 2.0 * 0.5 or whatever at random vec
float4 main(VS_OUTPUT input) : SV_TARGET
{
	int kernel_size = 32;
	float radius = 0.2;

	float3 normal = t1.Sample(s0, input.texCoord).xyz;
	float3 frag_pos = t2.Sample(s0, input.texCoord).xyz;

	float2 screen_size;
	t2.GetDimensions(screen_size.x, screen_size.y);

	float2 noise_size;
	t5.GetDimensions(noise_size.x, noise_size.y);

	float2 noise_scale = screen_size / noise_size;
	float2 tex = noise_scale * input.texCoord;
	float3 random_vec = t5.Sample(s0, tex).xyz;

	float3 tangent = normalize(random_vec - normal * dot(random_vec, normal));
	float3 bitangent = cross(tangent, normal);
	float3x3 TBN =float3x3(tangent, bitangent, normal);

	float occlusion = 0.0;
	for (int i = 0; i < kernel_size; i++) {
		float3 sample_pos = mul(samples[i].xyz, TBN).xyz;
		sample_pos = frag_pos + sample_pos * radius;

		float4 offset = float4(sample_pos, 1.0);
		offset = mul(proj, offset);
		offset.y = -offset.y;
		offset.xyz /= offset.w;
		offset.xyz = offset.xyz * 0.5 + 0.5;

		float sample_depth = -t2.Sample(s0, offset).w;

		float range_check = smoothstep(0.0, 1.0, radius / abs(frag_pos.z - sample_depth));
		occlusion += (sample_depth >= sample_pos.z ? 1.0 : 0.0) * range_check;
	}

	occlusion = 1.0 - (occlusion / float(kernel_size));
	return occlusion;
}
