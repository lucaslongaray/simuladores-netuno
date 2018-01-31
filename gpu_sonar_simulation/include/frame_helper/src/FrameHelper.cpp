#include "FrameHelper.h"

#include <stdexcept>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <jpeg_conversion/src/jpeg_conversion.hpp>

#ifdef WITH_LIBV4l2
#include <libv4lconvert.h>
#endif

using namespace base::samples::frame;

namespace frame_helper
{
    enum ConvertMode{
        COPY = 0,
        RESIZE = 1,
        COLOR = 2,
        UNDISTORT = 4
    };

    FrameHelper::FrameHelper()
    {
	calibration.setImageSize( cv::Size( -1, -1 ) );
    }

    int FrameHelper::getOpenCvType(const base::samples::frame::Frame& frame)
    {
	int itype = 0;
	switch (frame.getChannelCount())
	{
	case 1:
	    switch (frame.getPixelSize())
	    {
	    case 1:
		itype = CV_8UC1;
		break;
	    case 2:
		itype = CV_16UC1;
		break;
	    default:
		throw "Unknown format. Can not convert Frame "
		"to cv::Mat.";
	    }
	    break;
	case 3:
	    switch (frame.getPixelSize())
	    {
	    case 3:
		itype = CV_8UC3;
		break;
	    case 6:
		itype = CV_16UC3;
		break;
		throw "Unknown format. Can not convert Frame "
		"to cv::Mat.";
	    }
	    break;
	    throw "Unknown format. Can not convert Frame "
	    "to cv::Mat.";
	}
	return itype;
    }

    cv::Mat FrameHelper::convertToCvMat(const base::samples::frame::Frame &frame)
    {
	return cv::Mat(frame.size.height,frame.size.width, getOpenCvType(frame), (void *)frame.getImageConstPtr());
    }

    //TODO
    //use exceptions 
    //use opencv for rotation
    //remove static variable (not thread save!!!)
    //move hole stuff to the end of the file (history will be lost)
    bool FrameHelper::convertPJPGToRGB24(const uint8_t *source, uint8_t *target_buffer,const size_t source_size, const int width, const int height)
    {
        #ifdef WITH_LIBV4l2
            #if defined __USE_BSD || defined __USE_XOPEN2K
                setenv("LIBV4LCONTROL_CONTROLS", "0", 1);
            #else
                putenv("LIBV4LCONTROL_CONTROLS=0");
            #endif
                static struct v4lconvert_data *v4lconvert_data;
                static struct v4l2_format src_fmt;
                static struct v4l2_format fmt;

                //Initialize v4l converter
                v4lconvert_data = v4lconvert_create(0);

                //Initializate needed conversion structures
                src_fmt.fmt.pix.pixelformat = v4l2_fourcc('P', 'J', 'P', 'G');
                src_fmt.fmt.pix.width = height;
                src_fmt.fmt.pix.height = width;
                src_fmt.fmt.pix.bytesperline = src_fmt.fmt.pix.width;
                src_fmt.fmt.pix.sizeimage = src_fmt.fmt.pix.width * src_fmt.fmt.pix.height;
                memcpy(&fmt, &src_fmt, sizeof fmt);

                fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_RGB24;
                fmt.fmt.pix.sizeimage = src_fmt.fmt.pix.width * src_fmt.fmt.pix.height * 3;

                unsigned char tmp[fmt.fmt.pix.sizeimage];

                //Do Conversion
                //int bytes = v4lconvert_convert(v4lconvert_data,&src_fmt,&fmt,(unsigned char*)source,source_size,(unsigned char*)target_buffer,fmt.fmt.pix.sizeimage);
                int bytes = v4lconvert_convert(v4lconvert_data,&src_fmt,&fmt,(unsigned char*)source,source_size,tmp,fmt.fmt.pix.sizeimage);



                //Check if conversion was sucsessful
                if(bytes < 0){
                    printf("error occoured %i, %i %s:%i\n",bytes,fmt.fmt.pix.sizeimage,__FILE__,__LINE__);
                    printf("Width: %i, height: %i, source_size: %i\n",width,height,(int)source_size);
                    return false;
                }

                //Image is rotated 90deg so rotate buffer
                for(int x=0;x<width;x++){
                    for(int y=0;y<height;y++){
                        target_buffer[y*(width*3)+(x*3)+0] = tmp[x*(height*3)+(y*3)+0];
                        target_buffer[y*(width*3)+(x*3)+1] = tmp[x*(height*3)+(y*3)+1];
                        target_buffer[y*(width*3)+(x*3)+2] = tmp[x*(height*3)+(y*3)+2];
                    }
                }
        #else
            throw std::runtime_error("frame_helper was built without v4l2 support!");
        #endif
        return true;	
    }


