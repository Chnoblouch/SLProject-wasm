#include "SimHelperGui.h"
#include <sens/SENSSimHelper.h>

void SimHelperGui::render(SENSSimHelper* simHelper)
{
    if (!simHelper)
    {
        reset();
        return;
    }

    float framePadding = 0.02f * _screenH;
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(framePadding, framePadding));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(framePadding, framePadding));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(framePadding, framePadding));
    ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 2.f * framePadding + _fontHeading->FontSize);

    ImGui::PushFont(_fontHeading);

    ImGui::Begin(_title.c_str());

    //pop heading font
    ImGui::PopFont();
    ImGui::PushFont(_fontText);

    ImGui::BeginChild("##scrollingRenderSimInfos", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

    float w = ImGui::GetContentRegionAvailWidth();
    //float btnW = w * 0.5f - ImGui::GetStyle().ItemSpacing.x;
    float btnW = w;
    //recording
    {
        ImGui::Text("Sensor Recording");

        if (simHelper->gps()) //if there is a valid sensor we can record
        {
            if (ImGui::Checkbox("gps##record", &simHelper->recordGps))
            {
                simHelper->updateGpsRecording();
            }
        }
        ImGui::SameLine();

        if (simHelper->orientation()) //if there is a valid sensor we can record
        {
            if (ImGui::Checkbox("orientation##record", &simHelper->recordOrientation))
            {
                simHelper->updateOrientationRecording();
            }
        }
        ImGui::SameLine();

        if (simHelper->camera()) //if there is a valid sensor we can record
        {
            if (ImGui::Checkbox("camera##record", &simHelper->recordCamera))
            {
                simHelper->updateCameraRecording();
            }
        }

        if (simHelper->recorderIsRunning())
        {
            if (ImGui::Button("Stop recording##RecordBtn", ImVec2(btnW, 0)))
                simHelper->stopRecording();
        }
        else
        {
            if (ImGui::Button("Start recording##RecordBtn", ImVec2(btnW, 0)))
                simHelper->startRecording();
        }

        std::vector<std::string> errorMsgs;
        if (simHelper->getRecorderErrors(errorMsgs))
        {
            for (const std::string& msg : errorMsgs)
            {
                ImGui::TextUnformatted(msg.c_str());
            }
        }
    }

    //simulation (only show if recorder is not running because we cannot make changes while recording)
    if (!simHelper->recorderIsRunning())
    {
        ImGui::Separator();
        ImGui::Text("Sensor simulation");

        if (!simHelper->simIsRunning())
        {
            //first select a directory which contains the recorder output
            if (ImGui::BeginCombo("Sim data", _selectedSimData.c_str()))
            {
                std::vector<std::string> simDataStrings = Utils::getDirNamesInDir(simHelper->simDataDir(), false);
                for (int n = 0; n < simDataStrings.size(); n++)
                {
                    bool isSelected = (_selectedSimData == simDataStrings[n]); // You can store your selection however you want, outside or inside your objects
                    if (ImGui::Selectable(simDataStrings[n].c_str(), isSelected))
                    {
                        _selectedSimData = simDataStrings[n];
                        //instantiate simulator with selected data. After that we know, if we can simulate a sensor
                        simHelper->initSimulator(_selectedSimData);
                    }
                    if (isSelected)
                        ImGui::SetItemDefaultFocus(); // Set the initial focus when opening the combo (scrolling + for keyboard navigation support in the upcoming navigation branch)
                }
                ImGui::EndCombo();
            }

            //add checkboxes for simulated sensors if successfully loaded from sim directory
            if (simHelper->canSimGps())
            {
                if (ImGui::Checkbox("gps##sim", &simHelper->simulateGps))
                    simHelper->updateGpsSim();
                ImGui::SameLine();
            }

            if (simHelper->canSimOrientation())
            {
                if (ImGui::Checkbox("orientation##sim", &simHelper->simulateOrientation))
                    simHelper->updateOrientationSim();
                ImGui::SameLine();
            }

            if (simHelper->canSimCamera())
            {
                if (ImGui::Checkbox("camera##sim", &simHelper->simulateCamera))
                    simHelper->updateCameraSim();
            }

            if (ImGui::Button("Start simulation##SimBtn", ImVec2(btnW, 0)))
                simHelper->startSim();
        }
        else
        {
            //current simulation time
            auto simNow = simHelper->simTime();
            ImGui::Text("Sim time: %.0fms", (double)simNow.time_since_epoch().count() / 1000.0);
            auto passedSimTime = simHelper->passedSimTime();
            ImGui::Text("Passed time: %.0fms", (double)passedSimTime.count() / 1000.0);

            if (ImGui::Button("Stop simulation##SimBtn", ImVec2(btnW, 0)))
                simHelper->stopSim();

            //pause
            if (simHelper->isPausedSim())
            {
                if (ImGui::Button("Resume##PauseSim", ImVec2(btnW, 0)))
                    simHelper->resumeSim();
            }
            else
            {
                if (ImGui::Button("Pause##PauseSim", ImVec2(btnW, 0)))
                    simHelper->pauseSim();
            }

            std::vector<std::string> errorMsgs;
            if (simHelper->getSimulatorErrors(errorMsgs))
            {
                for (const std::string& msg : errorMsgs)
                {
                    ImGui::TextUnformatted(msg.c_str());
                }
            }
        }
    }

    ImGui::EndChild();
    ImGui::End();

    ImGui::PopStyleVar(5);
    ImGui::PopFont();
}
