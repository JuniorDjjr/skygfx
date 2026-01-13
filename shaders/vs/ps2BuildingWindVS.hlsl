float4x4	combined	: register(c0);
float3		ambient		: register(c4);
float4		matCol		: register(c19);
float3		surfProps	: register(c20);
float2		windPos		: register(c21);
float		windIntensity : register(c22);

float4		shaderParams	: register(c29);
float4		dayparam	: register(c30);
float4		nightparam	: register(c31);
float4x4	texmat		: register(c32);

#define colorScale (shaderParams.x)
#define surfAmb (surfProps.x)

struct VS_INPUT
{
	float4 Position		: POSITION;
	float2 TexCoord		: TEXCOORD0;
	float4 NightColor	: COLOR0;
	float4 DayColor		: COLOR1;
};

struct VS_OUTPUT {
	float4 Position		: POSITION;
	float2 Texcoord0	: TEXCOORD0;
	float4 Color		: COLOR0;
};

float hash(float n)
{
	return frac(sin(n) * 43758.5453);
}

float noise(float2 x)
{
	float2 p = floor(x);
	float2 f = frac(x);

	f = f * f * (3.0 - 2.0 * f);
	float n = p.x + p.y * 57.0;

	return lerp(lerp(lerp(hash(n + 0.0), hash(n + 1.0), f.x),
				lerp(hash(n + 57.0), hash(n + 58.0), f.x), f.y),
		lerp(lerp(hash(n + 113.0), hash(n + 114.0), f.x),
			 lerp(hash(n + 170.0), hash(n + 171.0), f.x), f.y), 0.5);
}

VS_OUTPUT main(in VS_INPUT IN)
{
	VS_OUTPUT OUT;

	float4 dayColor = IN.DayColor;
	float intensityMask = (-dayColor.a + 1.0f);
	dayColor.a = IN.NightColor.a; // day vertex alpha is used for wind mask, so consider as night vertex alpha

	float4 PositionEdited = IN.Position;

	float dist = ((noise(windPos + PositionEdited.xy) - 0.5) * windIntensity * intensityMask);

	PositionEdited.x = (PositionEdited.x + dist);
	PositionEdited.y = (PositionEdited.y + dist);
	PositionEdited.z = (PositionEdited.z + (dist * 0.3));

	OUT.Position = mul(PositionEdited, combined);

	OUT.Texcoord0 = mul(texmat, float4(IN.TexCoord, 0.0, 1.0)).xy;

	OUT.Color = dayColor * dayparam + IN.NightColor * nightparam;
	OUT.Color *= matCol / colorScale;
	OUT.Color.rgb += ambient * surfAmb;

	return OUT;
}
