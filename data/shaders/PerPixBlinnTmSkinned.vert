//#############################################################################
//  File:      PerPixBlinnTmSkinned.vert
//  Purpose:   GLSL vertex program for per pixel Blinn-Phong lighting with
//             Texture Mapping and vertex skinning
//  Author:    Marc Wacker and Marcus Hudritsch
//  Date:      January 2015
//  Copyright: Marcus Hudritsch
//             This software is provide under the GNU General Public License
//             Please visit: http://opensource.org/licenses/GPL-3.0
//#############################################################################

precision highp float;

//-----------------------------------------------------------------------------
layout (location = 0) in vec4  a_position;      // Vertex position attribute
layout (location = 1) in vec3  a_normal;        // Vertex normal attribute
layout (location = 2) in vec2  a_uv1;           // Vertex texture coordinate attribute
layout (location = 6) in vec4  a_jointIds;      // Vertex joint indices attributes
layout (location = 7) in vec4  a_jointWeights;  // Vertex joint weights attributes

uniform mat4  u_mvMatrix;           // modelview matrix
uniform mat3  u_nMatrix;            // normal matrix=transpose(inverse(mv))
uniform mat4  u_mvpMatrix;          // = projection * modelView
uniform mat4  u_jointMatrices[100]; // joint matrices for skinning

out     vec3  v_P_VS;       // Point of illumination in view space (VS)
out     vec3  v_N_VS;       // Normal at P_VS in view space
out     vec2  v_uv1;        // Texture coordinate output
//-----------------------------------------------------------------------------
void main(void)
{  
    // In skinned skeleton animation every vertex of a mesh is transformed by
    // max. four joints (bones) of a skeleton identified by indices. The joint
    // matrix is a weighted sum fore joint matrices and can change per frame
    // to animate the mesh
    mat4 jm = u_jointMatrices[int(a_jointIds.x)] * a_jointWeights.x
            + u_jointMatrices[int(a_jointIds.y)] * a_jointWeights.y
            + u_jointMatrices[int(a_jointIds.z)] * a_jointWeights.z
            + u_jointMatrices[int(a_jointIds.w)] * a_jointWeights.w;

    // Build the 3x3 submatrix in GLSL 110 (= mat3 jt3 = mat3(jt))
    // for the normal transform that is the normally the inverse transpose.
    // The inverse transpose can be ignored as long as we only have
    // rotation and uniform scaling in the 3x3 submatrix.
    mat3 jnm;
    jnm[0][0] = jm[0][0]; jnm[1][0] = jm[1][0]; jnm[2][0] = jm[2][0];
    jnm[0][1] = jm[0][1]; jnm[1][1] = jm[1][1]; jnm[2][1] = jm[2][1];
    jnm[0][2] = jm[0][2]; jnm[1][2] = jm[1][2]; jnm[2][2] = jm[2][2];

    v_P_VS = vec3(u_mvMatrix * jm * a_position);
    v_N_VS = vec3(u_nMatrix * jnm * a_normal);

    v_uv1 = a_uv1;

    // Transform the vertex with the modelview and joint matrix
    gl_Position = u_mvpMatrix * jm * a_position;
}
//-----------------------------------------------------------------------------