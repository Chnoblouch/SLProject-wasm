//#############################################################################
//  File:      SLGLTextureIBL.h
//  Author:    Carlos Arauz, Marcus Hudritsch
//  Date:      April 2018
//  Codestyle: https://github.com/cpvrlab/SLProject/wiki/Coding-Style-Guidelines
//  Copyright: Marcus Hudritsch
//             This software is provide under the GNU General Public License
//             Please visit: http://opensource.org/licenses/GPL-3.0
//#############################################################################

#ifndef SLGLTEXTUREGENERATED_H
#define SLGLTEXTUREGENERATED_H

#include <CVImage.h>
#include <SLGLVertexArray.h>
#include <SLGLProgram.h>
#include <SLGLTexture.h>
#include <SLGLFrameBuffer.h>

//-----------------------------------------------------------------------------
//! Texture object generated in run time from another texture
/*!
This class is mainly used to generate the textures used for reflections in the
IBL techniques. It takes a source texture and projects it into a cube map with different
rendering techniques to produces the textures needed in IBL like the irradiance or the
prefilter map. This textures can be given to the SLMaterial with IBL. It uses the
SLGLFrameBuffer class to render the scene into a cube map.
*/
class SLGLTextureIBL : public SLGLTexture
{
public:
    //! Default constructor
    SLGLTextureIBL() : SLGLTexture() { ; }

    //! ctor for generated textures
    SLGLTextureIBL(SLAssetManager*  assetMgr,
                   SLstring         shaderPath,
                   SLGLTexture*     sourceTexture,
                   SLGLFrameBuffer* fbo,
                   SLVec2i          size,
                   SLTextureType    texType,
                   SLenum           target,
                   SLint            min_filter = GL_LINEAR,
                   SLint            mag_filter = GL_LINEAR);

    virtual ~SLGLTextureIBL();

    virtual void   clearData();
    virtual void   build(SLint texID = 0);
    virtual SLuint width() { return _generatedSize.x; }
    virtual SLuint height() { return _generatedSize.y; }
    virtual SLint  bytesPerPixel() { return _generatedBytesPerPixel; }

protected:
    // converting the hdr image file to cubemap
    void renderCube();
    void renderQuad();

    SLuint _cubeVAO = 0;
    SLuint _cubeVBO = 0;
    SLuint _quadVAO = 0;
    SLuint _quadVBO = 0;

    SLGLTexture*     _sourceTexture;          //!< 2D Texture from the HDR Image
    SLGLProgram*     _shaderProgram;          //!< shader program to render the texture
    SLGLFrameBuffer* _captureFBO;             //!< Frame Buffer Object for capturing the textures
    SLMat4f          _captureProjection;      //!< Projection matrix for capturing the textures
    SLVMat4f         _captureViews;           //!< all 6 positions of the views that represent the 6 sides of the cube map
    SLVec2i          _generatedSize;          //! Generated size of texture (there is no _image in SLGLTexture)
    SLuint           _generatedBytesPerPixel; //! Generated bytesPerPixel of texture (there is no _image in SLGLTexture)
};
//-----------------------------------------------------------------------------
#endif
