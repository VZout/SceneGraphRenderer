Texture2D t0 : register(t0);
Texture2D t1 : register(t1);
Texture2D t2 : register(t2);
Texture2D t3 : register(t3);
Texture2D t4 : register(t5);
TextureCube t_reflect : register(t6);
SamplerState s0 : register(s0);

struct Light {
	float4 pos;
    float4 color;
	float constant;
    float lin;
    float quadratic;
	float radius;
};

cbuffer ConstantBuffer : register(b0)
{
	float4x4 view;
	float4x4 proj;
	float4x4 shadow_view;
	float4x4 shadow_proj;
	Light lights[10];
	uint num_lights;
};

struct VS_OUTPUT
{
	float4 pos: SV_POSITION;
	float2 texCoord: TEXCOORD0;
};

float3 CalcPointLight(Light light, float3 albedo, float specular_in, float3 normal, float3 frag_pos, float3 view_dir) {
	light.pos = mul(view, light.pos);
	float3 light_dir = normalize(light.pos.xyz - frag_pos);

	// diffuse
	float3 diffuse = max(dot(normal, light_dir), 0.0) * albedo * light.color;

	// specular
	float3 halfway_dir = normalize(light_dir + view_dir);
	float3 reflect_dir = reflect(-light_dir, normal);
	float spec = pow(max(dot(normal, reflect_dir), 0.0), 32.0);
	float3 specular = (light.color * spec) * specular_in;

	// attenuation
	float dist = length(light.pos.xyz - frag_pos);
    float attenuation = 1.0 / (light.constant + light.lin * dist + light.quadratic * dist * dist);  
    
	float3 result = float3(0, 0, 0);
	diffuse *= attenuation;
	specular *= attenuation;
	result += diffuse + specular;

	return result;
}

float oldtextureProj(float4 P, float layer, float2 offset)
{
	float shadow = 1.0;
	float4 shadowCoord = P / P.w;
	shadowCoord.xy = shadowCoord.xy * 0.5 + 0.5;
	
	if (shadowCoord.z > -1.0 && shadowCoord.z < 1.0) 
	{
		float dist = t3.Sample(s0, float3(shadowCoord.xy + offset, layer)).r;
		if (shadowCoord.w > 0.0 && dist < shadowCoord.z) 
		{
			shadow = 0.25;
		}
	}
	return shadow;
}

float textureProj(float4 P, float2 off)
{
	float shadow = 1.0;
	float4 shadowCoord = P / P.w;
	shadowCoord.xy = shadowCoord.xy * 0.5 + 0.5;

	if (shadowCoord.z > -1.0 && shadowCoord.z < 1.0) 
	{
		float dist = t3.Sample(s0, shadowCoord.xy + off).r;
		if (shadowCoord.w > 0.0 && dist < shadowCoord.z) 
		{
			shadow = 0.0;
		}
	}
	return shadow;
}

float filterPCF(float4 sc, float layer)
{
	float shadow_map_size = 2048;
	float scale = 1.5;
	float dx = scale * 1.0 / float(shadow_map_size);
	float dy = scale * 1.0 / float(shadow_map_size);

	float shadowFactor = 0.0;
	int count = 0;
	int range = 1;
	
	for (int x = -range; x <= range; x++)
	{
		for (int y = -range; y <= range; y++)
		{
			shadowFactor += textureProj(sc, float2(dx*x, dy*y));
			count++;
		}
	
	}
	return shadowFactor / count;
}


float4x4 inverse(float4x4 input)
{
#define minor(a,b,c) determinant(float3x3(input.a, input.b, input.c))
	//determinant(float3x3(input._22_23_23, input._32_33_34, input._42_43_44))
 
	float4x4 cofactors = float4x4(
		minor(_22_23_24, _32_33_34, _42_43_44),
		-minor(_21_23_24, _31_33_34, _41_43_44),
		minor(_21_22_24, _31_32_34, _41_42_44),
		-minor(_21_22_23, _31_32_33, _41_42_43),
 
		-minor(_12_13_14, _32_33_34, _42_43_44),
		minor(_11_13_14, _31_33_34, _41_43_44),
		-minor(_11_12_14, _31_32_34, _41_42_44),
		minor(_11_12_13, _31_32_33, _41_42_43),
 
		minor(_12_13_14, _22_23_24, _42_43_44),
		-minor(_11_13_14, _21_23_24, _41_43_44),
		minor(_11_12_14, _21_22_24, _41_42_44),
		-minor(_11_12_13, _21_22_23, _41_42_43),
 
		-minor(_12_13_14, _22_23_24, _32_33_34),
		minor(_11_13_14, _21_23_24, _31_33_34),
		-minor(_11_12_14, _21_22_24, _31_32_34),
		minor(_11_12_13, _21_22_23, _31_32_33)
		);
#undef minor
	return transpose(cofactors) / determinant(input);
}

float4 calc_static_sky_reflection(float3 normal, float3 view_dir, float metalic_map) {
	//float3 i = normalize(world_pos - view_pos);
	float3 r = reflect(view_dir, normal);
	float4 reflectColor = t_reflect.Sample(s0, r);
	reflectColor = reflectColor * metalic_map;
	reflectColor.a = 1;
 
	return reflectColor;
}

struct PS_OUTPUT
{
	float4 deferred : SV_TARGET0;
	float4 overdose : SV_TARGET1;
};

PS_OUTPUT main(VS_OUTPUT input) : SV_TARGET
{
	PS_OUTPUT psout;

	float4 temp_gbuffer = t0.Sample(s0, input.texCoord);
	float3 albado = temp_gbuffer.rgb;
	float specular = temp_gbuffer.a;
	float3 normal = normalize(t1.Sample(s0, input.texCoord).rgb);
	float3 frag_pos = t2.Sample(s0, input.texCoord).rgb;
	float ssao = t4.Sample(s0, input.texCoord).r;

	float amb = 0.1 * albado * ssao;
	float3 lighting = float3(amb, amb, amb);
	float3 view_dir = normalize(frag_pos.xyz);

	float4 reflect_color = calc_static_sky_reflection(normal, view_dir, t1.Sample(s0, input.texCoord).a);
	albado += reflect_color;

	float4x4 iv = transpose(inverse(-view));
	float4 modelpos = mul(float4(frag_pos.xyz, 1.0), iv);
	float4x4 pv = mul(shadow_proj, shadow_view);
	float4 shadowClip = mul(pv, modelpos);
	shadowClip.y *= -1;

	float shadowFactor = filterPCF(shadowClip, float2(0, 0));

	lighting *= shadowFactor;

	for(int i = 0; i < num_lights; i++)
    {
		float3 value = CalcPointLight(lights[i], albado, specular, normal, frag_pos, view_dir);
		lighting += value;
	}

	// overdose (bloom)
	float brightness = dot(lighting, float3(0.2126, 0.7152, 0.0722));
	if (brightness > 1.0)
		psout.overdose = float4(lighting, 1.0);
	else
		psout.overdose = float4(0.0, 0.0, 0.0, 1.0);


	// HDR
	//psout.overdose = float4(lighting, 1);
	psout.deferred = float4(lighting, 1);
	
	return psout;
}