    void FrameHelper::convert(const base::samples::frame::Frame &src,
            base::samples::frame::Frame &dst,
            int offset_x,
            int offset_y,
            ResizeAlgorithm algo,
            bool bdewrap)
    {
        //set bayer pattern if not specified
        if(dst.getFrameMode() == MODE_BAYER && src.isBayer()) {
            dst.frame_mode = src.getFrameMode();
        }

        //find out which mode shall be used
        int mode = COPY;
        if(src.getFrameMode() != dst.getFrameMode() || src.getDataDepth() != dst.getDataDepth()) {
            mode += COLOR;
        }

        // Call resize only if destination frame sizes are properly initialized
        if(dst.size.width != 0 && dst.size.height != 0) 
        { 
            if(src.size.width != dst.size.width || src.size.height != dst.size.height)
		        mode += RESIZE;
        }

        if(bdewrap) {
	    // only apply the undistortion, if not already happened
	    if( src.getAttribute<int>("undistorted") != 1 )
		mode += UNDISTORT;
        }

        //this is needed to prevent copies 
        switch(mode)
        {
        case COPY:
            dst.init(src,true);
            break;
        case RESIZE:
            resize(src,dst,offset_x,offset_y,algo);
            break;
        case COLOR:
            convertColor(src,dst);
            break;
        case UNDISTORT:
            undistort(src,dst);
            break;
        case RESIZE + COLOR:
            frame_buffer.init(src.getWidth(),src.getHeight(),dst.getDataDepth(),dst.getFrameMode(),false);
            convertColor(src,frame_buffer);
            resize(frame_buffer,dst,offset_x,offset_y,algo);
            break;
        case RESIZE + UNDISTORT:
            dst.attributes.clear();
            frame_buffer2.init(dst,false);
            resize (src,frame_buffer2,offset_x,offset_y,algo);
            undistort(frame_buffer2,dst);
            break;
        case COLOR + UNDISTORT:
            dst.attributes.clear();
            frame_buffer2.init(dst,false);
            convertColor(src, frame_buffer2);
            undistort(frame_buffer2,dst);
            break;
        case RESIZE + COLOR + UNDISTORT:
            dst.attributes.clear();
            frame_buffer.init(src.getWidth(),src.getHeight(),dst.getDataDepth(),dst.getFrameMode(),false);
            convertColor(src,frame_buffer);
            frame_buffer2.init(dst,false);
            resize(frame_buffer,frame_buffer2,offset_x,offset_y,algo);
            undistort(frame_buffer2,dst);
            break;
        }
        dst.copyImageIndependantAttributes(src);
    }

    void FrameHelper::setCalibrationParameter(const CameraCalibration &para)
    {
        //calcCalibrationMatrix is called from dewrap when the image size is known
	calibration.setCalibration( para );
    }

    void FrameHelper::setCalibrationParameter(const CameraCalibrationCv &para)
    {
	calibration = para;
    }

    void FrameHelper::undistort(const base::samples::frame::Frame &src,
            base::samples::frame::Frame &dst)
    {
        // check if format is supported
        if(src.getFrameMode() != MODE_RGB && src.getFrameMode() != MODE_GRAYSCALE )
            throw std::runtime_error("FrameHelper::undistort: frame mode is not supported!");

	// avoid calling undistort twice
	if( src.getAttribute<int>( "undistorted" ) == 1 )
            throw std::runtime_error("The source frame seems to be already undistorted");

	// check if the calibration is valid
	if( !calibration.getCalibration().isValid() )
	{
	    // try to get the calibration from the source frame
	    calibration.setCalibration( CameraCalibration::fromFrame( src ) );

	    if( !calibration.getCalibration().isValid() )
		throw std::runtime_error("Could not get valid calibration parameters for undistort.");
	}

        // if is not yet valid, or if size has changed
        if( !calibration.isInitialized() || 
		calibration.getImageSize() != cv::Size( src.getWidth(), src.getHeight() ) )
        {
	    calibration.setImageSize( cv::Size( src.getWidth(), src.getHeight() ) );
	    calibration.initCv();
        }

        dst.init(src,false);
        const cv::Mat cv_src = FrameHelper::convertToCvMat(src);
        cv::Mat cv_dst = FrameHelper::convertToCvMat(dst);
        remap(cv_src, cv_dst, calibration.map1, calibration.map2, cv::INTER_CUBIC);

        //encode the focal length and center into the frame
	calibration.getCalibration().toFrame( dst );
	dst.setAttribute( "undistorted", 1 );
    }

    void FrameHelper::resize(const base::samples::frame::Frame &src,
            base::samples::frame::Frame &dst,
            int offset_x,int offset_y,
            ResizeAlgorithm algo)
    {
        // check if both images have the same color format
        if(src.getFrameMode() != dst.getFrameMode())
            throw std::runtime_error("FrameHelper::resize: Cannot resize frame. Dst and src have different frame modes.");

        // when compressed the image should not be resized
        if(src.isCompressed()) 
        {
            throw std::runtime_error("FrameHelper::resize: Compressed frames cannot be resized\
 yet, set 'width' and 'height' in the startup-script accordingly to the delivered camera image");
        }

        cv::Mat cv_src = FrameHelper::convertToCvMat(src);
        cv::Mat cv_dst = FrameHelper::convertToCvMat(dst);
        if(offset_x != 0 || offset_y != 0)
            cv_src = cv::Mat(cv_src,cv::Rect(offset_x,offset_y,cv_dst.cols,cv_dst.rows));

        switch(algo)
        {
        case INTER_LINEAR:
            cv::resize(cv_src, cv_dst, cv::Size(cv_dst.cols, cv_dst.rows), 0, 0, cv::INTER_LINEAR);
            break;
        case INTER_NEAREST:
            cv::resize(cv_src, cv_dst, cv::Size(cv_dst.cols, cv_dst.rows), 0, 0, cv::INTER_NEAREST);
            break;
        case INTER_AREA:
            cv::resize(cv_src, cv_dst, cv::Size(cv_dst.cols, cv_dst.rows), 0, 0, cv::INTER_AREA);
            break;
        case INTER_LANCZOS4:
            cv::resize(cv_src, cv_dst, cv::Size(cv_dst.cols, cv_dst.rows), 0, 0, cv::INTER_LANCZOS4);
            break;
        case INTER_CUBIC:
            cv::resize(cv_src, cv_dst, cv::Size(cv_dst.cols, cv_dst.rows), 0, 0, cv::INTER_CUBIC);
            break;
        case BAYER_RESIZE:
            resize_bayer(src,dst,offset_x,offset_y);
            break;
        }
    }

