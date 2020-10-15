//#############################################################################
//  File:      StereoOculus.frag
//  Purpose:   Oculus Rift Distortion Shader
//  Author:    Marc Wacker, Roman Kuehne
//  Date:      November 2013
//  Copyright: Marcus Hudritsch
//             This software is provide under the GNU General Public License
//             Please visit: http://opensource.org/licenses/GPL-3.0
//#############################################################################

precision highp float;

//-----------------------------------------------------------------------------
layout (location = 0) in vec4  a_position;     // Vertex position attribute

out   vec2	v_texCoord;
//-----------------------------------------------------------------------------
void main()
{
    gl_Position = vec4(a_position, 0.0, 1.0);
    v_texCoord = 0.5 * (a_position + 1.0);
}
//-----------------------------------------------------------------------------