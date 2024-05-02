Texture2D tex : register(t0);
SamplerState samplerWrap : register(s0);


struct Vin
{
    float2 pos : POSITION;
    float2 texCoord : TEXCOORD0;
};

struct Vout
{
    float4 Pos : SV_Position;
    float2 texCoord : TEXCOORD0;
};


cbuffer WorldTransform : register(b0)
{
    float4x4 ProjectionTransform;
};

cbuffer ObjectBuffer : register(b1)
{
    float4x4 constantBuffer;
    float4 color;
};


Vout VS(Vin vin)
{    
    Vout vout;
    vout.Pos = mul(float4(vin.pos, 0, 1), constantBuffer);
    vout.Pos = mul(vout.Pos, ProjectionTransform);
    vout.texCoord = vin.texCoord;
    return vout;
}


float4 PS(Vout vin) : SV_Target
{
    return color;
}
