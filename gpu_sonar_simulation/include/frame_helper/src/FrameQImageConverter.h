/*
 * FrameQImageConverter.h
 *
 *  Created on: Oct 15, 2010
 *      Author: aduda
 */

/*
 * This Class is Qt depended !!!
 * It can be used in Qt projects to convert base::samples::frame::Frame
 * to QImages
 *
 */

#ifndef FRAMEQIMAGECONVERTER_H_
#define FRAMEQIMAGECONVERTER_H_

#include <QtGui/QImage>
#include "FrameHelper.h"
#include <stdexcept>
#include <math.h>
#include <limits>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

// cv::applyColorMap was moved to imgproc
// and contrib no longer exists
#if CV_MAJOR_VERSION < 3
    #include <opencv2/contrib/contrib.hpp>
#endif

namespace frame_helper
{
    //to a rgb image
    //if a vaule of 10 is reached ==>  h = 360° but s will be reduced 
    static const float MAX_H = 10.0f;    //this value is used to convert a 64Bit grayscale image 
    class FrameQImageConverter
    {
        public:
            FrameQImageConverter(){};
            virtual
                ~FrameQImageConverter(){};

            /**
             * Converts an HSV color value to RGB. Conversion formula
             * adapted from http://en.wikipedia.org/wiki/HSV_color_space.
             * Assumes h, s, and v are contained in the set [0, 1] and
             * returns r, g, and b in the set [0, 255].
             *
             * @param   Number  h       The hue
             * @param   Number  s       The saturation
             * @param   Number  v       The value
             * @return  Array           The RGB representation
             */
            void hsvToRgb(float h, float s, float v,int &r, int &g, int &b)
            {
                int i = h * 6;
                float f = h * 6 - i;
                float p = v * (1 - s);
                float q = v * (1 - f * s);
                float t = v * (1 - (1 - f) * s);
                switch(i % 6){
                case 0: r = v*255; g = t*255; b = p*255; break;
                case 1: r = q*255; g = v*255; b = p*255; break;
                case 2: r = p*255; g = v*255; b = t*255; break;
                case 3: r = p*255; g = q*255; b = v*255; break;
                case 4: r = t*255; g = p*255; b = v*255; break;
                case 5: r = v*255; g = p*255; b = q*255; break;
                }
            }

