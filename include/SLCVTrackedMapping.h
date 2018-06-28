//#############################################################################
//  File:      SLCVTrackedMapping.h
//  Author:    Michael Goettlicher
//  Date:      March 2018
//  Codestyle: https://github.com/cpvrlab/SLProject/wiki/Coding-Style-Guidelines
//  Copyright: Marcus Hudritsch, Michael Goettlicher
//             This software is provide under the GNU General Public License
//             Please visit: http://opensource.org/licenses/GPL-3.0
//#############################################################################

#ifndef SLCVTrackedMapping_H
#define SLCVTrackedMapping_H

/*
The OpenCV library version 3.1 with extra module must be present.
If the application captures the live video stream with OpenCV you have
to define in addition the constant SL_USES_CVCAPTURE.
All classes that use OpenCV begin with SLCV.
See also the class docs for SLCVCapture, SLCVCalibration and SLCVTracked
for a good top down information.
*/

#include <SLCV.h>
#include <SLCVTracked.h>
#include <SLNode.h>
#include <SLCVFrame.h>
#include <SLCVMap.h>
#include <SLCVMapTracking.h>

namespace ORB_SLAM2 {
    class Initializer;
    class LocalMapping;
}

class SLCVKeyFrameDB;

//-----------------------------------------------------------------------------
class SLCVTrackedMapping : public SLCVTracked, public SLCVMapTracking
{
    public:

        int getNumLoopClosings() { return _numOfLoopClosings; }

        //enum TrackingStates { IDLE, INITIALIZE, TRACK_VO, TRACK_3DPTS, TRACK_OPTICAL_FLOW };

                SLCVTrackedMapping    (SLNode* node, SLCVMapNode* mapNode=NULL );
                ~SLCVTrackedMapping();

        SLbool  track               (SLCVMat imageGray,
                                     SLCVMat imageRgb,
                                     SLCVCalibration* calib,
                                     SLbool drawDetection,
                                     SLSceneView* sv);

        void Reset() override;

        //ghm1: the next tracked frame gets mapped (local mapping, keyframe generation and adding to map)
        void mapNextFrame() { _mapNextFrame = true; }

        void globalBundleAdjustment();

        size_t getSizeOf();
    private:
        // Map initialization for monocular
        void CreateInitialMapMonocular();
        void CreateNewKeyFrame();

        //! initialization routine
        void initialize();
        void trackVO();
        void track3DPts();
        void trackOpticalFlow();

        void decorate();
        bool Relocalization();
        bool TrackReferenceKeyFrame();
        bool TrackLocalMap();
        void UpdateLocalMap();
        void SearchLocalPoints();
        void UpdateLocalKeyFrames();
        void UpdateLocalPoints();
        bool TrackWithMotionModel();
        void UpdateLastFrame();


        //Motion Model
        cv::Mat mVelocity;

        //Last Frame, KeyFrame and Relocalisation Info
        unsigned int mnLastRelocFrameId = 0;

        //! states, that we try to make a new key frame out of the next frame
        bool _addKeyframe;

        // ORB vocabulary used for place recognition and feature matching.
        ORBVocabulary* mpVocabulary;

        // Initialization Variables (Monocular)
        //std::vector<int> mvIniLastMatches;
        std::vector<int> mvIniMatches;
        std::vector<cv::Point2f> mvbPrevMatched;
        std::vector<cv::Point3f> mvIniP3D;
        SLCVFrame mInitialFrame;

        //extractor instance
        ORB_SLAM2::ORBextractor* _extractor = NULL;
        ORBextractor* mpIniORBextractor = NULL;

        // Initalization (only for monocular)
        Initializer* mpInitializer = NULL;

        SLCVMat _img;

        // In case of performing only localization, this flag is true when there are no matches to
        // points in the map. Still tracking will continue if there are enough matches with temporal points.
        // In that case we are doing visual odometry. The system will try to do relocalization to recover
        // "zero-drift" localization to the map.
        bool mbVO = false;

        LocalMapping* mpLocalMapper = NULL;
        LoopClosing* mpLoopClosing = NULL;

        //New KeyFrame rules (according to fps)
        // Max/Min Frames to insert keyframes and to check relocalisation
        int mMinFrames = 0;
        int mMaxFrames = 30; //= fps

        // Lists used to recover the full camera trajectory at the end of the execution.
        // Basically we store the reference keyframe for each frame and its relative transformation
        list<cv::Mat> mlRelativeFramePoses;
        list<SLCVKeyFrame*> mlpReferences;
        list<double> mlFrameTimes;
        list<bool> mlbLost;

        //Last Frame, KeyFrame and Relocalisation Info
        SLCVKeyFrame* mpLastKeyFrame=NULL;
        //SLCVFrame mLastFrame;
        unsigned int mnLastKeyFrameId;
        //unsigned int mnLastRelocFrameId;

        //render flags
        bool _drawMapPoints = true; 
        bool _drawMapPointsMatches = true;
        bool _drawKeyFrames = true;

        bool _mapNextFrame = false;

        int _numOfLoopClosings = 0;

        bool _retainImg = true;
};
//-----------------------------------------------------------------------------
#endif // SLCVTrackedMapping_H