    void FrameHelper::resize_bayer(const base::samples::frame::Frame &src,base::samples::frame::Frame &dst,
            int offset_x,int offset_y)
    {
        //check if src mode is bayer
        if(!src.isBayer())
            throw std::runtime_error("FrameHelper::resize_bayer: Cannot resize frame. Src mode is not MODE_BAYER.");

        //check if both images have the same color format
        if(src.getFrameMode() != dst.getFrameMode())
            throw std::runtime_error("FrameHelper::resize_bayer: Cannot resize frame. Dst and src have different frame modes.");

        const uint8_t * psrc = src.getImageConstPtr()+offset_y*src.getWidth();
        uint8_t *pdst = dst.getImagePtr();
        int src_width = src.getWidth()-offset_x;
        int src_height = src.getHeight()-offset_y;
        
        const int resize_factor_x = src_width/ dst.size.width;
        const int resize_factor_y = src_height / dst.size.height;

        //check if dst size is a multiple of two
        if(dst.size.width%2 || dst.size.height%2)
            throw std::runtime_error("FrameHelper::resize_bayer: cannot resize frame. "
                    "The dst size is not a multiple of two");

        //check if src size is a multiple of two times dst size
        if(src_width%dst.size.width ||
           src_height%dst.size.height ||
                resize_factor_x != 2||
                resize_factor_y != 2)
            throw std::runtime_error("FrameHelper::resize_bayer: cannot resize frame. "
                    "Only 0.5 as scale factor is supported");

        //for each row
        for(int i=0; i<src_height;++i)
        {
            psrc += offset_x;
            const uint8_t *pend = psrc+src_width;
            //copy row
            while(psrc < pend)
            {
                *(pdst++) = *(psrc++);
                *(pdst++) = *(psrc++);
                //skip columns
                psrc = psrc + resize_factor_x;
            }
            //skip rows
            if(i%2)
            {
                i += resize_factor_y;
                psrc += resize_factor_y*src.getWidth();
            }
        }
    }

