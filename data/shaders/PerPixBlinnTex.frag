//#############################################################################
//  File:      PerPixBlinnTex.frag
//  Purpose:   GLSL per pixel lighting with texturing
//  Author:    Marcus Hudritsch
//  Date:      July 2014
//  Copyright: Marcus Hudritsch
//             This software is provide under the GNU General Public License
//             Please visit: http://opensource.org/licenses/GPL-3.0
//#############################################################################

/*
The preprocessor constant #define NUM_LIGHTS will be added at the shader
compilation time. It must be constant to be used in the for loop in main().
Therefore this number it can not be passed as a uniform variable.
*/

#ifdef GL_ES
precision mediump float;
#endif

//-----------------------------------------------------------------------------
in      vec3        v_P_VS;     // Interpol. point of illum. in view space (VS)
in      vec3        v_N_VS;     // Interpol. normal at v_P_VS in view space
in      vec2        v_texCoord; // Interpol. texture coordinate in tex. space

uniform bool        u_lightIsOn[NUM_LIGHTS];        // flag if light is on
uniform vec4        u_lightPosVS[NUM_LIGHTS];       // position of light in view space
uniform vec4        u_lightAmbient[NUM_LIGHTS];     // ambient light intensity (Ia)
uniform vec4        u_lightDiffuse[NUM_LIGHTS];     // diffuse light intensity (Id)
uniform vec4        u_lightSpecular[NUM_LIGHTS];    // specular light intensity (Is)
uniform vec3        u_lightSpotDirVS[NUM_LIGHTS];   // spot direction in view space
uniform float       u_lightSpotCutoff[NUM_LIGHTS];  // spot cutoff angle 1-180 degrees
uniform float       u_lightSpotCosCut[NUM_LIGHTS];  // cosine of spot cutoff angle
uniform float       u_lightSpotExp[NUM_LIGHTS];     // spot exponent
uniform vec3        u_lightAtt[NUM_LIGHTS];         // attenuation (const,linear,quadr.)
uniform bool        u_lightDoAtt[NUM_LIGHTS];       // flag if att. must be calc.
uniform vec4        u_globalAmbient;                // Global ambient scene color
uniform float       u_oneOverGamma;                 // 1.0f / Gamma correction value

uniform vec4        u_matAmbient;       // ambient color reflection coefficient (ka)
uniform vec4        u_matDiffuse;       // diffuse color reflection coefficient (kd)
uniform vec4        u_matSpecular;      // specular color reflection coefficient (ks)
uniform vec4        u_matEmissive;      // emissive color for self-shining materials
uniform float       u_matShininess;     // shininess exponent
uniform sampler2D   u_matTexture0;      // Color texture map

uniform int         u_camProjection;    // type of stereo
uniform int         u_camStereoEye;     // -1=left, 0=center, 1=right
uniform mat3        u_camStereoColors;  // color filter matrix
uniform bool        u_camFogIsOn;       // flag if fog is on
uniform int         u_camFogMode;       // 0=LINEAR, 1=EXP, 2=EXP2
uniform float       u_camFogDensity;    // fog densitiy value
uniform float       u_camFogStart;      // fog start distance
uniform float       u_camFogEnd;        // fog end distance
uniform vec4        u_camFogColor;      // fog color (usually the background)

