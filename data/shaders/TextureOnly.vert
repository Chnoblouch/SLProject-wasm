//#############################################################################
//  File:      TextureOnly.vert
//  Purpose:   GLSL vertex program for texture mapping only
//  Date:      July 2014
//  Authors:   Marcus Hudritsch
//  License:   This software is provided under the GNU General Public License
//             Please visit: http://opensource.org/licenses/GPL-3.0
//#############################################################################

precision highp float;

//-----------------------------------------------------------------------------
layout (location = 0) in vec4  a_position;  // Vertex position attribute
layout (location = 2) in vec2  a_uv0;       // Vertex texture attribute

uniform mat4  u_mMatrix;    // Model matrix
uniform mat4  u_vMatrix;    // View matrix
uniform mat4  u_pMatrix;    // Projection matrix

out     vec2  v_uv0;        // texture coordinate at vertex
//-----------------------------------------------------------------------------
void main()
{
    // Set the texture coord. output for interpolated tex. coords.
    v_uv0 = a_uv0;
   
    // Set the transformes vertex position   
    gl_Position = u_pMatrix * u_vMatrix * u_mMatrix * a_position;
}
//-----------------------------------------------------------------------------
