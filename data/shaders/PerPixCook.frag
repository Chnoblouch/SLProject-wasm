//#############################################################################
//  File:      PerPixCook.frag
//  Purpose:   GLSL fragment shader for Cook-Torrance physical based rendering.
//             Based on the physically based rendering (PBR) tutorial with GLSL
//             from Joey de Vries on https://learnopengl.com/#!PBR/Theory
//  Author:    Marcus Hudritsch
//  Date:      July 2017
//  Copyright: Marcus Hudritsch
//             This software is provide under the GNU General Public License
//             Please visit: http://opensource.org/licenses/GPL-3.0
//#############################################################################

#ifdef GL_ES
precision highp float;
#endif

//-----------------------------------------------------------------------------
// SLGLShader::preprocessPragmas replaces #Lights by SLVLights.size()
#pragma define NUM_LIGHTS #Lights
//-----------------------------------------------------------------------------
in      vec3    v_P_VS;              // Interpol. point of illum. in view space (VS)
in      vec3    v_N_VS;              // Interpol. normal at v_P_VS in view space

uniform bool    u_lightIsOn[NUM_LIGHTS];      // flag if light is on
uniform vec4    u_lightPosVS[NUM_LIGHTS];     // position of light in view space
uniform vec4    u_lightDiffuse[NUM_LIGHTS];   // diffuse light intensity (Id)
uniform float   u_oneOverGamma;      // 1.0f / Gamma correction value

uniform vec4    u_matDiffuse;        // diffuse color reflection coefficient (kd)
uniform float   u_matRoughness;      // Cook-Torrance material roughness 0-1
uniform float   u_matMetallic;       // Cook-Torrance material metallic 0-1

uniform int     u_camProjection;     // type of stereo
uniform int     u_camStereoEye;      // -1=left, 0=center, 1=right
uniform mat3    u_camStereoColors;   // color filter matrix
uniform bool    u_camFogIsOn;        // flag if fog is on
uniform int     u_camFogMode;        // 0=LINEAR, 1=EXP, 2=EXP2
uniform float   u_camFogDensity;     // fog densitiy value
uniform float   u_camFogStart;       // fog start distance
uniform float   u_camFogEnd;         // fog end distance
uniform vec4    u_camFogColor;       // fog color (usually the background)

out     vec4    o_fragColor;        // output fragment color
//-----------------------------------------------------------------------------
const float AO = 1.0;               // Constant ambient occlusion factor
const float PI = 3.14159265359;
//-----------------------------------------------------------------------------
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}
//-----------------------------------------------------------------------------
float distributionGGX(vec3 N, vec3 H, float roughness)
{
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}
//-----------------------------------------------------------------------------
float geometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}
//-----------------------------------------------------------------------------
float geometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = geometrySchlickGGX(NdotV, roughness);
    float ggx1  = geometrySchlickGGX(NdotL, roughness);
    return ggx1 * ggx2;
}
//-----------------------------------------------------------------------------
void pointLightCookTorrance(in    int   i,         // Light number
                            in    vec3  P_VS,      // Point of illumination in VS
                            in    vec3  N,         // Normalized normal at v_P_VS
                            in    vec3  V,         // Normalized vector from v_P_VS to view in VS
                            in    vec3  F0,        // Frenel reflection at 90 deg. (0 to N)
                            in    vec3  diffuse,   // Material diffuse color
                            in    float roughness, // Material roughness
                            in    float metallic,  // Material metallic
                            inout vec3  Lo)        // reflected intensity
{
    vec3 L = u_lightPosVS[i].xyz - v_P_VS;         // Vector from v_P_VS to the light in VS
    float distance = length(L);                    // distance to light
    L /= distance;                                 // normalize light vector
    vec3 H = normalize(V + L);                     // Normalized halfvector between eye and light vector
    float att = 1.0 / (distance*distance);         // quadratic light attenuation
    vec3 radiance = u_lightDiffuse[i].rgb * att;   // per light radiance

     // cook-torrance brdf
     float NDF = distributionGGX(N, H, roughness);
     float G   = geometrySmith(N, V, L, roughness);
     vec3  F   = fresnelSchlick(max(dot(H, V), 0.0), F0);

     vec3 kS = F;
     vec3 kD = vec3(1.0) - kS;
     kD *= 1.0 - metallic;

     vec3  nominator   = NDF * G * F;
     float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.001;
     vec3  specular    = nominator / denominator;

     // add to outgoing radiance Lo
     float NdotL = max(dot(N, L), 0.0);

     Lo += (kD*diffuse/PI + specular) * radiance * NdotL;
}
//-----------------------------------------------------------------------------
#pragma include "fogBlend.glsl"
#pragma include "doStereoSeparation.glsl
//-----------------------------------------------------------------------------
void main()
{
    vec3 N = normalize(v_N_VS);     // A input normal has not anymore unit length
    vec3 V = normalize(-v_P_VS);    // Vector from p to the viewer
    vec3 F0 = vec3(0.04);           // Init Frenel reflection at 90 deg. (0 to N)
    F0 = mix(F0, u_matDiffuse.rgb, u_matMetallic);
    vec3 Lo = vec3(0.0);            // Get the reflection from all lights into Lo

    for (int i = 0; i < NUM_LIGHTS; ++i)
    {
        if (u_lightIsOn[i])
        {
            //if (u_lightPosVS[i].w == 0.0)
            //   directLightBlinnPhong(i, N, E, Ia, Id, Is);
            //else
            pointLightCookTorrance(i, v_P_VS, N, V, F0, u_matDiffuse.rgb, u_matRoughness, u_matMetallic, Lo);
        }
    }

    // ambient lighting (note that the next IBL tutorial will replace
    // this ambient lighting with environment lighting).
    vec3 ambient = vec3(0.03) * u_matDiffuse.rgb * AO;
    vec3 color = ambient + Lo;

    // HDR tonemapping
    color = color / (color + vec3(1.0));
    o_fragColor = vec4(color, 1.0);

    // Apply fog by blending over distance
    if (u_camFogIsOn)
        o_fragColor = fogBlend(v_P_VS, o_fragColor);

    // Apply gamma correction
    o_fragColor.rgb = pow(o_fragColor.rgb, vec3(u_oneOverGamma));

    // Apply stereo eye separation
    if (u_camProjection > 1)
        doStereoSeparation();
}
//-----------------------------------------------------------------------------