    void FrameHelper::convertColor(const base::samples::frame::Frame &src,base::samples::frame::Frame &dst)
    {
        switch(src.getFrameMode())
        {
        case MODE_BGR:
            switch(dst.getFrameMode())
            {
                //BGR --> BGR
            case MODE_BGR:
                if(src.getDataDepth() != dst.getDataDepth())
                    throw std::runtime_error("FrameHelper::convertColor: Cannot convert frame mode bgr to bgr with different data depths. Conversion is not implemented.");
                else
                    dst.init(src,true);
                break;
            case MODE_PJPG:
                throw  std::runtime_error("FrameHelper::convertColor: Cannot convert frame mode bgr to pjpg. Conversion is not implemented.");
                break;

                //BGR --> JPEG
            case MODE_JPEG:
            {
                conversion::JpegConversion jpeg_conversion;
                jpeg_conversion.compress(src, dst);
                break;
            }

                //BGR --> RGB
            case MODE_RGB:
                if(src.getDataDepth() != dst.getDataDepth())
                    throw std::runtime_error("FrameHelper::convertColor: Cannot convert frame mode bgr to rgb with different data depths. Conversion is not implemented.");
                else
                {
                    const cv::Mat cv_src = FrameHelper::convertToCvMat(src);
                    dst.init(cv_src.cols,cv_src.rows,src.getDataDepth(),dst.frame_mode,-1);
                    cv::Mat cv_dst = FrameHelper::convertToCvMat(dst);
                    cv::cvtColor(cv_src,cv_dst,cv::COLOR_BGR2RGB);
                }
                break;

                //BGR --> grayscale
            case MODE_GRAYSCALE:
                {
                    const cv::Mat cv_src = FrameHelper::convertToCvMat(src);
                    dst.init(cv_src.cols,cv_src.rows,src.getDataDepth(),dst.frame_mode,-1);
                    cv::Mat cv_dst = FrameHelper::convertToCvMat(dst);
                    cv::cvtColor(cv_src,cv_dst,cv::COLOR_BGR2GRAY);
                    break;
                }
                //BGR --> bayer pattern  
            case MODE_BAYER:
                throw  std::runtime_error("FrameHelper::convertColor: Cannot convert frame mode bgr to bayer pattern. Please specify bayer pattern (RGGB,GRBG,BGGR,GBRG).");
                break;

            case MODE_BAYER_RGGB:
            case MODE_BAYER_GRBG:
            case MODE_BAYER_BGGR:
            case MODE_BAYER_GBRG:
                throw  std::runtime_error("FrameHelper::convertColor: Cannot convert frame mode bgr to bayer pattern. Conversion is not implemented.");
                break;
                //BGR --> PNG
            case MODE_PNG:
            {
                const cv::Mat cv_src = FrameHelper::convertToCvMat(src);
                cv::imencode(".png",cv_src,dst.image);
                dst.size = src.size;
                break;
            }
            default:
                throw std::runtime_error("FrameHelper::convertColor: Cannot convert frame mode - mode is unknown");
            }
            break;

        case MODE_RGB:
            switch(dst.getFrameMode())
            {
                //RGB --> RGB
            case MODE_RGB:
                if(src.getDataDepth() != dst.getDataDepth())
                    throw std::runtime_error("FrameHelper::convertColor: Cannot convert frame mode rgb to rgb with different data depths. Conversion is not implemented.");
                else
                    dst.init(src,true);
                break;

                //RGB --> BGR 
            case MODE_BGR:
                if(src.getDataDepth() != dst.getDataDepth())
                    throw std::runtime_error("FrameHelper::convertColor: Cannot convert frame mode rgb to bgr with different data depths. Conversion is not implemented.");
                else
                {
                    const cv::Mat cv_src = FrameHelper::convertToCvMat(src);
                    dst.init(cv_src.cols,cv_src.rows,src.getDataDepth(),dst.frame_mode,-1);
                    cv::Mat cv_dst = FrameHelper::convertToCvMat(dst);
                    cv::cvtColor(cv_src,cv_dst,cv::COLOR_RGB2BGR);
                }
                break;


                //RGB --> grayscale
            case MODE_GRAYSCALE:
                convertRGBToGray(src,dst);
                break;

                //RGB --> bayer pattern  
            case MODE_BAYER:
                throw  std::runtime_error("FrameHelper::convertColor: Cannot convert frame mode rgb to bayer pattern. Please specify bayer pattern (RGGB,GRBG,BGGR,GBRG).");
                break;

            case MODE_BAYER_RGGB:
            case MODE_BAYER_GRBG:
            case MODE_BAYER_BGGR:
            case MODE_BAYER_GBRG:
                throw  std::runtime_error("FrameHelper::convertColor: Cannot convert frame mode rgb to bayer pattern. Conversion is not implemented.");
                break;

                //RGB --> PJPG
            case MODE_PJPG:
                throw  std::runtime_error("FrameHelper::convertColor: Cannot convert frame mode rgb to pjpg. Conversion is not implemented.");
                break;

                //RGB --> JPEG
            case MODE_JPEG:
            {
                conversion::JpegConversion jpeg_conversion;
                jpeg_conversion.compress(src, dst);
                break;
            }

                //RGB --> PNG
            case MODE_PNG:
            {
                const cv::Mat cv_src = FrameHelper::convertToCvMat(src);
                cv::Mat mat;
                cv::cvtColor(cv_src,mat,cv::COLOR_RGB2BGR);
                cv::imencode(".png",mat,dst.image);
                dst.size = src.size;
                break;
            }

            default:
                throw std::runtime_error("FrameHelper::convertColor: Cannot convert frame mode - mode is unknown");
            }
            break;




        case MODE_GRAYSCALE:
            switch(dst.getFrameMode())
            {
                //GRAYSCALE --> GRAYSCALE
            case MODE_GRAYSCALE:
                if(src.getDataDepth() != dst.getDataDepth())
                    throw std::runtime_error("FrameHelper::convertColor: Cannot convert frame mode grayscale to grayscale with different data depths. Conversion is not implemented.");
                else
                    dst.init(src,true);
                break;

                //GRAYSCALE --> JPEG
            case MODE_JPEG:
            {
                conversion::JpegConversion jpeg_conversion;
                jpeg_conversion.compress(src, dst);
                break;
            }

                //GRAYSCALE --> PNG
            case MODE_PNG:
            {
                const cv::Mat cv_src = FrameHelper::convertToCvMat(src);
                cv::imencode(".png",cv_src,dst.image);
                dst.size = src.size;
                break;
            }

            default:
                throw std::runtime_error("FrameHelper::convertColor: Cannot convert frame mode grayscale to ?. Conversion is not implemented.");
                break;
            }
            break;

        case MODE_PJPG:
        {
            switch(dst.getFrameMode())
            {
                //PJPG --> RGB
            case MODE_RGB:
                convertPJPGToRGB24(src,dst);
                break;

                //PJPG --> PJPG
            case MODE_PJPG:
                dst.init(src,true);
                break;

                throw  std::runtime_error("FrameHelper::convertColor: Cannot convert frame mode pjpg to gray pattern. Conversion not implements.");
                break;

                //PJPG --> bayer pattern
            case MODE_BAYER:
                throw  std::runtime_error("FrameHelper::convertColor: Cannot convert frame mode pjpg to bayer pattern. Please specify bayer pattern (RGGB,GRBG,BGGR,GBRG).");
                break;

            case MODE_BAYER_RGGB:
            case MODE_BAYER_GRBG:
            case MODE_BAYER_BGGR:
            case MODE_BAYER_GBRG:
                throw  std::runtime_error("FrameHelper::convertColor: Cannot convert frame mode pjpg to bayer pattern. Conversion is not implemented.");
                break;

            //PJPG --> JPEG
            case MODE_JPEG: 
            {
                base::samples::frame::Frame frame_tmp;
                convertPJPGToRGB24(src, frame_tmp);

                conversion::JpegConversion jpeg_conversion;
                jpeg_conversion.compress(frame_tmp, dst); // Copies image independent attributes as well.
                break;
            }
            default:
                throw std::runtime_error("FrameHelper::convertColor: Cannot convert frame mode pjpg - mode is unknown");
            }
            break;

        case MODE_BAYER:
            throw std::runtime_error("FrameHelper::convertColor: Cannot convert frame mode bayer to ?. Please specify bayer pattern (RGGB,GRBG,BGGR,GBRG).");
            break;
        }

        case MODE_BAYER_RGGB:
        case MODE_BAYER_GRBG:
        case MODE_BAYER_BGGR:
        case MODE_BAYER_GBRG:
            switch(dst.getFrameMode())
            {

            case MODE_PJPG:
                throw  std::runtime_error("FrameHelper::convertColor: Cannot convert frame mode bayer to pjpg. Conversion is not implemented.");
                break;

            case MODE_JPEG: 
            {
                base::samples::frame::Frame frame_tmp(src.getWidth(), src.getHeight(), src.getDataDepth(), base::samples::frame::MODE_RGB, -1);
                convertBayerToRGB24(src.getImageConstPtr(), frame_tmp.getImagePtr(), src.getWidth(), src.getHeight(), src.getFrameMode());
                frame_tmp.copyImageIndependantAttributes(src);

                conversion::JpegConversion jpeg_conversion;
                jpeg_conversion.compress(frame_tmp, dst); // Copies image independent attributes as well.
                break;
            }
                //bayer --> BGR
            case MODE_BGR:
                if(src.getDataDepth() != dst.getDataDepth())
                    throw std::runtime_error("FrameHelper::convertColor: Cannot convert frame mode bayer to bgr with different data depths. Conversion is not implemented.");
                dst.init(src.getWidth(),src.getHeight(),src.getDataDepth(),MODE_RGB,-1);
                frame_buffer3.init( src.getWidth(), src.getHeight(), src.getDataDepth(),dst.getFrameMode(),-1);
                convertBayerToRGB24(src.getImageConstPtr(),frame_buffer3.getImagePtr(),src.getWidth(),src.getHeight(),src.frame_mode);	
                {
                    cv::Mat cv_dst = FrameHelper::convertToCvMat(dst);
                    cv::cvtColor(FrameHelper::convertToCvMat(frame_buffer3),cv_dst,cv::COLOR_RGB2BGR);
                    dst.copyImageIndependantAttributes(src);
                }
                break;

                //bayer --> RGB
            case MODE_RGB:
                if(src.getDataDepth() != dst.getDataDepth())
                    throw std::runtime_error("FrameHelper::convertColor: Cannot convert frame mode bayer to rgb with different data depths. Conversion is not implemented.");
                dst.init(src.getWidth(),src.getHeight(),src.getDataDepth(),dst.getFrameMode(), -1);
                convertBayerToRGB24(src.getImageConstPtr(),dst.getImagePtr(),src.getWidth(),src.getHeight(),src.frame_mode);	
                dst.copyImageIndependantAttributes(src);
                break;

                //bayer --> grayscale
            case MODE_GRAYSCALE:
                if(src.getDataDepth() != dst.getDataDepth())
                    throw std::runtime_error("FrameHelper::convertColor: Cannot convert frame mode bayer to grayscale with different data depths. Conversion is not implemented.");

                // convert to RGB24 first and then to greyscale    
                frame_buffer3.init( src.getWidth(), src.getHeight(), src.getDataDepth(), MODE_RGB );
                convertBayerToRGB24(src.getImageConstPtr(),frame_buffer3.getImagePtr(),src.getWidth(),src.getHeight(),src.frame_mode);	

                dst.init(src.getWidth(),src.getHeight(),src.getDataDepth(),dst.getFrameMode());
                convertRGBToGray(frame_buffer3,dst);

                dst.copyImageIndependantAttributes(src);

                break;

                //bayer --> bayer pattern  
            case MODE_BAYER:
                dst.frame_mode = src.frame_mode;
            case MODE_BAYER_RGGB:
            case MODE_BAYER_GRBG:
            case MODE_BAYER_BGGR:
            case MODE_BAYER_GBRG:
                if(src.getDataDepth() == dst.getDataDepth() && dst.getFrameMode() == src.getFrameMode())
                    dst.init(src,true);
                else
                    throw std::runtime_error("FrameHelper::convertColor: Cannot convert frame mode bayer to bayer pattern with different pattern or data depth. Conversion is not implemented.");
                break;

            default:
                throw std::runtime_error("FrameHelper::convertColor: Cannot convert frame mode bayer to ? - mode is unknown");
            }
            break;




        case MODE_JPEG:
        {
            conversion::JpegConversion jpeg_conversion;
            switch(dst.getFrameMode()) 
            {
            //JPEG --> RGB
            case MODE_RGB:
            {
                jpeg_conversion.decompress(src, base::samples::frame::MODE_RGB, dst);
                break;
            }

            //JPEG --> BGR
            case MODE_BGR:
                jpeg_conversion.decompress(src, base::samples::frame::MODE_BGR, dst);
                break;

            // JPEG --> JPEG
            case MODE_JPEG:
                dst.init(src,true);
                break;

            //JPEG --> grayscale
            case MODE_GRAYSCALE:
                jpeg_conversion.decompress(src, base::samples::frame::MODE_GRAYSCALE, dst);
                break;

            case MODE_BAYER:
                throw  std::runtime_error("FrameHelper::convertColor: Cannot convert frame mode jpeg to bayer pattern. Please specify bayer pattern (RGGB,GRBG,BGGR,GBRG).");
                break;

            case MODE_BAYER_RGGB:
            case MODE_BAYER_GRBG:
            case MODE_BAYER_BGGR:
            case MODE_BAYER_GBRG:
                throw  std::runtime_error("FrameHelper::convertColor: Cannot convert frame mode jepg to bayer pattern. Conversion is not implemented.");
                break;

            case MODE_PJPG:
                throw  std::runtime_error("FrameHelper::convertColor: Cannot convert frame mode jpeg to pjpg. Conversion is not implemented.");
                break;

            default:
                throw std::runtime_error("FrameHelper::convertColor: Cannot convert frame mode jpeg - mode is unknown");
            }
            break;
        } // end case MODE_JPEG

        case MODE_PNG:
        {
            //PNG --> RGB
            switch(dst.getFrameMode())
            {
            case MODE_RGB:
                {
                    cv::Mat out = cv::imdecode(src.image,CV_LOAD_IMAGE_COLOR);
                    dst.init(out.cols,out.rows,8,MODE_RGB);
                    cv::cvtColor(out,FrameHelper::convertToCvMat(dst),cv::COLOR_BGR2RGB);
                    break;
                }

                //PNG --> BGR
            case MODE_BGR:
                {
                    cv::Mat out = cv::imdecode(src.image,CV_LOAD_IMAGE_COLOR);
                    copyMatToFrame(out,dst);
                    break;
                }

                // PNG --> PNG
            case MODE_PNG:
                dst.init(src,true);
                break;

                //PNG--> grayscale
            case MODE_GRAYSCALE:
                {
                    cv::Mat out = cv::imdecode(src.image,0);
                    copyMatToFrame(out,dst);
                    break;
                }

            default:
                throw std::runtime_error("FrameHelper::convertColor: Cannot convert frame mode png to requested mode. Requested mode is unknown");
            } // end dst.getFrameMode()
        }
        default:
            throw std::runtime_error("FrameHelper::convertColor: Cannot convert frame mode ?- mode is unknown");
        }  // end switch(src.getFrameMode())
    }

