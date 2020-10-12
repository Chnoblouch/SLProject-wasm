#ifndef SENS_FRAME_H
#define SENS_FRAME_H

#include <opencv2/core.hpp>

//Camera frame obeject
struct SENSFrameBase
{
    SENSFrameBase(cv::Mat imgBGR, cv::Mat intrinsics)
      : imgBGR(imgBGR),
        intrinsics(intrinsics)
    {
    }

    //! cropped input image
    cv::Mat imgBGR;
    cv::Mat intrinsics;
};

typedef std::shared_ptr<SENSFrameBase> SENSFrameBasePtr;

struct SENSFrame
{
    SENSFrame(cv::Mat imgBGR,
              cv::Mat imgManip,
              int     captureWidth,
              int     captureHeight,
              int     cropW,
              int     cropH,
              bool    mirroredH,
              bool    mirroredV,
              float   scaleToManip,
              cv::Mat intrinsics)
      : imgBGR(imgBGR),
        imgManip(imgManip),
        captureWidth(captureWidth),
        captureHeight(captureHeight),
        cropW(cropW),
        cropH(cropH),
        mirroredH(mirroredH),
        mirroredV(mirroredV),
        scaleToManip(scaleToManip),
        intrinsics(intrinsics)
    {
    }

    cv::Mat imgBGR;
    //! scaled and maybe gray manipulated image
    cv::Mat imgManip;

    const int  captureWidth;
    const int  captureHeight;
    const int  cropW;
    const int  cropH;
    const bool mirroredH;
    const bool mirroredV;
    //! scale between imgManip and imgBGR
    const float scaleToManip;

    cv::Mat intrinsics;
};
typedef std::shared_ptr<SENSFrame> SENSFramePtr;

#endif //SENS_FRAME_H
