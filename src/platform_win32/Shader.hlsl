cbuffer RootConstants : register(b0) {
	float2 scale;
};

Texture2D g_texture : register(t0);
SamplerState g_sampler : register(s0);

struct FragmentData {
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD;
};


FragmentData vertexShader(uint vertexIndex : SV_VertexID) {
	const float2 uvs[4] = { {0,0}, {0, 1}, {1, 0}, {1,1} };
	const float2 pos[4] = { {-1,-1}, {-1, 1}, {1, -1}, {1,1} };

	FragmentData result;
	result.position = float4(0.0f, 0.0f, 0.0f, 1.0f);
	result.position.xy = pos[vertexIndex] * scale;
	result.uv = uvs[vertexIndex];
	return result;
}

float4 pixelShader(FragmentData data) : SV_TARGET
{
	return g_texture.Sample(g_sampler, data.uv);
}

float4 pixelShaderFiltered(FragmentData data) : SV_TARGET
{
	const float bias = 0.5;
	const float remBias = (1.0 - bias) * 0.25;
	const float spread = 0.3;
	uint texWidth = 0;
	uint texHeight = 0;
	g_texture.GetDimensions(texWidth, texHeight);
	const float2 stepSize = 0.25 * spread * float2(1.0f / texWidth, 1.0f / texHeight);

	float4 color = float4(0, 0, 0, 0);
	// +++++++++
	// +     1 +
	// +4      +
	// +   0   +
	// +      2+
	// + 3     +
	// +++++++++

	const float2 samplePoint1 = data.uv + float2(stepSize.x, 2 * stepSize.y);
	const float2 samplePoint2 = data.uv + float2(2 * stepSize.x, -stepSize.y);
	const float2 samplePoint3 = data.uv + float2(-stepSize.x, -2 * stepSize.y);
	const float2 samplePoint4 = data.uv + float2(-2 * stepSize.x, stepSize.y);

	color += bias * g_texture.Sample(g_sampler, data.uv);
	color += remBias * g_texture.Sample(g_sampler, samplePoint1);
	color += remBias * g_texture.Sample(g_sampler, samplePoint2);
	color += remBias * g_texture.Sample(g_sampler, samplePoint3);
	color += remBias * g_texture.Sample(g_sampler, samplePoint4);
	return color;
}