#include "CalibrationCv.h"
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <stdexcept>

using namespace frame_helper;

CameraCalibrationCv::CameraCalibrationCv() : valid(false), initialized(false) {}

void CameraCalibrationCv::setCalibration( const CameraCalibration& calib )
{
    if( !calib.isValid() )
        throw std::runtime_error("Camera calibration values are not valid.");
    this->calib = calib;

    camMatrix.create(3, 3, CV_64F);
    camMatrix = 0.0;
    camMatrix.at<double>(2,2) = 1.0;
    camMatrix.at<double>(0,0) = calib.fx;
    camMatrix.at<double>(0,2) = calib.cx;
    camMatrix.at<double>(1,1) = calib.fy;
    camMatrix.at<double>(1,2) = calib.cy;

    distCoeffs.create(1, 4, CV_64F);
    distCoeffs.at<double>(0,0) = calib.d0;
    distCoeffs.at<double>(0,1) = calib.d1;
    distCoeffs.at<double>(0,2) = calib.d2;
    distCoeffs.at<double>(0,3) = calib.d3;

    imageSize = cv::Size( calib.width, calib.height );

    initialized = false;
    valid = true;
}

void CameraCalibrationCv::setImageSize( cv::Size size )
{
    initialized = false;
    imageSize = size;
}

void CameraCalibrationCv::initCv()
{
    if( imageSize == cv::Size() )
	throw std::runtime_error("CameraCalibrationCv: image size not set.");

    if( !valid )
	throw std::runtime_error("CameraCalibrationCv: calibration not set.");

    cv::initUndistortRectifyMap(
	    camMatrix, distCoeffs, 
	    R, P, imageSize, 
	    CV_32FC1, map1, map2 );

    initialized = true;
}

bool CameraCalibrationCv::isInitialized() const
{
    return initialized;
}

void CameraCalibrationCv::undistortAndRectify( const cv::Mat& input, cv::Mat& output )
{
    // undistort/rectify image
    cv::remap(input, output, map1, map2, cv::INTER_CUBIC);
}

StereoCalibrationCv::StereoCalibrationCv() : valid(false), initialized(false) {}

void StereoCalibrationCv::setCalibration( const StereoCalibration& stereoCalib )
{
    calib = stereoCalib;
    camLeft.setCalibration( stereoCalib.camLeft );
    camRight.setCalibration( stereoCalib.camRight );

    if( !calib.extrinsic.isValid() )
        throw std::runtime_error("Stereo calibration values are not valid.");

    T.create(3, 1, CV_64F);
    T.at<double>(0,0) = calib.extrinsic.tx;
    T.at<double>(1,0) = calib.extrinsic.ty;
    T.at<double>(2,0) = calib.extrinsic.tz;

    //convert from rotation vector to rotation matrix
    cv::Mat tempRot;
    tempRot.create(3, 1, CV_64F);
    tempRot.at<double>(0,0) = calib.extrinsic.rx;
    tempRot.at<double>(1,0) = calib.extrinsic.ry;
    tempRot.at<double>(2,0) = calib.extrinsic.rz;
    cv::Rodrigues(tempRot, R);

    valid = true;
}

StereoCalibration StereoCalibrationCv::getCalibration() const
{
    return calib;
}

void StereoCalibrationCv::setImageSize( cv::Size size )
{
    camLeft.setImageSize( size );
    camRight.setImageSize( size );
    imageSize = size;
}

void StereoCalibrationCv::initCv()
{
    if( imageSize == cv::Size() )
	throw std::runtime_error("CameraCalibrationCv: image size not set.");

    if( !valid )
	throw std::runtime_error("CameraCalibrationCv: calibration not set.");

    cv::stereoRectify(
	    camLeft.camMatrix,
	    camLeft.distCoeffs,
	    camRight.camMatrix,
	    camRight.distCoeffs,
	    imageSize,
	    R,
	    T,
	    camLeft.R,
	    camRight.R,
	    camLeft.P,
	    camRight.P,
	    Q,
	    cv::CALIB_ZERO_DISPARITY,
	    0 // zoom, so that only valid pixels are visible
	    );


  if (std::isnan(Q.at<double>(3, 3)))
  {
     throw std::runtime_error("StereoCalibrationCv::initCv: nan in Q calculation. Check calibration parameters.");
  }

   camLeft.initCv();
    camRight.initCv();

    initialized = true;
}

bool StereoCalibrationCv::isInitialized() const
{
    return initialized && camLeft.isInitialized() && camRight.isInitialized();
}

