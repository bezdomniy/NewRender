/* Copyright (c) 2025, Sascha Willems
 *
 * SPDX-License-Identifier: MIT
 *
 */

struct VSInput
{
    float2 Pos;
};

struct VSOutput
{
    float4 Pos : SV_POSITION;
    // float PSize : SV_PointSize;
    // float GradientPos;
    // float2 CenterPos;
    // float PointSize;
};

Sampler2D samplerColorMap;
Sampler2D samplerGradientRamp;

struct UBO
{
    float4x4 projection;
    float4x4 modelview;
    float2 screendim;
};
ConstantBuffer<UBO> ubo;

[shader("vertex")]
VSOutput vertMain(VSInput input)
{
    VSOutput output;
    output.Pos = float4(input.Pos, 0.0, 1.0);
    // output.PSize = 10.0;
    // const float spriteSize = 0.005 * input.Pos.w; // Point size influenced by mass (stored in input.Pos.w);

    // float4 eyePos = mul(ubo.modelview, float4(input.Pos.x, input.Pos.y, input.Pos.z, 1.0));
    // float4 projectedCorner = mul(ubo.projection, float4(0.5 * spriteSize, 0.5 * spriteSize, eyePos.z, eyePos.w));
    // output.PSize = output.PointSize = clamp(ubo.screendim.x * projectedCorner.x / projectedCorner.w, 1.0, 128.0);

    // output.Pos = mul(ubo.projection, eyePos);
    // output.CenterPos = ((output.Pos.xy / output.Pos.w) + 1.0) * 0.5 * ubo.screendim;

    // output.GradientPos = input.Vel.w;
    return output;
}

[shader("fragment")]
float4 fragMain(VSOutput input) : SV_TARGET
{
    // float3 color = samplerGradientRamp.Sample(float2(input.GradientPos, 0.0)).rgb;
    // float2 PointCoord = (input.Pos.xy - input.CenterPos.xy) / input.PointSize + 0.5;
    // return float4(samplerColorMap.Sample(PointCoord).rgb * color, 1);

    return float4(1, 0, 0, 1);
}