//#############################################################################
//  File:      AppDemoGuiInfosMapNodeTransform.h
//  Author:    Michael Goettlicher, Jan Dellsperger
//  Date:      September 2018
//  Codestyle: https://github.com/cpvrlab/SLProject/wiki/Coding-Style-Guidelines
//  Copyright: Marcus Hudritsch
//             This software is provide under the GNU General Public License
//             Please visit: http://opensource.org/licenses/GPL-3.0
//#############################################################################

#ifndef APP_DEMO_GUI_MAP_POINT_EDITOR_H
#define APP_DEMO_GUI_MAP_POINT_EDITOR_H

#include <AppDemoGuiInfosDialog.h>
#include <string>

class SLScene;
class SLSceneView;
struct WAIEvent;
//-----------------------------------------------------------------------------
class AppDemoGuiMapPointEditor : public AppDemoGuiInfosDialog
{
public:
    AppDemoGuiMapPointEditor(std::string            name,
                             bool*                  activator,
                             std::queue<WAIEvent*>* eventQueue,
                             ImFont*                font,
                             std::string            slamRootDir);

    void loadFileNamesInVector(std::string               directory,
                               std::vector<std::string>& fileNames,
                               std::vector<std::string>& extensions,
                               bool                      addEmpty = true);

    void buildInfos(SLScene* s, SLSceneView* sv) override;

private:
    std::queue<WAIEvent*>* _eventQueue;
    bool _showMatchFinder = false;
    bool _showSelectionChoice = false;
    std::string _currMatchedFile = "";
    std::string _slamRootDir;
    int _nbVideoInMap;
    int _videoId;
    std::vector<int> _kFVidMatching;
};

#endif