    //only a data depth of 1 Byte is supported
    void FrameHelper::convertRGBToGray(const base::samples::frame::Frame &src,base::samples::frame::Frame &dst,bool copy_attributes)
    {
        if(src.getPixelSize() != 3)
            throw std::runtime_error("FrameHelper::convertRGBToGray: Can only convert frame mode rgb 24 bit to grayscale 8 bit!");

        static bool initialized = false;
        static uint8_t rFact[256];
        static uint8_t gFact[256];
        static uint8_t bFact[256];

        if(!initialized)
        {
            // populating some lookup tables
            int i;
            for (i = 0; i < 256; ++i)
            {
                rFact[i] =  (uint8_t)((double)(0.299) * (double)(i));
                gFact[i] =  (uint8_t)((double)(0.587) * (double)(i));
                bFact[i] =  (uint8_t)((double)(0.114) * (double)(i));
            }
            initialized = true;
        }

        //set source to right format if it is not set
        if(src.getWidth() != dst.getWidth() ||
                src.getHeight() != dst.getHeight() ||
                dst.getFrameMode() != MODE_GRAYSCALE ||
                dst.getPixelSize() != 1)
        {
            dst.init(src.getWidth(),src.getHeight(),8,MODE_GRAYSCALE,-1);
        }

        //convert pixels
        const uint8_t *psrc = src.getImageConstPtr();
        uint8_t *pdst = dst.getImagePtr();
        const uint8_t *pend =  src.getLastConstByte();
        ++pend;
        while (psrc != pend)
        {
            uint8_t r = *(psrc++);
            uint8_t g = *(psrc++);
            uint8_t b = *(psrc++);
            uint32_t val =  (uint8_t)(rFact[r] + gFact[g] + bFact[b]);
            *(pdst++) = (uint8_t)( (val > 255) ? 255 : val);
        }

        //copy frame attributes
        if(copy_attributes)
            dst.copyImageIndependantAttributes(src);
    }


