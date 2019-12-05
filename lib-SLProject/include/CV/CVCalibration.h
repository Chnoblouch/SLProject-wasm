//#############################################################################
//  File:      CVCalibration.h
//  Author:    Michael Goettlicher, Marcus Hudritsch
//  Date:      Winter 2016
//  Codestyle: https://github.com/cpvrlab/SLProject/wiki/SLProject-Coding-Style
//  Copyright: Marcus Hudritsch
//             This software is provide under the GNU General Public License
//             Please visit: http://opensource.org/licenses/GPL-3.0
//#############################################################################

#ifndef CVCALIBRATION_H
#define CVCALIBRATION_H

/*
The OpenCV library version 3.4 or above with extra module must be present.
If the application captures the live video stream with OpenCV you have
to define in addition the constant APP_USES_CVCAPTURE.
All classes that use OpenCV begin with CV.
See also the class docs for CVCapture, CVCalibration and CVTracked
for a good top down information.
*/

#include <CVTypedefs.h>
#include <ftplib.h>
#include <CVTypes.h>

using namespace std;

//-----------------------------------------------------------------------------
//! OpenCV Calibration state
enum CVCalibState
{
    CS_uncalibrated, //!< The camera is not calibrated (no calibration found)
    CS_calibrated,   //!< The camera is calibrated
    CS_guessed,      //!< The camera intrinsics where estimated from FOV
};

//-----------------------------------------------------------------------------
//! Live video camera calibration class with OpenCV an OpenCV calibration.
/*! The camera calibration can determine the inner or intrinsic parameters such
as the focal length and the lens distortion and external or extrinsic parameter
such as the camera pose towards a known geometry.
\n
For a good calibration we have to make 15-20 images from a chessboard pattern.
The chessboard pattern can be printed from the CalibrationChessboard_8x5_A4.pdf
in the folder data/calibration. It is important that one side has an odd number
of inner corners. Like this it is unambiguous and can be rotated in any direction.
\n
The different calibration states are handled within AppDemoTracking::onUpdateTracking:
\n
- CS_uncalibrated:     The camera is not calibrated (no calibration found found)
- CS_calibrated:       The camera is calibrated
- CS_guessed
\n
The core of the intrinsic calibration is stored in the members _cameraMat and
_distortion. For the calibration internals see the OpenCV documentation:
http://docs.opencv.org/3.1.0/dc/dbb/tutorial_py_calibration.html
After a successfully calibration the parameters are stored in a config file on
the SL::configPath. If it exists, it is loaded from there at startup.
If doesn't exist a simple calibration from a default field of view angle is
estimated.
\n
The CVCapture instance has two video camera calibrations, one for a main camera
(CVCapture::mainCam) and one for the selfie camera on mobile devices
(CVCapture::scndCam). The member CVCapture::activeCamera points to the active
one and is set by the CVCapture::videoType (VT_NONE, VT_MAIN, VT_SCND) during the
scene assembly in AppDemoLoad. On mobile devices the front camera is the
selfie camera (our secondary) and the back camera is the our main camera.
*/

class CVCalibration
{
public:
    //default constructor with uncalibrated state (this is not good because
    //it is not a valid state so everybody who uses it has to check the calibration state first)
    CVCalibration(CVCameraType camType);
    //creates a fully defined calibration
    CVCalibration(cv::Mat            cameraMat,
                  cv::Mat            distortion,
                  cv::Size           imageSize,
                  cv::Size           boardSize,
                  float              boardSquareMM,
                  float              reprojectionError,
                  int                numCaptured,
                  const std::string& calibrationTime,
                  int                camSizeIndex,
                  bool               mirroredH,
                  bool               mirroredV,
                  CVCameraType       camType);
    //creates a guessed calibration using image size and fov angle
    CVCalibration(cv::Size     imageSize,
                  float        fovH,
                  bool         mirroredH,
                  bool         mirroredV,
                  CVCameraType type);
    //create a guessed calibration using sensor size, camera focal length and captured image size
    CVCalibration(float        sensorWMM,
                  float        sensorHMM,
                  float        focalLengthMM,
                  cv::Size     imageSize,
                  bool         mirroredH,
                  bool         mirroredV,
                  CVCameraType camType);

