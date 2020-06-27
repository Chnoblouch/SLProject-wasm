//#############################################################################
//  File:      TextureOnly.vert
//  Purpose:   GLSL vertex program for texture mapping only
//  Author:    Marcus Hudritsch
//  Date:      July 2014
//  Copyright: Marcus Hudritsch
//             This software is provide under the GNU General Public License
//             Please visit: http://opensource.org/licenses/GPL-3.0
//#############################################################################

//-----------------------------------------------------------------------------
in      vec4     a_position;    // Vertex position attribute
in      vec3     a_texCoord;    // Vertex texture coord. attribute

uniform mat4     u_mvpMatrix;   // = projection * modelView

out     vec2     v_texCoord;    // texture coordinate at vertex
//-----------------------------------------------------------------------------
void main()
{
    // Set the texture coord. output for interpolated tex. coords.
    v_texCoord = a_texCoord.xy;
   
    // Set the transformes vertex position   
    gl_Position = u_mvpMatrix * a_position;
}
//-----------------------------------------------------------------------------
