//#############################################################################
//  File:      SLShadowMap.h
//  Author:    Michael Schertenleib
//  Date:      May 2020
//  Codestyle: https://github.com/cpvrlab/SLProject/wiki/SLProject-Coding-Style
//  Copyright: Michael Schertenleib
//             This software is provide under the GNU General Public License
//             Please visit: http://opensource.org/licenses/GPL-3.0
//#############################################################################

#ifndef SLSHADOWMAP_H
#define SLSHADOWMAP_H

#include <SL.h>
#include <SLEnums.h>
#include <SLPlane.h>
#include <SLMat4.h>

class SLGLDepthBuffer;
class SLGLVertexArrayExt;
class SLLight;
class SLLightDirect;
class SLLightSpot;
class SLMaterial;
class SLMaterial;
class SLNode;
class SLSceneView;
class SLCamera;
//-----------------------------------------------------------------------------
//! Class for shadow mapping
/*!
Shadow mapping is a technique to render shadows. The scene gets rendered from
the point of view of the lights which cast shadows. The resulting depth-map of
that render-pass can be used to determine which fragments are affected by which
lights.
*/
class SLShadowMap
{
public:
    SLShadowMap(SLProjection   projection,
                SLLight*       light,
                float          clipNear = 0.1f,
                float          clipFar  = 20.0f,
                const SLVec2f& size     = SLVec2f(8, 8),
                const SLVec2i& texSize  = SLVec2i(1024, 1024));

    SLShadowMap(SLProjection   projection,
                SLLight*       light,
                SLCamera*      camera,
                const SLVec2f& size       = SLVec2f(8, 8),
                const SLVec2i& texSize    = SLVec2i(1024, 1024),
                int            nbCascades = 4);

    ~SLShadowMap();

    // Setters
    void useCubemap(SLbool useCubemap) { _useCubemap = useCubemap; }
    void rayCount(const SLVec2i& rayCount) { _rayCount.set(rayCount); }
    void clipNear(SLfloat clipNear) { _clipNear = clipNear; }
    void clipFar(SLfloat clipFar) { _clipFar = clipFar; }
    void size(const SLVec2f& size)
    {
        _size.set(size);
        _halfSize.set(size / 2);
    }
    void textureSize(const SLVec2i& textureSize) { _textureSize.set(textureSize); }
    void nbCascades(int nbCascades) { _nbCascades = nbCascades; }

    // Getters
    SLProjection                  projection() { return _projection; }
    SLbool                        useCubemap() const { return _useCubemap; }
    SLbool                        useCascaded() const { return _useCascaded; }
    SLMat4f*                      mvp() { return _mvp; }
    SLGLDepthBuffer*              depthBuffer() { return _depthBuffers.at(0); }
    std::vector<SLGLDepthBuffer*> depthBuffers() { return _depthBuffers; }
    SLVec2i                       rayCount() { return _rayCount; }
    SLfloat                       clipNear();
    SLfloat                       clipFar();
    SLVec2f                       size() { return _size; }
    SLVec2i                       textureSize() { return _textureSize; }
    int                           nbCascades() { return _nbCascades; }

    // Other methods
    void drawFrustum();
    void drawRays();
    void updateMVP();
    void render(SLSceneView* sv, SLNode* root);
    void renderDirectionalLightCascaded(SLSceneView* sv, SLNode* root);

private:
    SLLight*                      _light;        //!< The light which uses this shadow map
    SLProjection                  _projection;   //!< Projection to use to create shadow map
    SLbool                        _useCubemap;   //!< Flag if cubemap should be used for perspective projections
    SLbool                        _useCascaded;  //!< Flag if cubemap should be used for perspective projections
    SLint                         _nbCascades;   //!< Number of cascades
    SLMat4f                       _v[6];         //!< View matrices
    SLMat4f                       _p[6];         //!< Projection matrix
    SLMat4f                       _mvp[6];       //!< Model-view-projection matrices
    std::vector<SLGLDepthBuffer*> _depthBuffers; //!< Framebuffer and texture
    SLGLVertexArrayExt*           _frustumVAO;   //!< Visualization of light-space-frustum
    SLVec2i                       _rayCount;     //!< Amount of rays drawn by drawRays()
    SLMaterial*                   _mat;          //!< Material used to render the shadow map
    SLfloat                       _clipNear;     //!< Near clipping plane
    SLfloat                       _clipFar;      //!< Far clipping plane
    SLVec2f                       _size;         //!< Height and width of the frustum (only for SLLightDirect)
    SLVec2f                       _halfSize;     //!< _size divided by two
    SLVec2i                       _textureSize;  //!< Size of the shadow map texture
    SLCamera*                     _camera;


    std::vector<SLVec2f> getShadowMapCascades(int nbCascades, float n, float f);
    void drawNodesIntoDepthBufferCulling(SLNode* node, SLSceneView* sv, SLMat4f& p, SLMat4f& v, SLPlane *planes);
    void drawNodesIntoDepthBuffer(SLNode* node, SLSceneView* sv, SLMat4f& p, SLMat4f& v);
    void findOptimalNearPlane(SLNode* node, SLSceneView* sv, SLMat4f& P, SLMat4f& lv, SLPlane* planes, std::vector<SLNode*>& visibleNodes);
    void drawNodesDirectionalCulling(std::vector<SLNode*> visibleNodes, SLSceneView* sv, SLMat4f& P, SLMat4f& lv, SLPlane* planes);
    void drawNodesDirectional(SLNode* node, SLSceneView* sv, SLMat4f& P, SLMat4f& lv);
};
//-----------------------------------------------------------------------------
#endif //SLSHADOWMAP_H