    //assumption 
    //the size of the object does not change with its position on the image plane
    cv::Point2f FrameHelper::calcRelPosToPoint(float fx,float fy, int x1,int y1,int x2,int y2, float d)
    {
        cv::Point2f point;
        point.x = (x1-x2)*d/fx;
        point.y = (y1-y2)*d/fy;
        return point;
    }

    //assumption 
    //the size of the object does not change with its position on the image plane
    float FrameHelper::calcDistanceToObject(float f,float virtual_size,float real_size)
    {
        return real_size*f/virtual_size;
    }

    cv::Point2f FrameHelper::calcRelPosToPoint(const base::samples::frame::Frame &frame,
            int x1,int y1,int x2,int y2, float d)
    {
        if(!frame.hasAttribute("fx"))
            throw std::runtime_error("FrameHelper::calcDistanceToObject: frame has no attribute fx");
        if(!frame.hasAttribute("fy"))
            throw std::runtime_error("FrameHelper::calcDistanceToObject: frame has no attribute fy");

        float fx = frame.getAttribute<float>("fx"); 
        float fy = frame.getAttribute<float>("fy"); 
        return calcRelPosToPoint(fx,fy,x1,y1,x2,y2,d);
    }

    cv::Point2f FrameHelper::calcRelPosToCenter(const base::samples::frame::Frame &frame,
            int x1,int y1, float d)
    {
        return calcRelPosToPoint(frame,x1,y1,frame.getWidth()*0.5,frame.getHeight()*0.5,d);
    }

    float FrameHelper::calcDistanceToObject(const base::samples::frame::Frame &frame,
            float virtual_width,float real_width,
            float virtual_height,float real_height)
    {
        if(virtual_width > virtual_height)
        {
            if(!frame.hasAttribute("fx"))
                throw std::runtime_error("FrameHelper::calcDistanceToObject: frame has no attribute fx");
            float fx = frame.getAttribute<float>("fx"); 
            return calcDistanceToObject(fx,virtual_width,real_width);
        }

        if(!frame.hasAttribute("fy"))
            throw std::runtime_error("FrameHelper::calcDistanceToObject: frame has no attribute fy");

        float fy = frame.getAttribute<float>("fy"); 
        return calcDistanceToObject(fy,virtual_height,real_height);
    }


