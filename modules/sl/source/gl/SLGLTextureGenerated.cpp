//#############################################################################
//  File:      SLGLTextureGenerated.cpp
//  Author:    Carlos Arauz, Marcus Hudritsch
//  Date:      April 2018
//  Codestyle: https://github.com/cpvrlab/SLProject/wiki/Coding-Style-Guidelines
//  Copyright: Marcus Hudritsch
//             This software is provide under the GNU General Public License
//             Please visit: http://opensource.org/licenses/GPL-3.0
//#############################################################################

#include <SLAssetManager.h>
#include <SLScene.h>
#include <SLGLTextureGenerated.h>

//-----------------------------------------------------------------------------
//! ctor for generated textures from hdr textures
SLGLTextureGenerated::SLGLTextureGenerated(SLAssetManager*  am,
                                           SLstring         shaderPath,
                                           SLGLTexture*     sourceTexture,
                                           SLGLFrameBuffer* fbo,
                                           SLTextureType    texType,
                                           SLenum           target,
                                           SLint            min_filter,
                                           SLint            mag_filter,
                                           SLint            wrapS,
                                           SLint            wrapT)
{
    if (sourceTexture != nullptr)
    {
        assert(sourceTexture->texType() >= TT_hdr);
        assert(texType > TT_hdr);
    }

    _sourceTexture        = sourceTexture;
    _captureFBO           = fbo;
    _texType              = texType;
    _min_filter           = min_filter;
    _mag_filter           = mag_filter;
    _wrap_s               = wrapS;
    _wrap_t               = wrapT;
    _target               = target;
    _texID                = 0;
    _bumpScale            = 1.0f;
    _resizeToPow2         = false;
    _autoCalcTM3D         = false;
    _needsUpdate          = false;
    _bytesOnGPU           = 0;
    _generatedWidth       = 0;
    _generatedHeight      = 0;
    _generatedBytesPerPixel = 0;

    name("Generated " + typeName());

    if (texType == TT_environmentCubemap)
        _shaderProgram = new SLGLProgramGeneric(am,
                                                shaderPath + "PBR_CubeMap.vert",
                                                shaderPath + "PBR_CylinderToCubeMap.frag");
    if (texType == TT_irradianceCubemap)
        _shaderProgram = new SLGLProgramGeneric(am,
                                                shaderPath + "PBR_CubeMap.vert",
                                                shaderPath + "PBR_IrradianceConvolution.frag");
    if (texType == TT_roughnessCubemap)
        _shaderProgram = new SLGLProgramGeneric(am,
                                                shaderPath + "PBR_CubeMap.vert",
                                                shaderPath + "PBR_PrefilterRoughness.frag");
    if (texType == TT_brdfLUT)
        _shaderProgram = new SLGLProgramGeneric(am,
                                                shaderPath + "PBR_BRDFIntegration.vert",
                                                shaderPath + "PBR_BRDFIntegration.frag");

    // perspective projection with field of view of 90 degrees
    _captureProjection.perspective(90.0f, 1.0f, 0.1f, 10.0f);

    // initialize capture views: 6 views each at all 6 directions of the faces of a cube map
    SLMat4f mat;
    //clang-format off
    mat.lookAt(0, 0, 0, 1, 0, 0, 0, -1, 0);
    _captureViews.push_back(mat);
    mat.lookAt(0, 0, 0, -1, 0, 0, 0, -1, 0);
    _captureViews.push_back(mat);
    mat.lookAt(0, 0, 0, 0, 1, 0, 0, 0, 1);
    _captureViews.push_back(mat);
    mat.lookAt(0, 0, 0, 0, -1, 0, 0, 0, -1);
    _captureViews.push_back(mat);
    mat.lookAt(0, 0, 0, 0, 0, 1, 0, -1, 0);
    _captureViews.push_back(mat);
    mat.lookAt(0, 0, 0, 0, 0, -1, 0, -1, 0);
    _captureViews.push_back(mat);
    //clang-format on

    if (_sourceTexture != nullptr)
        _sourceTexture->bindActive();

    build();

    // Add pointer to the global resource vectors for deallocation
    if (am)
        am->textures().push_back(this);
}
//-----------------------------------------------------------------------------
SLGLTextureGenerated::~SLGLTextureGenerated()
{
    clearData();
}
//-----------------------------------------------------------------------------
void SLGLTextureGenerated::clearData()
{
    glDeleteTextures(1, &_texID);

    numBytesInTextures -= _bytesOnGPU;

    for (SLint i = 0; i < _images.size(); ++i)
    {
        delete _images[i];
        _images[i] = 0;
    }
    _images.clear();

    _texID      = 0;
    _bytesOnGPU = 0;
    _vaoSprite.clearAttribs();
}
//-----------------------------------------------------------------------------
/*!
Build the texture into a cube map, rendering the texture 6 times and capturing each time
one side of the cube (except for the BRDF LUT texture, which is completely generated by
calculations directly with the shader).
*/
void SLGLTextureGenerated::build(SLint texID)
{
    if (_captureFBO->fboId() && _captureFBO->rboId())
    {
        _captureFBO->bind();
        _captureFBO->bindRenderBuffer();

        glGenTextures(1, &_texID);
        glBindTexture(_target, _texID);

        if (_texType == TT_environmentCubemap || _texType == TT_irradianceCubemap)
        {
            _generatedWidth       = _captureFBO->rboWidth();
            _generatedHeight      = _captureFBO->rboHeight();
            _generatedBytesPerPixel = 3 * 2; // GL_RGB16F

            for (SLuint i = 0; i < 6; i++)
            {
                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                             0,
                             GL_RGB16F,
                             _captureFBO->rboWidth(),
                             _captureFBO->rboHeight(),
                             0,
                             GL_RGB,
                             GL_FLOAT,
                             nullptr);
            }

            glTexParameteri(_target, GL_TEXTURE_WRAP_S, _wrap_s);
            glTexParameteri(_target, GL_TEXTURE_WRAP_T, _wrap_t);
            glTexParameteri(_target, GL_TEXTURE_WRAP_R, _wrap_t);
            glTexParameteri(_target, GL_TEXTURE_MIN_FILTER, _min_filter);
            glTexParameteri(_target, GL_TEXTURE_MAG_FILTER, _mag_filter);

            _shaderProgram->useProgram();
            _shaderProgram->uniform1i("u_texture0", 0);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(_sourceTexture->target(),
                          _sourceTexture->texID());

            glViewport(0, 0, _captureFBO->rboWidth(), _captureFBO->rboHeight());
            _captureFBO->bind();

            for (SLuint i = 0; i < 6; i++)
            {
                SLMat4f mvp = _captureProjection * _captureViews[i];
                _shaderProgram->uniformMatrix4fv("u_mvpMatrix", 1, mvp.m());
                _captureFBO->attachTexture2D(GL_COLOR_ATTACHMENT0,
                                             GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                                             this);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                renderCube();
            }

            _captureFBO->unbind();

            if (_texType == TT_environmentCubemap)
            {
                glBindTexture(GL_TEXTURE_CUBE_MAP, _texID);
                glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
            }
        }
        else if (_texType == TT_roughnessCubemap)
        {
            assert(_sourceTexture->texType() == TT_environmentCubemap &&
                   "the source texture is not an environment map");
            _captureFBO->unbind();

            _generatedWidth       = 128;
            _generatedHeight      = 128;
            _generatedBytesPerPixel = 3 * 2; // GL_RGB16F

            for (unsigned int i = 0; i < 6; ++i)
            {
                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                             0,
                             GL_RGB16F,
                             _generatedWidth,
                             _generatedHeight,
                             0,
                             GL_RGB,
                             GL_FLOAT,
                             nullptr);
            }
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

            _shaderProgram->useProgram();
            _shaderProgram->uniform1i("u_texture0", 0);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(_sourceTexture->target(), _sourceTexture->texID());

            _captureFBO->bindRenderBuffer();
            SLuint maxMipLevels = 5;
            for (SLuint mip = 0; mip < maxMipLevels; ++mip)
            {
                // resize framebuffer according to mip-level size
                SLuint mipWidth  = _generatedWidth * pow(0.5, mip);
                SLuint mipHeight = _generatedHeight * pow(0.5, mip);
                _captureFBO->bufferStorage(mipWidth, mipHeight);
                glViewport(0, 0, mipWidth, mipHeight);

                SLfloat roughness = (SLfloat)mip / (SLfloat)(maxMipLevels - 1);
                _shaderProgram->uniform1f("u_roughness", roughness);
                for (SLuint i = 0; i < 6; ++i)
                {
                    SLMat4f mvp = _captureProjection * _captureViews[i];
                    _shaderProgram->uniformMatrix4fv("u_mvpMatrix", 1, mvp.m());
                    _captureFBO->attachTexture2D(GL_COLOR_ATTACHMENT0,
                                                 GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                                                 this,
                                                 mip);
                    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                    renderCube();
                }

                _captureFBO->unbind();
            }
        }
        else if (_texType == TT_brdfLUT)
        {
            _captureFBO->unbind();

            _generatedWidth       = 512;
            _generatedHeight      = 512;
            _generatedBytesPerPixel = 2 * 2; // GL_RG16F

            glTexImage2D(_target,
                         0,
                         GL_RG16F,
                         _generatedWidth,
                         _generatedHeight,
                         0,
                         GL_RG,
                         GL_FLOAT,
                         0);
            glTexParameteri(_target, GL_TEXTURE_WRAP_S, _wrap_s);
            glTexParameteri(_target, GL_TEXTURE_WRAP_T, _wrap_t);
            glTexParameteri(_target, GL_TEXTURE_MIN_FILTER, _min_filter);
            glTexParameteri(_target, GL_TEXTURE_MAG_FILTER, _mag_filter);

            _captureFBO->bufferStorage(_generatedWidth, _generatedHeight);
            _captureFBO->attachTexture2D(GL_COLOR_ATTACHMENT0, _target, this);

            glViewport(0, 0, _generatedWidth, _generatedHeight);
            _shaderProgram->useProgram();
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            renderQuad();
            _captureFBO->unbind();
        }
    }
}
//-----------------------------------------------------------------------------
//! renders 1x1 cube, used to project the source texture and then capture on if its sides
void SLGLTextureGenerated::renderCube()
{
    // initialize (if necessary)
    if (_cubeVAO == 0)
    {
        // clang-format off
        float vertices[] = {
            // back face
            -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
             1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
             1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
             1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
            -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
            -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
            // front face
            -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
             1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
             1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
             1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
            -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
            -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
            // left face
            -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
            -1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
            -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
            -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
            -1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
            -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
            // right face
             1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
             1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
             1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right         
             1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
             1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
             1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left     
            // bottom face
            -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
             1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
             1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
             1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
            -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
            -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
            // top face
            -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
             1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
             1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right     
             1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
            -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
            -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left        
        };
        // clang-format on
        glGenVertexArrays(1, &_cubeVAO);
        glGenBuffers(1, &_cubeVBO);
        // fill buffer
        glBindBuffer(GL_ARRAY_BUFFER, _cubeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        // link vertex attributes
        glBindVertexArray(_cubeVAO);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
    // render Cube
    glBindVertexArray(_cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}
//-----------------------------------------------------------------------------
//! renders a 1x1 XY quad, used for rendering and capturung the BRDF integral solution
void SLGLTextureGenerated::renderQuad()
{
    if (_quadVAO == 0)
    {
        // clang-format off
        float quadVertices[] = {
            // positions         // texture Coords
            -1.0f,  1.0f, 0.0f,  0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f,  0.0f, 0.0f,
             1.0f,  1.0f, 0.0f,  1.0f, 1.0f,
             1.0f, -1.0f, 0.0f,  1.0f, 0.0f,
        };
        // clang-format on

        // setup plane VAO
        glGenVertexArrays(1, &_quadVAO);
        glGenBuffers(1, &_quadVBO);
        glBindVertexArray(_quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, _quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }
    glBindVertexArray(_quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}
//-----------------------------------------------------------------------------
