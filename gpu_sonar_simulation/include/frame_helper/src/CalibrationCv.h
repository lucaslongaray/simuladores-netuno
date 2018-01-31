#ifndef __FRAME_HELPER_CALIBRATIONCV_H__
#define __FRAME_HELPER_CALIBRATIONCV_H__

#include <stddef.h>
#include <opencv2/core/core.hpp>
#include "Calibration.h"

namespace frame_helper
{
    /** 
     * Storage structure for opencv undistort maps. 
     *
     * Use the setCalibration function to calculate camMatrix and distCoeffs cv
     * structures.  Optionally set R and P matrices manually (see
     * cv::initUndistortRectifyMap). Call initCv to initiallize map1 and map2,
     * which can be used for the remap call for undistorting an image.
     */
    struct CameraCalibrationCv 
    {
	CameraCalibrationCv();
	void setCalibration( const CameraCalibration& calib );
	void setImageSize( cv::Size imageSize );
	void initCv();

	const CameraCalibration& getCalibration() const { return calib; }
	cv::Size getImageSize() const { return imageSize; }
	bool isInitialized() const;

	void undistortAndRectify( const cv::Mat& input, cv::Mat& output );

	cv::Mat camMatrix, distCoeffs;
	cv::Mat R, P;
	cv::Mat map1, map2;

    protected:
	CameraCalibration calib;
	cv::Size imageSize;
	bool valid;
	bool initialized;
    };

    /** 
     * Storage structure for a stereo camera setup.
     *
     * Similar to CameraCalibrationCv, a call to setCallibration will trigger
     * the generation of the CameraCalibrationCv for left and right, as well as
     * R and T matrices. A call to initCv will generate the Q matrix.
     */
    struct StereoCalibrationCv
    {
	StereoCalibrationCv();
	void setCalibration( const StereoCalibration& stereoCalib );
	StereoCalibration getCalibration() const;
	void setImageSize( cv::Size size );
	void initCv();

	cv::Size getImageSize() const { return imageSize; }
	bool isInitialized() const;

	CameraCalibrationCv camLeft, camRight;
	cv::Mat R, T, Q;
	
    protected:
	StereoCalibration calib;
	cv::Size imageSize;
	bool valid;
	bool initialized;
    };
}

#endif
