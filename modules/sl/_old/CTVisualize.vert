//#############################################################################
//  File:      CTVisualize.vert
//  Purpose:   Voxelization visualization
//  Date:      September 2018
//  Authors:   Stefan Thoeni
//  License:   This software is provided under the GNU General Public License
//             Please visit: http://opensource.org/licenses/GPL-3.0
//#############################################################################

precision highp float;

//-----------------------------------------------------------------------------
layout(location = 0) in vec4 a_position;

out vec2 textureCoordinateFrag;
//-----------------------------------------------------------------------------
// Scales and bias a given vector (i.e. from [-1, 1] to [0, 1]).
vec2 scaleAndBias(vec2 p)
{
	return 0.5f * p + vec2(0.5f);
}
//-----------------------------------------------------------------------------
void main()
{
	textureCoordinateFrag = scaleAndBias(a_position.xy);
	gl_Position = a_position;
}
//-----------------------------------------------------------------------------
