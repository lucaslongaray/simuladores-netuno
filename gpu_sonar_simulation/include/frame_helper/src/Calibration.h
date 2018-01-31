#ifndef __FRAME_HELPER_CALIBRATION_H__
#define __FRAME_HELPER_CALIBRATION_H__

#include <string>
#include <eigen3/Eigen/Geometry>
#include <base-types/base/Eigen.hpp>
#include <base-types/base/samples/Frame.hpp>
#include <base-types/base/Float.hpp>

namespace frame_helper
{
    /**
     * Calibration parameters expressing the camera matrix and the coefficients
     * for the lens distortion. See: 
     * http://opencv.willowgarage.com/documentation/camera_calibration_and_3d_reconstruction.html
     * for a description of the values and the model that is used.
     */
    struct CameraCalibration
    {
	CameraCalibration();

	CameraCalibration( double fx, double fy, double cx, double cy, 
                double d0, double d1, double d2, double d3, 
                int width = -1, int height = -1 );

        /** focal length the calibration in x and y direction
         */
	double fx, fy;
        /** optical center in x and y direction 
         */
        double cx, cy;
        /** distortion coefficients
         */
        double d0, d1, d2, d3;
        /** size of the image
         */
        int width, height;
        /** 
         * Re-projection error in pixels during calibration (x and y coordinates)
         * Calibration Errors 1-sigma standard deviation 
         */
        double ex, ey;

        /** 
         * indicates that the distrotion parameter are for a fisheye lens
         */
        bool fisheye;

	/**
	 * @return the 3x3 camera matrix, which converts scene points into screen points
	 */
	Eigen::Matrix3d getCameraMatrix() const;

        /**
         * @return the 2x2 covariance matrix of the pixel reprojection error
         */
        Eigen::Matrix2d getPixelCovariance() const;

	/**
	 * @return true if the calibration values are set
	 */
	bool isValid() const;

	/**
	 * @brief rescale the calibration to a different image size
	 *
	 * Use this function if you have a calibration for a particular resolution,
	 * and you have changed the resolution of the image. Useful for e.g. downscaled
	 * images with lower resolution. 
	 *
	 * @note this function will not maintain the aspect ratio, you have to
	 * do this yourselves.
	 */
	void rescale( int width, int height );

	/**
	 * @brief create a calibration struct based on the information embedded
	 *        in the frame
	 *
	 * this function will throw if the calibration parameters are not embedded
	 * as attributes in the frame
	 */
	static CameraCalibration fromFrame( const base::samples::frame::Frame& frame );

	/**
	 * @brief write the calibration into the attributes of the frame
	 *
	 * will throw if the size of the calibration does not match the size
	 * of the frame
	 */
	void toFrame( base::samples::frame::Frame& frame ) const;
    };

    /** 
     * extrinsic calibration parameters for a stereo camera setup
     */
    struct ExtrinsicCalibration
    {
	double tx, ty, tz;
	double rx, ry, rz;

	ExtrinsicCalibration();
	ExtrinsicCalibration( double tx, double ty, double tz, double rx, double ry, double rz );

	/**
	 * @return true if the calibration values are set
	 */
	bool isValid() const;

	/** 
	 * @return the transform that when applied to a point in the reference frame
         * of the left camera will give the point in the frame of the right camera
	 */
	Eigen::Affine3d getTransform() const;
    };

    /**
     * Full set of parameters required for a stereo camera calibration.
     */
    struct StereoCalibration
    {
	CameraCalibration camLeft, camRight;
	ExtrinsicCalibration extrinsic;

        StereoCalibration();
        StereoCalibration( const CameraCalibration& left, const CameraCalibration& right, const ExtrinsicCalibration& extrinsic );

	/**
	 * @return true if the calibration values are set
	 */
	bool isValid() const;

	/**
	 * Creates a calibration object from the output .txt file of
	 * the matlab calibration toolbox. Note, this will not make a valid
         * calibration, as the size of the target image still hast to be set
	 */
	static StereoCalibration fromMatlabFile( const std::string& file );

	/**
	 * Creates a calibration object from the output .txt file of
	 * the matlab calibration toolbox. 
         */
	static StereoCalibration fromMatlabFile( const std::string& file, int width, int height );

    };

}

#endif
