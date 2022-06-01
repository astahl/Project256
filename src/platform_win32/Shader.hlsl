struct FragmentData {
	float4 position : POSITION;
	float2 uv : TEXCOORD;
};

FragmentData vertexShader(uint vertexIndex : SV_VertexID) {
	const float2 uvs[4] = { {0,0}, {0, 1}, {1, 0}, {1,1} };
	const float2 pos[4] = { {-1,-1}, {-1, 1}, {1, -1}, {1,1} };

	const float2 scale = { 0.5, 1.0 };

	FragmentData result;
	result.position = float4(0.0f, 0.0f, 0.0f, 1.0f);
	result.position.xy = pos[vertexIndex] * scale;
	result.uv = pos[vertexIndex];
	return result;
}


float4 pixelShader(FragmentData data) : SV_TARGET
{
	float4 color = float4(0.0, 0.0, 0.0, 1.0);
	color.xy = data.uv;
	return color;
}