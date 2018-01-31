#ifndef FRAMEHELPER_H
#define FRAMEHELPER_H

// Workaround for GCC 4.6
#include <stddef.h>
#include <base-types/base/samples/Frame.hpp>
#include <opencv2/core/core.hpp>
#include "FrameHelperTypes.h"
#include "CalibrationCv.h"
//#include <base/samples/compressed_frame.h>

//TODO 
//at the moment some functions call copyImageIndependantAttributes
//automatically
//this should be replaced by a flag

namespace frame_helper
{
    class FrameHelper
    {
        private:
            //the buffer are used to convert one frame to an other one 
            //if src and dst attributes are not changing no memory allocation
            //is done
            //DO NOT USE THIS VARIABLES INSIGHT any function other than convert
            base::samples::frame::Frame frame_buffer;
            base::samples::frame::Frame frame_buffer2;

            //used insight convertColor
            base::samples::frame::Frame frame_buffer3;

            //mapping matrix for undistort
            //if the size of the image is not changing
            //the mapping is calculated only once 
            CameraCalibrationCv calibration;

        public:
            FrameHelper();

            //converts one frame to another frame
            //this can be used to convert colors resize and undistort frames
            //the attributes are implied by the dst attributes
            //call setCalibrationParameters to set the calibration parameters
            //this is none static because an internal buffer is used which is not be resized if the
            //src and dst have always the same attributes
            void convert(const base::samples::frame::Frame &src,base::samples::frame::Frame &dst,
                    int offset_x = 0, int offset_y = 0, ResizeAlgorithm algo = INTER_LINEAR, bool bundistort=false);

            //sets the calibration paramter for the camera
            //parameter. Be aware that this does not set the R and P parameter
	    //for stereo undistort. Use the method that takes a cameraCalibrationCv
	    //instead.
            //para              intrinsic and distortion parameters
            void setCalibrationParameter(const CameraCalibration &para);
	    
	    // sets the calibration parameters for the camera
	    // which uses R and P matrices for stereo calibration as well
            void setCalibrationParameter(const CameraCalibrationCv &para);

            //this is a convenience function to undistort a frame
            //call setCalibrationParameter first
            //see cv::remap 
            void undistort(const base::samples::frame::Frame &src,base::samples::frame::Frame &dst);


            //copies a cv::Mat into a Frame
            //frame is initialized if it has not got the right size
            static void copyMatToFrame(const cv::Mat &src, base::samples::frame::Frame &frame);

            //resizes a bayer image without converting it to another color format by skipping pixels
            //The size of src must be a multiple of two times dst size
            //otherwise an exception will be thrown
            static void resize_bayer(const base::samples::frame::Frame &src,base::samples::frame::Frame &dst,
                    int offset_x = 0, int offset_y = 0);

            //resize a frame
            //the size is implied by the dst frame
            static void resize(const base::samples::frame::Frame &src,base::samples::frame::Frame &dst,
                    int offset_x = 0, int offset_y = 0,ResizeAlgorithm algo = INTER_LINEAR);

            //rotates a frame by 180 degrees
            //dst is inititialized as a copy of src with rotated image
            static void rotateBy180Degrees(const base::samples::frame::Frame &src,
                    base::samples::frame::Frame &dst);


            //calculates the distance in meters to an object seen on an image
            //parameters:
            //f                 focal length of the camera in pixels which has taken the image
            //                  set f = fx if the width of the object is used as size
            //                  set f = fy if the height of the object is used as size
            //virtual_size      size of the object in pixels  
            //real_size         size of the object in meters 
            static float calcDistanceToObject(float f,float virtual_size,float real_size);


            //calculates the distance in meters to an object seen on an image
            //parameters:
            //frame             frame which must have the additional attributes fx (focal length) and fy 
            //virtual_width     width of the object in pixels  
            //real_width        width of the object in meters 
            //virtual_height    height of the object in pixels  
            //real_height       height of the object in meters 
            //you can set width or height to zero if you do not want to use it
            //internal always the biggest dimension is used to calculate the distance
            static float calcDistanceToObject(const base::samples::frame::Frame &frame,
                                              float virtual_width,float real_width,
                                              float virtual_height,float real_height);

            //calculates the relative positions in meters between to image points
            //parameters:
            //fx        focal length of the camera 
            //fy        focal length of the camera 
            //x1        x coordinate of the first point in pixels 
            //y1        y coordinate of the first point in pixels
            //x2        x coordinate of the second point in pixels 
            //y2        y coordinate of the second point in pixels
            //d         distance between the camera and the two points
            static cv::Point2f calcRelPosToPoint(float fx,float fy, int x1,int y1,int x2,int y2, float d);
            static cv::Point2f calcRelPosToPoint(const base::samples::frame::Frame &frame,
                                             int x1,int y1,int x2,int y2, float d);
            static cv::Point2f calcRelPosToCenter(const base::samples::frame::Frame &frame,int x1,int y1,float d);

            //converts src to dst 
            //the mode of dst implies the conversion 
            void convertColor(const base::samples::frame::Frame &src,base::samples::frame::Frame &dst);

            //converts a bayer pattern image to rgb image which has 8 bit data depth for each channel
            static void convertBayerToRGB24(const base::samples::frame::Frame &src,base::samples::frame::Frame &dst);
            static void convertBayerToRGB24(const uint8_t *src, uint8_t *dst, int width, int height ,base::samples::frame::frame_mode_t mode);

            //converts a rgb image to gray scale image
            static void convertRGBToGray(const base::samples::frame::Frame &src,base::samples::frame::Frame &dst,bool copy_attributes =true);

            //copies only the green channel of a bayer pattern image to dst
            //dst has to be MODE_GRAYSCALE
            static void convertBayerToGreenChannel(const base::samples::frame::Frame &src,base::samples::frame::Frame &dst);
            static void convertBayerToGreenChannel(const uint8_t *src, uint8_t *dst, int width, int height, base::samples::frame::frame_mode_t mode);

            //calculates the diff between src1 and src2 
            //dst[i] = |src1[i]-src2[i]|
            //only 8 Bit data depth is supported 
            static void calcDiff(const base::samples::frame::Frame &src1,const base::samples::frame::Frame &src2,base::samples::frame::Frame &dst);
			
	    /**
	     * /Brief conversion for PJPG to RGB24.
	    */
	    static void convertPJPGToRGB24(const base::samples::frame::Frame &src, base::samples::frame::Frame &dst);
	    static bool convertPJPGToRGB24(const uint8_t *source, uint8_t *target_buffer,const size_t source_size, const int width, const int height);

            // JPEG to RGB24 conversion using JpegConversion.
            static void convertJPEGToRGB24(uint8_t const* src, uint8_t* dst, size_t const src_size, int const width, int const height);
	    
	    static cv::Mat convertToCvMat(const base::samples::frame::Frame &frame);
	    
	    static int getOpenCvType(const base::samples::frame::Frame &frame);
    
            void saveFrame(const std::string &filename,base::samples::frame::Frame const &frame);
            void loadFrame(const std::string &filename,base::samples::frame::Frame &frame);
    };
};
#endif // FRAMEHELPER_H