            //returns 1 if the size or format of dst has been changed  otherwise 0
            //int copyFrameToQImageRGB888(QImage &dst,base::samples::frame::frame_mode_t mode,int pixel_size, int width,int height,const char* pbuffer)
            //{
            int copyFrameToQImageRGB888(QImage &dst,base::samples::frame::frame_mode_t mode,int pixel_size, unsigned int width,unsigned int height, const char* pbuffer, const size_t buffer_size=0)
            {
                //Safty check to prevent error on empty images
                if(width == 0 || height  == 0 || buffer_size == 0)
                {
                    //TODO could be replaced by dummy "no-Image" image
                    dst = QImage();
                    return 0;
                }

                int ireturn = 0;
                //check if dst has the right format
                if((unsigned int) dst.width() != width || (unsigned int) dst.height()!= height || dst.format() != QImage::Format_RGB888)
                {
                    dst = QImage(width,height,QImage::Format_RGB888);
                    ireturn = 1;
                }
                switch(mode)
                {
                case base::samples::frame::MODE_UNDEFINED:
                    throw std::runtime_error(" FrameQImageConverter::copyFrameToQImageRGB888: Unknown frame mode!");
                    break;

                case base::samples::frame::MODE_BAYER_RGGB:
                case base::samples::frame::MODE_BAYER_GRBG:
                case base::samples::frame::MODE_BAYER_BGGR:
                case base::samples::frame::MODE_BAYER_GBRG:
                    FrameHelper::convertBayerToRGB24((const uint8_t*)pbuffer,(uint8_t*) dst.bits(), width, height ,mode);
                    break;

		case base::samples::frame::MODE_BGR:
                    // Provide the buffer as const uchar* and call bits() to make QImage
                    // do a deep copy. This is needed to ensure that QImage gets proper
                    // buffer alignment
                    dst = QImage((const uchar*)pbuffer, width, height, width*pixel_size, QImage::Format_RGB888);
		    dst = dst.rgbSwapped();
                    break;
		    
                case base::samples::frame::MODE_RGB:
                    // Provide the buffer as const uchar* and do a deep copy
                    // This is needed to ensure that QImage gets proper
                    // buffer alignment
                    dst = QImage((const uchar*)pbuffer, width, height, width*pixel_size, QImage::Format_RGB888).copy();
                    break;
                case base::samples::frame::MODE_PJPG:
                {
                   FrameHelper::convertPJPGToRGB24((const uint8_t*)pbuffer,(uint8_t*) dst.bits(),buffer_size, width, height);
                   break;
                }
                case base::samples::frame::MODE_JPEG:
                {
                    FrameHelper::convertJPEGToRGB24((const uint8_t*)pbuffer,(uint8_t*) dst.bits(),buffer_size, width, height);
                    break;
                }
		case base::samples::frame::MODE_UYVY:
                {
                    // WARNING
                    // WARNING: this code has been changed in order to account for the
                    // WARNING: 4-byte line padding requirements of QImage
                    // WARNING: due to the lack of an UYVY image provider at the moment,
                    // WARNING: it cannot be tested
                    // WARNING
                    // WARNING: please contact rock-dev if it does not work.
                    unsigned int i,j;
                    uint8_t u,v,y1,y2,cb,cr,r1,r2,b1,b2,g1,g2;
                    for(i = 0 ;i < height ;++i){
                        uint8_t* output_line = dst.scanLine(i);
                        for(j = 0 ;j < width ;j+=2){
                            u      = pbuffer[(i*width*2)+j*2+0];
                            y1     = pbuffer[(i*width*2)+j*2+1];
                            v      = pbuffer[(i*width*2)+j*2+2];
                            y2      = pbuffer[(i*width*2)+j*2+3];

                            cb = u;
                            cr = v;	

                            //no-rounding conversion method
                            r1 = (255/219)*(y1-16) + (255/112)*0.701*(cr-128);
                            g1 = (255/219)*(y1-16) + (255/112)*0.886*(0.114/0.587)*(cb-128)-(255/112)*0.701*(0.299/0.587)*(cr-128);
                            b1 = (255/219)*(y1-16) + (255/112)*0.886*(cb-128);

                            r2 = (255/219)*(y2-16) + (255/112)*0.701*(cr-128);
                            g2 = (255/219)*(y2-16) + (255/112)*0.886*(0.114/0.587)*(cb-128)-(255/112)*0.701*(0.299/0.587)*(cr-128);
                            b2 = (255/219)*(y2-16) + (255/112)*0.886*(cb-128);

                            output_line[j*3+0] = r1;
                            output_line[j*3+1] = g1;
                            output_line[j*3+2] = b1;
                            output_line[j*3+3] = r2;
                            output_line[j*3+4] = g2;
                            output_line[j*3+5] = b2;
                        }
                    }
                    break;
                }
                case base::samples::frame::MODE_GRAYSCALE:
                {
                    switch(pixel_size)
                    {
                        case 1:
                            dst = QImage((uchar*)pbuffer, width, height, width, QImage::Format_Indexed8).convertToFormat(QImage::Format_RGB888);
                            break;
                        case 2:
                            {
                            dst = QImage(width, height, QImage::Format_RGB888);

                            //The CV functions does not work in 16bit images, so using only 8bit
                            //image from below
                            const cv::Mat mat_src(height,width,CV_16U,(void*)pbuffer);
                            cv::Mat src_new(height,width,CV_8U);
                            cv::MatConstIterator_<uint16_t> it_src = mat_src.begin<uint16_t>(); 
                            cv::MatIterator_<uint8_t> it_dst = src_new.begin<uint8_t>(); 
                           
                            //Create a logarhitmic scale for visualization
                            for (; it_src != mat_src.end<uint16_t>(); ++it_src, ++it_dst)
                            {
                                uint16_t v = *it_src;
                                static double base = 2.2;
                                double v_log = log((double)v+1) / log(base);
                                (*it_dst) = v_log*(255.0/ log((double)65536) / log(base));
                            }

                            cv::Mat mat_dst(height,width,CV_8UC3);
                            cv::applyColorMap(src_new,mat_dst,cv::COLORMAP_RAINBOW);

                            //Manual copy the array, to make zero values black
                            cv::Vec3b black = cv::Vec3b(0, 0, 255); //*mat_dst.begin<cv::Vec3b>();
                            for (cv::Mat3b::iterator it = mat_dst.begin<cv::Vec3b>(); it != mat_dst.end<cv::Vec3b>(); ++it) 
                            {
                                if (*it == black) 
                                {
                                    *it = cv::Vec3b(0, 0, 0);
                                }
                            }
                            dst = QImage((const uchar*)mat_dst.data, width, height, width*3, QImage::Format_RGB888).copy();

                            break;
                            }
                        case 8:     //we have to scale the 64 data depth --> a rgb color coding is used
                        {
                            static const float SCALE_H = 1/MAX_H;
                            static const double SCALE_S = 1/floor(std::numeric_limits<double>::max()*SCALE_H);

                            int pixels = width*height;
                            double *data = (double*)pbuffer;
                            unsigned char* data2 =  dst.bits();
                            int r,g,b;
                            float h,s,v;
                            v = 1;      //use always a value of one for the color coding
                            //h and s are changed accordingly to the pixel value

                            //for each pixel of the map 
                            for(int i=0;i < pixels;++i,++data)
                            {
                                //display black if value is +- infinity
                                if(fabs(*data) == std::numeric_limits<double>::infinity())
                                {
                                    r = 0;
                                    g = 0;
                                    b = 0;
                                }
                                else
                                {
                                    //+ and - values are displayed in the same way
                                    if(*data > 0)
                                    {
                                        h = ::fmod(*data,MAX_H)*SCALE_H;
                                        s = 1-floor(*data*SCALE_H)*SCALE_S;
                                    }
                                    else
                                    {
                                        h = ::fmod(-*data,MAX_H)*SCALE_H;
                                        s = 1-floor(-*data*SCALE_H)*SCALE_S;
                                    }
                                    hsvToRgb(h,s,v,r,g,b);
                                }
                                //copy data to dest image
                                *data2 = (unsigned char) r;
                                ++data2;
                                *data2 = (unsigned char) g;
                                ++data2;
                                *data2 = (unsigned char) b;
                                ++data2;
                            }
                            break;
                        }
                        default:
                            throw std::runtime_error("Pixel size is not supported by FrameQImageConverter!");
                    }
                    break;
                }
                case base::samples::frame::MODE_RGB32:
                {
                    switch(pixel_size)
                    {
                        case 8:
                            dst = QImage((uchar*)pbuffer, width, height, width, QImage::Format_RGB32).convertToFormat(QImage::Format_RGB888);
                    }
                    break;
                }

                case base::samples::frame::MODE_PNG:
                {
                    char *buffer = const_cast<char*>(pbuffer);          // workaround for Mat non const constructor
                    const cv::Mat input(1,buffer_size,CV_8UC1,buffer);  // ensure buffer will not be modified !!!
                    cv::Mat img = cv::imdecode(input,CV_LOAD_IMAGE_COLOR);
                    dst = QImage((const uchar*)img.data,img.cols,img.rows,QImage::Format_RGB888);
		    dst = dst.rgbSwapped();
                    break;
                }

                default:
                    throw std::runtime_error(" FrameQImageConverter::copyFrameToQImageRGB888: Can not convert frame to RGB888!");
                }
                return ireturn;
            };

            int copyFrameToQImageRGB888(QImage &dst,const base::samples::frame::Frame &frame)
            {
                return copyFrameToQImageRGB888(dst,
                        frame.frame_mode,
                        frame.pixel_size,
                        frame.getWidth(),
                        frame.getHeight(),
                        (const char*)frame.getImageConstPtr(),
                        frame.image.size());
            };

            int copyFrameToQImageRGB888(QImage &dst,const QString &mode,int pixel_size, int width,int height, const char* pbuffer, const size_t size=0)
            {
                base::samples::frame::frame_mode_t _mode =  base::samples::frame::Frame::toFrameMode(mode.toStdString());
                return copyFrameToQImageRGB888(dst,_mode,pixel_size,width,height,pbuffer,size);
            };

        private:
            QImage image_buffer;
    };
    };

#endif /* FRAMEQIMAGECONVERTER_H_ */
