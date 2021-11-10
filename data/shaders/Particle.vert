//#############################################################################
//  File:      ColorAttribute.vert
//  Purpose:   GLSL vertex program for simple per vertex attribute color
//  Date:      July 2014
//  Authors:   Marcus Hudritsch
//  License:   This software is provided under the GNU General Public License
//             Please visit: http://opensource.org/licenses/GPL-3.0
//#############################################################################

precision highp float;

//-----------------------------------------------------------------------------
in          vec3     a_position;       // Vertex position attribute


uniform mat4 u_mvMatrix;  // modelview matrix

//-----------------------------------------------------------------------------
void main()
{
    vec4 P = vec4(a_position.xyz, 1.0);
    gl_Position = u_mvMatrix * P; 
}
//-----------------------------------------------------------------------------
