//#############################################################################
//  File:      SLImGuiInfosMapNodeTransform.cpp
//  Date:      September 2018
//  Codestyle: https://github.com/cpvrlab/SLProject/wiki/SLProject-Coding-Style
//  Authors:   Michael Goettlicher, Jan Dellsperger, Marcus Hudritsch
//  License:   This software is provided under the GNU General Public License
//             Please visit: http://opensource.org/licenses/GPL-3.0
//#############################################################################

#include <imgui.h>
#include <imgui_internal.h>

#include <SLCVMapNode.h>
#include <SLCVMapStorage.h>
#include <SLCVMapTracking.h>
#include <SLImGuiInfosMapNodeTransform.h>

//-----------------------------------------------------------------------------
SLImGuiInfosMapNodeTransform::SLImGuiInfosMapNodeTransform(
  string           name,
  SLCVMapNode*     mapNode,
  SLCVMapTracking* tracking)
  : SLImGuiInfosDialog(name),
    _mapNode(mapNode),
    _tracking(tracking)
{
}

//-----------------------------------------------------------------------------
void SLImGuiInfosMapNodeTransform::buildInfos()
{
    static SLTransformSpace tSpace = TS_world;
    ImGui::Text("Transf. Space:");
    ImGui::SameLine();
    if (ImGui::RadioButton("Object", (int*)&tSpace, 0)) tSpace = TS_object;
    ImGui::SameLine();
    if (ImGui::RadioButton("World", (int*)&tSpace, 1)) tSpace = TS_world;
    ImGui::SameLine();
    if (ImGui::RadioButton("Parent", (int*)&tSpace, 2)) tSpace = TS_parent;
    ImGui::Separator();

    static SLfloat sp = 3; //spacing
    SLfloat        bW = (ImGui::GetContentRegionAvailWidth() - 2 * sp) / 3;

    //rotation
    if (ImGui::ButtonEx("--", ImVec2(0, 0), ImGuiButtonFlags_Repeat | ImGuiButtonFlags_PressedOnClick))
        _transformationRotValue -= 10.0f;
    ImGui::SameLine();
    if (ImGui::ButtonEx("++", ImVec2(0, 0), ImGuiButtonFlags_Repeat | ImGuiButtonFlags_PressedOnClick))
        _transformationRotValue += 10.0f;
    ImGui::SameLine();
    ImGui::InputFloat("Rot. Value", &_transformationRotValue, 0.1f, 10.f, 3);
    _transformationRotValue = ImClamp(_transformationRotValue, -360.0f, 360.0f);

    if (ImGui::Button("RotX", ImVec2(bW, 0.0f)))
    {
        _mapNode->rotate(_transformationRotValue, 1, 0, 0, tSpace);
    }
    ImGui::SameLine(0.0, sp);
    if (ImGui::Button("RotY", ImVec2(bW, 0.0f)))
    {
        _mapNode->rotate(_transformationRotValue, 0, 1, 0, tSpace);
    }
    ImGui::SameLine(0.0, sp);
    if (ImGui::Button("RotZ", ImVec2(bW, 0.0f)))
    {
        _mapNode->rotate(_transformationRotValue, 0, 0, 1, tSpace);
    }
    ImGui::Separator();

    //translation
    if (ImGui::ButtonEx("--", ImVec2(0, 0), ImGuiButtonFlags_Repeat | ImGuiButtonFlags_PressedOnClick))
        _transformationTransValue -= 1.0f;
    ImGui::SameLine();
    if (ImGui::ButtonEx("++", ImVec2(0, 0), ImGuiButtonFlags_Repeat | ImGuiButtonFlags_PressedOnClick))
        _transformationTransValue += 1.0f;
    ImGui::SameLine();
    ImGui::InputFloat("Transl. Value", &_transformationTransValue, 0.01f, 0.1f, 3);

    if (ImGui::Button("TransX", ImVec2(bW, 0.0f)))
    {
        _mapNode->translate(_transformationTransValue, 0, 0, tSpace);
    }
    ImGui::SameLine(0.0, sp);
    if (ImGui::Button("TransY", ImVec2(bW, 0.0f)))
    {
        _mapNode->translate(0, _transformationTransValue, 0, tSpace);
    }
    ImGui::SameLine(0.0, sp);
    if (ImGui::Button("TransZ", ImVec2(bW, 0.0f)))
    {
        _mapNode->translate(0, 0, _transformationTransValue, tSpace);
    }
    ImGui::Separator();

    //scale
    if (ImGui::ButtonEx("--", ImVec2(0, 0), ImGuiButtonFlags_Repeat | ImGuiButtonFlags_PressedOnClick))
        _transformationScaleValue -= 0.1f;
    ImGui::SameLine();
    if (ImGui::ButtonEx("++", ImVec2(0, 0), ImGuiButtonFlags_Repeat | ImGuiButtonFlags_PressedOnClick))
        _transformationScaleValue += 0.1f;
    ImGui::SameLine();
    ImGui::InputFloat("Scale Value", &_transformationScaleValue, 0.01f, 1.f, 3);
    _transformationScaleValue = ImClamp(_transformationScaleValue, 0.0f, 1000.0f);
    if (ImGui::Button("Scale", ImVec2(ImGui::GetContentRegionAvailWidth(), 0.0f)))
    {
        _mapNode->scale(_transformationScaleValue);
    }
    ImGui::Separator();

    if (ImGui::Button("Save State", ImVec2(ImGui::GetContentRegionAvailWidth(), 0.0f)))
    {
        SLCVMapStorage::saveMap(SLCVMapStorage::getCurrentId(), _tracking, true);
    }
}