    bool load(const string& calibDir,
              const string& calibFileName);
    void save(const string& calibDir,
              const string& calibFileName);

    void   uploadCalibration(const string& fullPathAndFilename);
    void   downloadCalibration(const string& fullPathAndFilename);
    string getLatestCalibFilename(ftplib& ftp, const string& calibFileWOExt);
    int    getVersionInCalibFilename(const string& calibFilename);

    void remap(CVMat& inDistorted,
               CVMat& outUndistorted);

    //! Adapts an already calibrated camera to a new resolution (cropping and scaling)
    void adaptForNewResolution(const CVSize& newSize);

    // Getters
    CVSize imageSize() { return _imageSize; }
    //int    camSizeIndex() { return _camSizeIndex;}
    float  imageAspectRatio() { return (float)_imageSize.width / (float)_imageSize.height; }
    CVMat& cameraMat() { return _cameraMat; }
    CVMat& cameraMatUndistorted() { return _cameraMatUndistorted; }
    CVMat& distortion() { return _distortion; }
    float  cameraFovVDeg() { return _cameraFovVDeg; }
    float  cameraFovHDeg() { return _cameraFovHDeg; }

    int  calibrationFlags() { return _calibFlags; }
    bool calibFixPrincipalPoint() { return _calibFlags & cv::CALIB_FIX_PRINCIPAL_POINT; }
    bool calibFixAspectRatio() { return _calibFlags & cv::CALIB_FIX_ASPECT_RATIO; }
    bool calibZeroTangentDist() { return _calibFlags & cv::CALIB_ZERO_TANGENT_DIST; }
    bool calibRationalModel() { return _calibFlags & cv::CALIB_RATIONAL_MODEL; }
    bool calibTiltedModel() { return _calibFlags & cv::CALIB_TILTED_MODEL; }
    bool calibThinPrismModel() { return _calibFlags & cv::CALIB_THIN_PRISM_MODEL; }
    bool isMirroredH() { return _isMirroredH; }
    bool isMirroredV() { return _isMirroredV; }

    float fx() { return _cameraMat.cols == 3 && _cameraMat.rows == 3 ? (float)_cameraMat.at<double>(0, 0) : 0.0f; }
    float fy() { return _cameraMat.cols == 3 && _cameraMat.rows == 3 ? (float)_cameraMat.at<double>(1, 1) : 0.0f; }
    float cx() { return _cameraMat.cols == 3 && _cameraMat.rows == 3 ? (float)_cameraMat.at<double>(0, 2) : 0.0f; }
    float cy() { return _cameraMat.cols == 3 && _cameraMat.rows == 3 ? (float)_cameraMat.at<double>(1, 2) : 0.0f; }
    float k1() { return _distortion.rows >= 4 ? (float)_distortion.at<double>(0, 0) : 0.0f; }
    float k2() { return _distortion.rows >= 4 ? (float)_distortion.at<double>(1, 0) : 0.0f; }
    float p1() { return _distortion.rows >= 4 ? (float)_distortion.at<double>(2, 0) : 0.0f; }
    float p2() { return _distortion.rows >= 4 ? (float)_distortion.at<double>(3, 0) : 0.0f; }
    float k3() { return _distortion.rows >= 5 ? (float)_distortion.at<double>(4, 0) : 0.0f; }
    float k4() { return _distortion.rows >= 6 ? (float)_distortion.at<double>(5, 0) : 0.0f; }
    float k5() { return _distortion.rows >= 7 ? (float)_distortion.at<double>(6, 0) : 0.0f; }
    float k6() { return _distortion.rows >= 8 ? (float)_distortion.at<double>(7, 0) : 0.0f; }
    float s1() { return _distortion.rows >= 9 ? (float)_distortion.at<double>(8, 0) : 0.0f; }
    float s2() { return _distortion.rows >= 10 ? (float)_distortion.at<double>(9, 0) : 0.0f; }
    float s3() { return _distortion.rows >= 11 ? (float)_distortion.at<double>(10, 0) : 0.0f; }
    float s4() { return _distortion.rows >= 12 ? (float)_distortion.at<double>(11, 0) : 0.0f; }
    float tauX() { return _distortion.rows >= 13 ? (float)_distortion.at<double>(12, 0) : 0.0f; }
    float tauY() { return _distortion.rows >= 14 ? (float)_distortion.at<double>(13, 0) : 0.0f; }