    void FrameHelper::convertBayerToRGB24(const base::samples::frame::Frame &src,base::samples::frame::Frame &dst)
    {
        dst.init(src.getWidth(),src.getHeight(),src.getDataDepth(),MODE_RGB, -1);
        convertBayerToRGB24(src.getImageConstPtr(),dst.getImagePtr(),src.getWidth(),src.getHeight(),src.frame_mode);	
        dst.copyImageIndependantAttributes(src);
    }

    void FrameHelper::convertBayerToRGB24(const uint8_t *src, uint8_t *dst, int width, int height, base::samples::frame::frame_mode_t mode)
    {
        const int srcStep = width;
        const int dstStep = 3 * width;
        int blue = mode == MODE_BAYER_RGGB
            || mode == MODE_BAYER_GRBG ? 1 : -1;
        int start_with_green = mode == MODE_BAYER_GBRG
            || mode == MODE_BAYER_GRBG ;
        int i, imax, iinc;

        if (!(mode==MODE_BAYER_RGGB||mode==MODE_BAYER_GBRG||mode==MODE_BAYER_GRBG||mode==MODE_BAYER_BGGR))
            throw std::runtime_error("Helper::convertBayerToRGB24: Unknown Bayer pattern");

        // add a black border around the image
        imax = width * height * 3;
        // black border at bottom
        for (i = width * (height - 1) * 3; i < imax; i++) 
            dst[i] = 0;

        iinc = (width - 1) * 3;
        // black border at right side
        for (i = iinc; i < imax; i += iinc) {
            dst[i++] = 0;
            dst[i++] = 0;
            dst[i++] = 0;
        }

        dst ++;
        width --;
        height --;

        //for each row 
        for (; height--; src += srcStep, dst += dstStep) {
            const uint8_t *srcEnd = src + width;

            if (start_with_green) {
                dst[-blue] = src[1];
                dst[0] = (src[0] + src[srcStep + 1] + 1) >> 1;
                dst[blue] = src[srcStep];
                src++;
                dst += 3;
            }

            if (blue > 0) {
                for (; src <= srcEnd - 2; src += 2, dst += 6) {
                    dst[-1] = src[0];
                    dst[0] = (src[1] + src[srcStep] + 1) >> 1;
                    dst[1] = src[srcStep + 1];

                    dst[2] = src[2];
                    dst[3] = (src[1] + src[srcStep + 2] + 1) >> 1;
                    dst[4] = src[srcStep + 1];
                }
            } else {
                for (; src <= srcEnd - 2; src += 2, dst += 6) {
                    dst[1] = src[0];
                    dst[0] = (src[1] + src[srcStep] + 1) >> 1;
                    dst[-1] = src[srcStep + 1];

                    dst[4] = src[2];
                    dst[3] = (src[1] + src[srcStep + 2] + 1) >> 1;
                    dst[2] = src[srcStep + 1];
                }
            }

            if (src < srcEnd) {
                dst[-blue] = src[0];
                dst[0] = (src[1] + src[srcStep] + 1) >> 1;
                dst[blue] = src[srcStep + 1];
                src++;
                dst += 3;
            }

            src -= width;
            dst -= width * 3;

            blue = -blue;
            start_with_green = !start_with_green;
        }
    }

    void FrameHelper::calcDiff(const base::samples::frame::Frame &src1,const base::samples::frame::Frame &src2,base::samples::frame::Frame &dst)
    {
        int frame_total_size = src1.getNumberOfBytes();

        if(frame_total_size != static_cast<int>(src2.getNumberOfBytes()))
            throw std::runtime_error("calcDiff: size missmatch between src1 and src2 --> can not calc diff! ");
        if(src1.data_depth != 8)
            throw std::runtime_error("calcDiff: only 8 bit data depth is supported!");

        const uint8_t *p_src1 = src1.getImageConstPtr();
        const uint8_t *p_src2 = src2.getImageConstPtr();

        dst.init(src1,false);
        uint8_t *p_dst = dst.getImagePtr();

        for(int i=0;i < frame_total_size;++i)
        {
            if(*p_src1 > *p_src2)
                *p_dst = *p_src1-*p_src2;
            else
                *p_dst = 0;//*p_src2-*p_src1;
            ++p_dst;
            ++p_src1;
            ++p_src2;
        }
    }

    void FrameHelper::convertBayerToGreenChannel(const base::samples::frame::Frame &src,base::samples::frame::Frame &dst)
    {
        //check dst format 
        if(dst.frame_mode != MODE_GRAYSCALE)
            std::runtime_error("FrameHelper::converBayerToGreenChannel: Cannot convert frame mode bayer to green channel. Dst image must be of mode MODE_GRAYSCALE.");

        if(src.getDataDepth() != dst.getDataDepth())
            std::runtime_error("FrameHelper::converBayerToGreenChannel: Cannot convert frame mode bayer to green channel. src and dst must have the same data depth.");

        dst.init(src.getWidth(),src.getHeight(),src.getDataDepth(),dst.getFrameMode(),-1);
        convertBayerToGreenChannel(src.getImageConstPtr(),dst.getImagePtr(),src.getWidth(),src.getHeight(),src.frame_mode);	
        dst.copyImageIndependantAttributes(src);
    }

