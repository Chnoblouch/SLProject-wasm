//#############################################################################
//  File:      SLGLProgramManager.cpp
//  Author:    Michael Goettlicher
//  Date:      March 2020
//  Codestyle: https://github.com/cpvrlab/SLProject/wiki/SLProject-Coding-Style
//  Copyright: Marcus Hudritsch
//             This software is provide under the GNU General Public License
//             Please visit: http://opensource.org/licenses/GPL-3.0
//#############################################################################

#include <stdafx.h> // Must be the 1st include followed by  an empty line

#include <SLGLProgramManager.h>

std::map<SLStdShaderProg, SLGLGenericProgram*> SLGLProgramManager::_programs;
std::string                                    SLGLProgramManager::_shaderDir;
//-----------------------------------------------------------------------------
void SLGLProgramManager::init(std::string shaderDir)
{
    _shaderDir = shaderDir;
}
//-----------------------------------------------------------------------------
SLGLGenericProgram* SLGLProgramManager::get(SLStdShaderProg id)
{
    auto it = _programs.find(id);
    if (it == _programs.end())
    {
        makeProgram(id);
    }

    return _programs[id];
}
//-----------------------------------------------------------------------------
void SLGLProgramManager::deletePrograms()
{
    for (auto it : _programs)
        delete it.second;
    _programs.clear();
}
//-----------------------------------------------------------------------------
void SLGLProgramManager::makeProgram(SLStdShaderProg id)
{
    assert(!_shaderDir.empty());

    switch (id)
    {
        case SP_colorAttribute:
            _programs.insert({id, new SLGLGenericProgram(nullptr, _shaderDir + "ColorAttribute.vert", _shaderDir + "Color.frag")});
            break;
        case SP_colorUniform:
            _programs.insert({id, new SLGLGenericProgram(nullptr, _shaderDir + "ColorUniform.vert", _shaderDir + "Color.frag")});
            break;
        case SP_perVrtBlinn:
            _programs.insert({id, new SLGLGenericProgram(nullptr, _shaderDir + "PerVrtBlinn.vert", _shaderDir + "PerVrtBlinn.frag")});
            break;
        case SP_perVrtBlinnColorAttrib:
            _programs.insert({id, new SLGLGenericProgram(nullptr, _shaderDir + "PerVrtBlinnColorAttrib.vert", _shaderDir + "PerVrtBlinn.frag")});
            break;
        case SP_perVrtBlinnTex:
            _programs.insert({id, new SLGLGenericProgram(nullptr, _shaderDir + "PerVrtBlinnTex.vert", _shaderDir + "PerVrtBlinnTex.frag")});
            break;
        case SP_TextureOnly:
            _programs.insert({id, new SLGLGenericProgram(nullptr, _shaderDir + "TextureOnly.vert", _shaderDir + "TextureOnly.frag")});
            break;
        case SP_perPixBlinn:
            _programs.insert({id, new SLGLGenericProgram(nullptr, _shaderDir + "PerPixBlinn.vert", _shaderDir + "PerPixBlinn.frag")});
            break;
        case SP_perPixBlinnTex:
            _programs.insert({id, new SLGLGenericProgram(nullptr, _shaderDir + "PerPixBlinnTex.vert", _shaderDir + "PerPixBlinnTex.frag")});
            break;
        case SP_perPixCookTorrance:
            _programs.insert({id, new SLGLGenericProgram(nullptr, _shaderDir + "PerPixCookTorrance.vert", _shaderDir + "PerPixCookTorrance.frag")});
            break;
        case SP_perPixCookTorranceTex:
            _programs.insert({id, new SLGLGenericProgram(nullptr, _shaderDir + "PerPixCookTorranceTex.vert", _shaderDir + "PerPixCookTorranceTex.frag")});
            break;
        case SP_bumpNormal:
            _programs.insert({id, new SLGLGenericProgram(nullptr, _shaderDir + "BumpNormal.vert", _shaderDir + "BumpNormal.frag")});
            break;
        case SP_bumpNormalParallax:
            _programs.insert({id, new SLGLGenericProgram(nullptr, _shaderDir + "BumpNormal.vert", _shaderDir + "BumpNormalParallax.frag")});
            break;
        case SP_fontTex:
            _programs.insert({id, new SLGLGenericProgram(nullptr, _shaderDir + "FontTex.vert", _shaderDir + "FontTex.frag")});
            break;
        case SP_stereoOculus:
            _programs.insert({id, new SLGLGenericProgram(nullptr, _shaderDir + "StereoOculus.vert", _shaderDir + "StereoOculus.frag")});
            break;
        case SP_stereoOculusDistortion:
            _programs.insert({id, new SLGLGenericProgram(nullptr, _shaderDir + "StereoOculusDistortionMesh.vert", _shaderDir + "StereoOculusDistortionMesh.frag")});
            break;
        case SP_errorTex:
            _programs.insert({id, new SLGLGenericProgram(nullptr, _shaderDir + "ErrorTex.vert", _shaderDir + "ErrorTex.frag")});
            break;
        default:
            SL_EXIT_MSG("SLGLProgramManager: unknown shader id!");
    }
}
//-----------------------------------------------------------------------------
