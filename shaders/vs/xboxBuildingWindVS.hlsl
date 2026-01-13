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
float4		fxParams	: register(c36);
float4		envXform	: register(c37);
float3x3	envmat		: register(c38);

#define surfAmb (surfProps.x)
#define surfDiff (surfProps.z)
#define shininess (fxParams.x)
#define lightmult (fxParams.y)

struct VS_INPUT
{
	float4 Position		: POSITION;
	float3 Normal		: NORMAL;
	float2 TexCoord		: TEXCOORD0;
	float4 NightColor	: COLOR0;
	float4 DayColor		: COLOR1;
};

struct VS_OUTPUT {
	float4 Position		: POSITION;
	float2 Texcoord0	: TEXCOORD0;
	float2 Texcoord1	: TEXCOORD1;
	float4 Color		: COLOR0;
	float4 Envcolor		: COLOR1;
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

	return lerp(lerp(hash(n + 0.0), hash(n + 1.0), f.x),
				lerp(hash(n + 57.0), hash(n + 58.0), f.x), f.y);
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
	OUT.Texcoord1 = mul(envmat, IN.Normal).xy * envXform.zw;

	float4 prelight = dayColor * dayparam + IN.NightColor * nightparam;
	OUT.Color = (prelight * surfDiff + float4(ambient, 0.0) * surfAmb) * matCol;

	OUT.Envcolor = float4(1.0, 1.0, 1.0, 0.0) * shininess * lightmult;

	return OUT;
}