    CVCameraType camType() { return _camType; }
    CVCalibState state() { return _state; }
    //int          numImgsToCapture() { return _numOfImgsToCapture; }
    int    numCapturedImgs() { return _numCaptured; }
    float  reprojectionError() { return _reprojectionError; }
    CVSize boardSize() { return _boardSize; }
    float  boardSquareMM() { return _boardSquareMM; }
    float  boardSquareM() { return _boardSquareMM * 0.001f; }
    string calibrationTime() { return _calibrationTime; }
    string calibDir() { return _calibDir; }
    string calibFileName() { return _calibFileName; }
    string computerInfos() { return _computerInfos; }
    string stateStr()
    {
        switch (_state)
        {
            case CS_uncalibrated: return "CS_uncalibrated";
            case CS_calibrated: return "CS_calibrated";
            case CS_guessed: return "CS_guessed";
            default: return "unknown";
        }
    }

private:
    void calcCameraFovFromUndistortedCameraMat();
    void calculateUndistortedCameraMat();
    void buildUndistortionMaps();
    void createFromGuessedFOV(int imageWidthPX, int imageHeightPX, float fovH);
    ///////////////////////////////////////////////////////////////////////////////////
    CVMat _cameraMat;  //!< 3x3 Matrix for intrinsic camera matrix
    CVMat _distortion; //!< 4x1 Matrix for intrinsic distortion
    ///////////////////////////////////////////////////////////////////////////////////

    CVCalibState _state         = CS_uncalibrated; //!< calibration state enumeration
    float        _cameraFovVDeg = 0.0f;            //!< Vertical field of view in degrees
    float        _cameraFovHDeg = 0.0f;            //!< Horizontal field of view in degrees
    string       _calibDir;                        //!< directory of calibration file
    string       _calibFileName;                   //!< name for calibration file
    int          _calibFlags  = 0;                 //!< OpenCV calibration flags
    bool         _isMirroredH = false;             //!< Flag if image must be horizontally mirrored
    bool         _isMirroredV = false;             //!< Flag if image must be vertically mirrored

    int    _numCaptured = 0;           //!< NO. of images captured
    CVSize _boardSize;                 //!< NO. of inner chessboard corners.
    float  _boardSquareMM     = 20.f;  //!< Size of chessboard square in mm
    float  _reprojectionError = -1.0f; //!< Reprojection error after calibration
    CVSize _imageSize;                 //!< Input image size in pixels (after cropping)
    int    _camSizeIndex = -1;         //!< The requested camera size index

    CVMat        _undistortMapX;         //!< Undistortion float map in x-direction
    CVMat        _undistortMapY;         //!< Undistortion float map in y-direction
    CVMat        _cameraMatUndistorted;  //!< Camera matrix that defines scene camera and may also be used for reprojection of undistorted image
    string       _calibrationTime = "-"; //!< Time stamp string of calibration
    string       _computerInfos;
    CVCameraType _camType = CVCameraType::FRONTFACING;

    static const int    _CALIBFILEVERSION; //!< Global const file format version
    static const string _FTP_HOST;         //!< ftp host for calibration up and download
    static const string _FTP_USER;         //!< ftp login user for calibration up and download
    static const string _FTP_PWD;          //!< ftp login pwd for calibration up and download
    static const string _FTP_DIR;          //!< ftp directory for calibration up and download
};
//-----------------------------------------------------------------------------

#endif // CVCalibration_H
