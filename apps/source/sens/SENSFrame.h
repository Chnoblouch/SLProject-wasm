#ifndef SENS_FRAME_H
#define SENS_FRAME_H

#include <opencv2/core.hpp>
#include <sens/SENS.h>

//Camera frame obeject
struct SENSFrameBase
{
    SENSFrameBase(SENSTimePt timePt, cv::Mat imgBGR, cv::Mat intrinsics)
      : imgBGR(imgBGR),
        intrinsics(intrinsics),
        timePt(timePt)
    {
    }
    
    SENSFrameBase(SENSTimePt timePt, cv::Mat imgBGR, cv::Mat intrinsics, cv::Mat camPose)
      : imgBGR(imgBGR),
        intrinsics(intrinsics),
        timePt(timePt),
        pose(camPose)
    {
    }
    
    //camera pose
    cv::Mat pose;
    
    //! cropped input image
    cv::Mat imgBGR;
    cv::Mat intrinsics;
    
    const SENSTimePt timePt;
};

typedef std::shared_ptr<SENSFrameBase> SENSFrameBasePtr;

struct SENSFrame
{
    SENSFrame(const SENSTimePt& timePt,
              cv::Mat           imgBGR,
              cv::Mat           imgManip,
              bool              mirroredH,
              bool              mirroredV,
              float             scaleToManip,
              cv::Mat           intrinsics)
      : timePt(timePt),
        imgBGR(imgBGR),
        imgManip(imgManip),
        mirroredH(mirroredH),
        mirroredV(mirroredV),
        scaleToManip(scaleToManip),
        intrinsics(intrinsics)
    {
    }

    const SENSTimePt timePt;
    
    //! original image (maybe cropped and scaled)
    cv::Mat imgBGR;
    //! scaled and maybe gray manipulated image
    cv::Mat imgManip;

    //const int  captureWidth;
    //const int  captureHeight;
    //const int  cropW;
    //const int  cropH;
    const bool mirroredH;
    const bool mirroredV;
    //! scale between imgManip and imgBGR
    const float scaleToManip;

    cv::Mat intrinsics;
};
typedef std::shared_ptr<SENSFrame> SENSFramePtr;

#endif //SENS_FRAME_H