    //reads the green channel from an image which uses a bayer pattern
    void FrameHelper::convertBayerToGreenChannel(const uint8_t *src, uint8_t *dst, int width, int height, base::samples::frame::frame_mode_t mode)
    {
        const int srcStep = width;
        const int dstStep =  width;
        int blue = mode == MODE_BAYER_RGGB
            || mode == MODE_BAYER_GRBG ? 1 : -1;
        int start_with_green = mode == MODE_BAYER_GBRG
            || mode == MODE_BAYER_GRBG ;
        int i, imax;

        if (!(mode==MODE_BAYER_RGGB||mode==MODE_BAYER_GBRG||mode==MODE_BAYER_GRBG||mode==MODE_BAYER_BGGR))
            throw std::runtime_error("Helper::convertBayerToRGB24: Unknown Bayer pattern");

        // add a black border around the image
        imax = width * height;

        // black border at bottom
        for(i = width * (height - 1); i < imax; i++) 
        {
            dst[i] = 0;
        }

        // black border at right side
        for (i = width-1; i < imax; i += width) 
            dst[i] = 0;

        //   dst ++;
        width --;
        height --;

        //for each row 
        for (; height--; src += srcStep, dst += dstStep) {
            const uint8_t *srcEnd = src + width;
            if (start_with_green) {
                dst[0] = (src[0] + src[srcStep + 1] + 1) >> 1;
                src++;
                dst++;
            }
            if (blue > 0) {
                for (; src <= srcEnd - 2; src += 2, dst += 2) {
                    dst[0] = (src[1] + src[srcStep] + 1) >> 1;
                    dst[1] = (src[1] + src[srcStep + 2] + 1) >> 1;
                }
            } else {
                for (; src <= srcEnd - 2; src += 2, dst += 2) {
                    dst[0] = (src[1] + src[srcStep] + 1) >> 1;
                    dst[1] = (src[1] + src[srcStep + 2] + 1) >> 1;
                }
            }

            if (src < srcEnd) {
                dst[0] = (src[1] + src[srcStep] + 1) >> 1;
                src++;
                dst++;
            }
            src -= width;
            dst -= width;

            blue = -blue;
            start_with_green = !start_with_green;
        }
    }

    void FrameHelper::convertPJPGToRGB24(const base::samples::frame::Frame &src,base::samples::frame::Frame &dst)
    {
        //check src format 
        if(src.frame_mode != MODE_PJPG)
            std::runtime_error("FrameHelper::convertPJPGToRGB888: Source frame mode must be MODE_PJPG.");

        //check dst format 
        if(dst.frame_mode != MODE_RGB)
            std::runtime_error("FrameHelper::convertPJPGToRGB888: Dst frame mode must be of mode MODE_RGB.");

        dst.init(src.getWidth(),src.getHeight(),8,dst.getFrameMode(),-1);
        convertPJPGToRGB24(src.getImageConstPtr(),dst.getImagePtr(),src.image.size(),src.getWidth(),src.getHeight());	
        dst.copyImageIndependantAttributes(src);
    }

    void FrameHelper::copyMatToFrame(const cv::Mat &src, base::samples::frame::Frame &frame)
    {
        frame_mode_t mode;
        int color_depth;
        switch (src.type())
        {
        case CV_8UC1:
            mode = MODE_GRAYSCALE;
            color_depth = 8;
            break;
        case CV_16UC1:
            mode = MODE_GRAYSCALE;
            color_depth = 16;
            break;
        case CV_8UC3:
            mode = MODE_BGR;
            color_depth = 8;
            break;
        default:
            throw std::runtime_error( "Unknown format. Can not convert cv:Mat to Frame." );
        }
        frame.init(src.cols,src.rows,color_depth,mode);
        cv::Mat dst = FrameHelper::convertToCvMat(frame);

        //this is only working if dst has the right size otherwise
        //cv is allocating new memory
        src.copyTo(dst);
    }

    void FrameHelper::convertJPEGToRGB24(uint8_t const* src, uint8_t* dst, 
            size_t const src_size, int const width, int const height)
    {
        conversion::JpegConversion jpeg_conversion;
        jpeg_conversion.decompress(src, src_size, width, height, 
                base::samples::frame::MODE_RGB, dst);
    }

    void FrameHelper::rotateBy180Degrees(const base::samples::frame::Frame &src,
            base::samples::frame::Frame &dst)
    {
        dst.init(src, false);

        cv::Mat cv_src = FrameHelper::convertToCvMat(src);
        cv::Mat cv_dst = FrameHelper::convertToCvMat(dst);

        //flips horizontally and vertically (same as rotating by 180 degrees)
        cv::flip(cv_src, cv_dst, -1);
    }

    void FrameHelper::saveFrame(const std::string &filename,const base::samples::frame::Frame &frame)
    {
        base::samples::frame::Frame temp;
        frame_mode_t mode = frame.frame_mode == MODE_GRAYSCALE ? MODE_GRAYSCALE : MODE_BGR;
        temp.init(frame.getWidth(),frame.getHeight(),frame.getDataDepth(),mode);
        convertColor(frame,temp);
        cv::imwrite(filename.c_str(),convertToCvMat(temp));
    }

    void FrameHelper::loadFrame(const std::string &filename,base::samples::frame::Frame &frame)
    {
        cv::Mat mat = cv::imread(filename.c_str());
        copyMatToFrame(mat,frame);
    }
}
