//#############################################################################
//  File:      SLDemoGui.h
//  Author:    Marcus Hudritsch
//  Date:      Summer 2017
//  Codestyle: https://github.com/cpvrlab/SLProject/wiki/Coding-Style-Guidelines
//  Copyright: Marcus Hudritsch
//             This software is provide under the GNU General Public License
//             Please visit: http://opensource.org/licenses/GPL-3.0
//#############################################################################

#ifndef SLGUIDEMO_H
#define SLGUIDEMO_H

#include <stdafx.h>
class SLScene;
class SLSceneView;
class SLNode;

//-----------------------------------------------------------------------------
//! ImGui UI class for the UI of the demo applications
/* The ImGui is completely build within this class by calling build function
SLDemoGui::buildDemoGui. This build function is passed in the
SLInterface::slCreateSceneView and it is called in SLSceneView::onPaint in
every frame. The entire UI is configured and built in every frame. That is why
it is called "Im" for immediate. See also the SLGLImGui class so see how it
minimaly integrated in the SLProject.
*/
class SLDemoGui
{
    public:
    static void         buildDemoGui        (SLScene* s, SLSceneView* sv);

    static void         buildMenuBar        (SLScene* s, SLSceneView* sv);
    static void         buildSceneGraph     (SLScene* s);
    static void         addSceneGraphNode   (SLScene* s, SLNode* node);
    static void         buildProperties     (SLScene* s);
    static void         loadConfig          ();
    static void         saveConfig          ();

    static SLstring     configTime;         //!< Time of stored configuration
    static SLstring     infoAbout;          //!< About info string
    static SLstring     infoCredits;        //!< Credits info string
    static SLstring     infoHelp;           //!< Help info string
    static SLstring     infoCalibrate;      //!< Calibration info string
    static SLbool       showMenu;           //!< Flag if menu bar should be shown
    static SLbool       showAbout;          //!< Flag if about info should be shown
    static SLbool       showHelp;           //!< Flag if help info should be shown
    static SLbool       showHelpCalibration;//!< Flag if calibration info should be shown
    static SLbool       showCredits;        //!< Flag if credits info should be shown
    static SLbool       showStatsTiming;    //!< Flag if timing info should be shown
    static SLbool       showStatsScene;     //!< Flag if scene info should be shown
    static SLbool       showStatsVideo;     //!< Flag if video info should be shown
    static SLbool       showInfosFrameworks;//!< Flag if frameworks info should be shown
    static SLbool       showInfosScene;     //!< Flag if scene info should be shown
    static SLbool       showSceneGraph;     //!< Flag if scene graph should be shown
    static SLbool       showProperties;     //!< Flag if properties should be shown
};
//-----------------------------------------------------------------------------
#endif
