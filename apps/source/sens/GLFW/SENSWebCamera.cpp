#include "SENSWebCamera.h"
#include <SENSException.h>
#include <SENSUtils.h>
#include <Utils.h>

#define LOG_WEBCAM_WARN(...) Utils::log("SENSWebCamera", __VA_ARGS__);
#define LOG_WEBCAM_INFO(...) Utils::log("SENSWebCamera", __VA_ARGS__);
#define LOG_WEBCAM_DEBUG(...) Utils::log("SENSWebCamera", __VA_ARGS__);

const SENSCameraConfig& SENSWebCamera::start(std::string                   deviceId,
                                             const SENSCameraStreamConfig& streamConfig,
                                             cv::Size                      imgRGBSize,
                                             bool                          mirrorV,
                                             bool                          mirrorH,
                                             bool                          convToGrayToImgManip,
                                             int                           imgManipWidth,
                                             bool                          provideIntrinsics,
                                             float                         fovDegFallbackGuess)
{
    if (_started)
    {
        Utils::warnMsg("SENSWebCamera", "Call to start was ignored. Camera is currently running!", __LINE__, __FILE__);
        return _config;
    }

    cv::Size targetSize;
    if (imgRGBSize.width > 0 && imgRGBSize.height > 0)
    {
        targetSize.width  = imgRGBSize.width;
        targetSize.height = imgRGBSize.height;
    }
    else
    {
        targetSize.width  = streamConfig.widthPix;
        targetSize.height = streamConfig.heightPix;
    }

    cv::Size imgManipSize(imgManipWidth,
                          (int)((float)imgManipWidth * (float)targetSize.height / (float)targetSize.width));

    //retrieve all camera characteristics
    if (_captureProperties.size() == 0)
        captureProperties();

    if (_captureProperties.size() == 0)
        throw SENSException(SENSType::CAM, "Could not retrieve camera properties!", __LINE__, __FILE__);

    if (!_captureProperties.containsDeviceId(deviceId))
        throw SENSException(SENSType::CAM, "DeviceId does not exist!", __LINE__, __FILE__);

    int id = std::stoi(deviceId);
    _videoCapture.open(id);

    if (!_videoCapture.isOpened())
        throw SENSException(SENSType::CAM, "Could not open camera with id: " + deviceId, __LINE__, __FILE__);

    _videoCapture.set(cv::CAP_PROP_FRAME_WIDTH, targetSize.width);
    _videoCapture.set(cv::CAP_PROP_FRAME_HEIGHT, targetSize.height);
    _started           = true;
    _permissionGranted = true;

    //init config here
    _config = {deviceId,
               &streamConfig,
               SENSCameraFocusMode::UNKNOWN,
               targetSize.width,
               targetSize.height,
               imgManipSize.width,
               imgManipSize.height,
               mirrorH,
               mirrorV,
               convToGrayToImgManip};

    initCalibration(fovDegFallbackGuess);

    return _config;
}

void SENSWebCamera::stop()
{
    if (_videoCapture.isOpened())
    {
        _videoCapture.release();

        _started = false;
    }
    else
    {
        LOG_WEBCAM_INFO("stop: ignored because camera is not open!");
    }
}

SENSFramePtr SENSWebCamera::latestFrame()
{
    SENSFramePtr sensFrame;

    if (!_started)
    {
        LOG_WEBCAM_WARN("getLatestFrame: Camera is not started!");
        return sensFrame;
    }

    if (!_videoCapture.isOpened())
        throw SENSException(SENSType::CAM, "Capture device is not open although camera is started!", __LINE__, __FILE__);

    cv::Mat rgbImg;
    if (_videoCapture.read(rgbImg))
    {
        sensFrame = postProcessNewFrame(rgbImg, cv::Mat(), false);
    }
    return sensFrame;
}

const SENSCaptureProperties& SENSWebCamera::captureProperties()
{
    if (!_captureProperties.size())
    {
        //definition of standard frame sizes that we want to test for support
        static std::vector<cv::Size> testSizes = {
          {640, 360},
          {640, 480},
          {960, 540},
          {1280, 960},
          {1280, 720},
          {1920, 1080}};

        //There is an invisible list of devices populated from your os and your webcams appear there in the order you plugged them in.
        //If you're e.g on a laptop with a builtin camera, that will be id 0, if you plug in an additional one, that's id 1.
        for (int i = 0; i < 10; ++i)
        {
            _videoCapture.open(i);

            if (_videoCapture.isOpened())
            {
                SENSCameraDeviceProperties characteristics(std::to_string(i), SENSCameraFacing::UNKNOWN);

                //try some standard capture sizes
                for (auto s : testSizes)
                {
                    _videoCapture.set(cv::CAP_PROP_FRAME_WIDTH, s.width);
                    _videoCapture.set(cv::CAP_PROP_FRAME_HEIGHT, s.height);
                    cv::Mat frame;
                    _videoCapture >> frame;
                    cv::Size newSize = frame.size();
                    if (!characteristics.contains(newSize) &&
                        newSize != cv::Size(0, 0))
                    {
                        //-1 means unknown focal length
                        characteristics.add(newSize.width, newSize.height, -1.f);
                    }
                }

                _captureProperties.push_back(characteristics);
                _videoCapture.release();
            }
        }
    }

    //if still no caputure properties add a dummy
    if (_captureProperties.size() == 0)
    {
        SENSCameraDeviceProperties dummyProps("0", SENSCameraFacing::UNKNOWN);
        dummyProps.add(640, 480, -1.f);
        _captureProperties.push_back(dummyProps);
    }

    return _captureProperties;
}
