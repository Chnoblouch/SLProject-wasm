//#############################################################################
//  File:      SLImGuiInfosMapTransform.h
//  Date:      April 2018
//  Codestyle: https://github.com/cpvrlab/SLProject/wiki/SLProject-Coding-Style
//  Authors:   Michael Goettlicher, Marcus Hudritsch
//  License:   This software is provided under the GNU General Public License
//             Please visit: http://opensource.org/licenses/GPL-3.0
//#############################################################################

#ifndef SL_IMGUI_INFOSMAPTRANSFORM_H
#define SL_IMGUI_INFOSMAPTRANSFORM_H

#include <string>
#include <SLImGuiInfosDialog.h>

//interface
class SLTrackingInfosInterface;
class SLCVMapTracking;
class SLCVMap;

//-----------------------------------------------------------------------------
class SLImGuiInfosMapTransform : public SLImGuiInfosDialog
{
public:
    SLImGuiInfosMapTransform(string name, SLCVMapTracking* tracking);

    void buildInfos() override;

private:
    void stopTracking();
    void applyTransformation(float                  transformationValue,
                             SLCVMap::TransformType type);

    SLCVMapTracking* _tracking = nullptr;
    SLCVMap*         _map      = nullptr;

    float _transformationRotValue = 10.0f;
    ;
    float _transformationTransValue = 0.1f;
    float _transformationScaleValue = 1.1f;
};

#endif //SL_IMGUI_INFOSMAPTRANSFORM_H