out     vec4        o_fragColor;        // output fragment color
//-----------------------------------------------------------------------------
void directLightBlinnPhong(in    int  i,    // Light number
                           in    vec3 N,    // Normalized normal at P_VS
                           in    vec3 E,    // Normalized vector from P_VS to eye in VS
                           inout vec4 Ia,   // Ambient light intensity
                           inout vec4 Id,   // Diffuse light intensity
                           inout vec4 Is)   // Specular light intensity
{  
    // We use the spot light direction as the light direction vector
    vec3 L = normalize(-u_lightSpotDirVS[i].xyz);
   
    // Calculate diffuse & specular factors
    float diffFactor = max(dot(N,L), 0.0);
    float specFactor = 0.0;
    
    if (diffFactor!=0.0)
    {
        vec3 H = normalize(L+E); // Half vector H between L and E
        specFactor = pow(max(dot(N,H), 0.0), u_matShininess);
    }
   
    // accumulate directional light intesities w/o attenuation
    Ia += u_lightAmbient[i];
    Id += u_lightDiffuse[i] * diffFactor;
    Is += u_lightSpecular[i] * specFactor;
}
//-----------------------------------------------------------------------------
void pointLightBlinnPhong( in    int  i,    // OpenGL light number
                           in    vec3 E,    // Normalized vector from v_P_VS
                           in    vec3 S,    // normalized spot direction
                           in    vec3 N,    // Normalized normal at v_P_VS
                           inout vec4 Ia,   // Ambient light intensity
                           inout vec4 Id,   // Diffuse light intensity
                           inout vec4 Is)   // Specular light intensity
{  
    // Vector from v_P_VS to the light in VS
    vec3 L = u_lightPosVS[i].xyz - v_P_VS;

    // Normalized halfvector between N and L
    vec3 H = normalize(L + E);
      
    // Calculate attenuation over distance & normalize L
    float att = 1.0;
    if (u_lightDoAtt[i])
    {
        vec3 att_dist;
        att_dist.x = 1.0;
        att_dist.z = dot(L,L);         // = distance * distance
        att_dist.y = sqrt(att_dist.z); // = distance
        att = 1.0 / dot(att_dist, u_lightAtt[i]);
        L /= att_dist.y;               // = normalize(L)
    } else L = normalize(L);
   
    // Calculate diffuse & specular factors
    float diffFactor = max(dot(N,L), 0.0);
    float specFactor = 0.0;
    if (diffFactor!=0.0) 
        specFactor = pow(max(dot(N,H), 0.0), u_matShininess);
   
    // Calculate spot attenuation
    if (u_lightSpotCutoff[i] < 180.0)
    {
        float spotDot; // Cosine of angle between L and spotdir
        float spotAtt; // Spot attenuation
        spotDot = dot(-L, S);
        if (spotDot < u_lightSpotCosCut[i]) spotAtt = 0.0;
        else spotAtt = max(pow(spotDot, u_lightSpotExp[i]), 0.0);
        att *= spotAtt;
    }
   
    // Accumulate light intesities
    Ia += att * u_lightAmbient[i];
    Id += att * u_lightDiffuse[i] * diffFactor;
    Is += att * u_lightSpecular[i] * specFactor;
}
//-----------------------------------------------------------------------------
vec4 fogBlend(vec3 P_VS, vec4 inColor)
{
    float factor = 0.0f;
    float distance = length(P_VS);

    switch (u_camFogMode)
    {
        case 0:
        factor = (u_camFogEnd - distance) / (u_camFogEnd - u_camFogStart);
        break;
        case 1:
        factor = exp(-u_camFogDensity * distance);
        break;
        default:
        factor = exp(-u_camFogDensity * distance * u_camFogDensity * distance);
        break;
    }

    vec4 outColor = factor * inColor + (1 - factor) * u_camFogColor;
    outColor = clamp(outColor, 0.0, 1.0);
    return outColor;
}
//-----------------------------------------------------------------------------
void doStereoSeparation()
{
    // See SLProjection in SLEnum.h
    if (u_camProjection > 8) // stereoColors
    {
        // Apply color filter but keep alpha
        o_fragColor.rgb = u_camStereoColors * o_fragColor.rgb;
    }
    else if (u_camProjection == 6) // stereoLineByLine
    {
        if (mod(floor(gl_FragCoord.y), 2.0) < 0.5)// even
        {
            if (u_camStereoEye ==-1)
            discard;
        } else // odd
        {
            if (u_camStereoEye == 1)
            discard;
        }
    }
    else if (u_camProjection == 7) // stereoColByCol
    {
        if (mod(floor(gl_FragCoord.x), 2.0) < 0.5)// even
        {
            if (u_camStereoEye ==-1)
            discard;
        } else // odd
        {
            if (u_camStereoEye == 1)
            discard;
        }
    }
    else if (u_camProjection == 8) // stereoCheckerBoard
    {
        bool h = (mod(floor(gl_FragCoord.x), 2.0) < 0.5);
        bool v = (mod(floor(gl_FragCoord.y), 2.0) < 0.5);
        if (h==v)// both even or odd
        {
            if (u_camStereoEye ==-1)
            discard;
        } else // odd
        {
            if (u_camStereoEye == 1)
            discard;
        }
    }
}
//-----------------------------------------------------------------------------
void main()
{
    vec4 Ia = vec4(0.0); // Accumulated ambient light intensity at v_P_VS
    vec4 Id = vec4(0.0); // Accumulated diffuse light intensity at v_P_VS
    vec4 Is = vec4(0.0); // Accumulated specular light intensity at v_P_VS
   
    vec3 N = normalize(v_N_VS);  // A input normal has not anymore unit length
    vec3 E = normalize(-v_P_VS);  // Vector from p to the eye

    for (int i = 0; i < NUM_LIGHTS; ++i)
    {
        if (u_lightIsOn[i])
        {
            vec3 S = u_lightSpotDirVS[i];

            if (u_lightPosVS[i].w == 0.0)
                directLightBlinnPhong(i, N, E, Ia, Id, Is);
            else
                pointLightBlinnPhong(i, N, E, S, Ia, Id, Is);
        }
    }

    // Sum up all the reflected color components
    o_fragColor =  u_globalAmbient +
                    u_matEmissive + 
                    Ia * u_matAmbient +
                    Id * u_matDiffuse;

    // Componentwise multiply w. texture color
    o_fragColor *= texture(u_matTexture0, v_texCoord);

    // add finally the specular RGB-part
    vec4 specColor = Is * u_matSpecular;
    o_fragColor.rgb += specColor.rgb;

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
