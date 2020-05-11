//#############################################################################
//  File:      SLGLDepthBuffer.h
//  Purpose:   Uses an OpenGL framebuffer object as a depth-buffer
//  Author:    Michael Schertenleib
//  Date:      May 2020
//  Copyright: Michael Schertenleib
//             This software is provide under the GNU General Public License
//             Please visit: http://opensource.org/licenses/GPL-3.0
//#############################################################################

#include <stdafx.h> // Must be the 1st include followed by  an empty line

#include <SLGLState.h>
#include <SLGLDepthBuffer.h>

//-----------------------------------------------------------------------------
SLGLDepthBuffer::SLGLDepthBuffer(SLuint w,
                                 SLuint h,
                                 SLenum magFilter,
                                 SLenum minFilter,
                                 SLint  wrap,
                                 float  borderColor[]) : _width(w), _height(h)
{
    SLint previousFrameBuffer;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &previousFrameBuffer);

    // Init framebuffer.
    glGenFramebuffers(1, &_fboID);
    glBindFramebuffer(GL_FRAMEBUFFER, _fboID);

    glGenTextures(1, &_texID);
    glBindTexture(GL_TEXTURE_2D, _texID);

    // Texture parameters.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);

    if (borderColor != nullptr)
    {
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    }

    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_DEPTH_COMPONENT,
                 w,
                 h,
                 0,
                 GL_DEPTH_COMPONENT,
                 GL_FLOAT,
                 nullptr);

    // Attach texture to framebuffer.
    glFramebufferTexture2D(GL_FRAMEBUFFER,
                           GL_DEPTH_ATTACHMENT,
                           GL_TEXTURE_2D,
                           _texID,
                           0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    glBindFramebuffer(GL_FRAMEBUFFER,
                      previousFrameBuffer);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cerr << "FBO failed to initialize correctly." << std::endl;
}
//-----------------------------------------------------------------------------
SLGLDepthBuffer::~SLGLDepthBuffer()
{
    glDeleteTextures(1, &_texID);
    glDeleteFramebuffers(1, &_fboID);
}
//-----------------------------------------------------------------------------
void SLGLDepthBuffer::activateAsTexture(const int       progId,
                                        const SLstring& glSamplerName,
                                        const int       textureUnit)
{
    glActiveTexture(GL_TEXTURE0 + textureUnit);
    glBindTexture(GL_TEXTURE_2D, _texID);
    glUniform1i(glGetUniformLocation(progId,
                                     glSamplerName.c_str()),
                textureUnit);
}
//-----------------------------------------------------------------------------
void SLGLDepthBuffer::bind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, _fboID);
}
//-----------------------------------------------------------------------------
