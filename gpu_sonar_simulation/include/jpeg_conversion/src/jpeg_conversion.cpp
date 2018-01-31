#include "jpeg_conversion.hpp"
#include <string.h>

namespace conversion {

void JpegConversion::compress(base::samples::frame::Frame const& frame_input,
        base::samples::frame::Frame& frame_output) {

    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;
    struct jpeg_destination_mgr mDestMgr;

    bool flip_rgb = false;
    bool is_jpeg = false;

    J_COLOR_SPACE j_color_space = getJpegColorspace(frame_input.getFrameMode(), 
            flip_rgb, is_jpeg);
    
    if(is_jpeg) {
        LOG_INFO("Image already compressed");
        frame_output.init(frame_input);
        return;
    }

    if(j_color_space == JCS_UNKNOWN) {
        throw  std::runtime_error("Frame color mode can not be compressed to JPEG.");
    }

    // Create buffer big enough for the jpeg.
    unsigned int new_buffer_size = frame_input.getWidth() * frame_input.getHeight() * 
            frame_input.getPixelSize();
    LOG_DEBUG("New buffer size: %d (width %d, height %d, pixel-size %d)", new_buffer_size, 
            frame_input.getWidth(), frame_input.getHeight(), frame_input.getPixelSize());

    // Create our in-memory output buffer to hold the jpeg.
    if(new_buffer_size > mBufferSize) {
        if(mBuffer != NULL) {
            delete mBuffer;
        }
        mBuffer = new unsigned char[new_buffer_size];
        mBufferSize = new_buffer_size;
        if(mBuffer != NULL) {
            LOG_DEBUG("New buffer memory allocated");
        } else {
            LOG_ERROR("New buffer could not be allocated");
            return;
        }
    }

    cinfo.err = jpeg_std_error(&jerr); // Has to be set before calling jpeg-create...
    LOG_DEBUG("Create compress structure");
    jpeg_create_compress(&cinfo);

    // Input
    LOG_DEBUG("Set image size and color space %d", j_color_space);
    cinfo.image_width      = frame_input.getWidth();
    cinfo.image_height     = frame_input.getHeight();
    cinfo.input_components = frame_input.getPixelSize();
    cinfo.in_color_space   = j_color_space;

    // Output
    LOG_DEBUG("Set further default compression parameters and the compression quality");
    jpeg_set_defaults(&cinfo);
    jpeg_set_quality (&cinfo, mJpegQuality, true);
    // Define destination.
    LOG_DEBUG("Pass output buffer and set destination methods");
    mDestMgr.next_output_byte    = mBuffer;
    mDestMgr.free_in_buffer      = mBufferSize;
    // Set destination methods.
    mDestMgr.init_destination    = init_buffer; // ijg callback
    mDestMgr.empty_output_buffer = empty_buffer; // ijg callback
    mDestMgr.term_destination    = term_buffer; // ijg callback
    cinfo.dest = &mDestMgr;
 
    LOG_DEBUG("Start compression");
    jpeg_start_compress(&cinfo, true);

    uint8_t const* image_to_compress =  frame_input.getImageConstPtr();
    LOG_DEBUG("Received image pointer %p", image_to_compress);

    // Create flip buffer if neccessary and copy image (input image should not be changed)
    if(flip_rgb) {
        LOG_DEBUG("Flip RGB");
        if(frame_input.getNumberOfBytes() > mFlipBufferSize) {
            LOG_DEBUG("Allocate new flip buffer with %d bytes", frame_input.getNumberOfBytes());
            if(mFlipBuffer != NULL) {
                LOG_DEBUG("Delete old flip-buffer");
                delete mFlipBuffer;
            }
            mFlipBuffer = new unsigned char[frame_input.getNumberOfBytes()];
            mFlipBufferSize = frame_input.getNumberOfBytes();
        }
        // Copy frame_input as BGR to mFlipBuffer.
        LOG_DEBUG("Fill flip buffer");
        switchRB(frame_input.getNumberOfBytes(), frame_input.getImageConstPtr(), mFlipBuffer);
        // Reset working image to created copy.
        image_to_compress = mFlipBuffer;
    }

    int row_size = frame_input.getRowSize();
    JSAMPROW row_pointer;
    LOG_DEBUG("Requested row size %d", row_size);

    // Main code to write jpeg data.
    LOG_DEBUG("Compress image");
    unsigned int ret_sum = 0;
    while (cinfo.next_scanline < cinfo.image_height) { 		
	    row_pointer = (JSAMPROW) &image_to_compress[cinfo.next_scanline * row_size];
	    ret_sum += jpeg_write_scanlines(&cinfo, &row_pointer, 1);
    }
    LOG_DEBUG("Scanlines written %d", ret_sum);

    LOG_DEBUG("Finish compression");
    jpeg_finish_compress(&cinfo);

    // Calculate size of the jpeg.
    int jpeg_size = cinfo.dest->next_output_byte - mBuffer;
    LOG_DEBUG("Calculated jpeg size %d", jpeg_size);

    LOG_DEBUG("Initialize output frame and set buffer");
    // Initialize member frame to JPEG with calculated size.
    frame_output.init(frame_input.getWidth(), frame_input.getHeight(), 8, 
            base::samples::frame::MODE_JPEG, 0, jpeg_size);

    // Copy jpeg to Frame buffer.
    frame_output.setImage((const char*)mBuffer, jpeg_size);

    // Copy image attributes including the importing status!
    frame_output.copyImageIndependantAttributes(frame_input);

    LOG_DEBUG("Destroy compression structure");
    jpeg_destroy_compress(&cinfo);
}

void JpegConversion::decompress(base::samples::frame::Frame const& frame_input, 
        base::samples::frame::frame_mode_t const decompress_to, 
        base::samples::frame::Frame& frame_output) {  

    // The input frame has to be a JPEG.
    bool input_flip_rgb = false;
    bool input_is_jpeg = false;

    getJpegColorspace(frame_input.getFrameMode(), input_flip_rgb, input_is_jpeg);

    if(!input_is_jpeg) {
        LOG_WARN("Only jpegs can be decompressed");
        throw  std::runtime_error("Only JPEGs can be decompressed.");
    }

    LOG_DEBUG("Initialize output frame (width %d, height %d", 
            frame_input.getWidth(), frame_input.getHeight());
    frame_output.init(frame_input.getWidth(), frame_input.getHeight(), 8, 
            decompress_to);

    // Copy image attributes including the importing status!
    frame_output.copyImageIndependantAttributes(frame_input);

    decompress(frame_input.getImageConstPtr(),
        frame_input.getNumberOfBytes(),
        frame_input.getWidth(),
        frame_input.getHeight(),
        decompress_to,
        frame_output.getImagePtr());
}
        
void JpegConversion::decompress(uint8_t const* src, 
        size_t src_size,
        int width, 
        int height,
        base::samples::frame::frame_mode_t const decompress_to,  
        uint8_t* dst) {

    if(src == NULL || dst == NULL) {
        LOG_ERROR("NULL pointer passed to decompress()");
        return;
    }

    // Just copy input if we should decompress to JPEG and abort if the color format
    // of the output image is not supported.
    bool output_flip_rgb = false;
    bool output_is_jpeg = false;

    J_COLOR_SPACE j_color_space_output = getJpegColorspace(decompress_to, 
            output_flip_rgb, output_is_jpeg);

    if(output_is_jpeg) {
        LOG_INFO("Image already compressed, data will just be copied");
        memcpy(dst, src, src_size);
        return;
    }

    if(j_color_space_output == JCS_UNKNOWN) {
        LOG_ERROR("Output color format not supported");
        throw  std::runtime_error("Frame can not be decompressed to the color mode of the output frame");
    }

	struct jpeg_decompress_struct dinfo; 
    struct jpeg_error_mgr jerr;
    struct jpeg_source_mgr mSrcMgr;

    // Request pixel size and image size of the decompressed image.
    base::samples::frame::Frame frame_tmp;
    frame_tmp.frame_mode = decompress_to;
    frame_tmp.setDataDepth(8);
    int dst_pixel_size = frame_tmp.getPixelSize();
    size_t dst_size = width * height * dst_pixel_size;    
    LOG_DEBUG("Size of the output image %d (width %d, height %d, pixel-size %d", 
            dst_size, width, height, dst_pixel_size);

    /*
    // Resize output frame if necessary.
    // Resets the memory every time! How to avoid?
    if(!mFrameInitialized) {
        frame_output.init(frame_input.getWidth(), frame_input.getHeight(), 8, 
                decompress_to);
    }
    mFrameInitialized = false;
    */

    LOG_DEBUG("Set jpeg std error method");
    dinfo.err = jpeg_std_error(&jerr); // Has to be set before calling jpeg-create...
    LOG_DEBUG("Set decompress structure");
    jpeg_create_decompress(&dinfo);

    LOG_DEBUG("Define source (input frame)");
    // Define source (input frame).
    mSrcMgr.next_input_byte = src;
    mSrcMgr.bytes_in_buffer = src_size;
    LOG_DEBUG("Set source methods");
    // Set source methods.
    mSrcMgr.init_source = my_init_source; // ijg callback
    mSrcMgr.fill_input_buffer = my_fill_input_buffer; // ijg callback
    mSrcMgr.skip_input_data = my_skip_input_data; // ijg callback
    mSrcMgr.resync_to_restart = my_resync_to_restart; // ijg callback
    mSrcMgr.term_source = my_term_source; // ijg callback
    dinfo.src = &mSrcMgr;

    // Read file header, set default decompression parameters, image size...
    LOG_DEBUG("Read jpeg file header");
    (void) jpeg_read_header(&dinfo, TRUE);  

    LOG_DEBUG("Set output parameters: width %d, height %d, color-space %d, color_components %d", 
            width, height, j_color_space_output, dst_pixel_size);
    dinfo.output_width = width;
    dinfo.output_height = height;
    dinfo.out_color_space = j_color_space_output;
    dinfo.out_color_components = dst_pixel_size;
    //dinfo.output_components = 3; // What does this mean?

    // Color conversion JCS_GRAYSCALE to JCS_YCbCr is not supported by IJG
    if(dinfo.jpeg_color_space == JCS_GRAYSCALE &&
            dinfo.out_color_space == JCS_YCbCr) {
        LOG_ERROR("JCS_GRAYSCALE can not be decompressed to JCS_YCbCr");
        throw  std::runtime_error("JCS_GRAYSCALE can not be decompressed to JCS_YCbCr"); 
    }

    // Start decompressor.
    LOG_DEBUG("Start decompression");
    (void) jpeg_start_decompress(&dinfo);

    // Physical row width in output buffer.
    int row_stride =  width * dst_pixel_size;
    LOG_DEBUG("Row width in output buffer %d", row_stride);

    // Decompress jpeg row by row.
    unsigned char* pos = dst; // Write directly into output frame.
    unsigned char* pos_end = dst + dst_size;
    int scanlines_sum = 0;
    while (dinfo.output_scanline < dinfo.output_height) {
        // jpeg_read_scanlines expects an array of pointers to scanlines.
        // Here the array is only one element long, but you could ask for
        // more than one scanline at a time if that's more convenient.
        if(pos + row_stride <= pos_end) {
            scanlines_sum += jpeg_read_scanlines(&dinfo, &pos, 1);
            pos += row_stride;
        } else {
            throw  std::runtime_error("Output buffer full, stop decompressing");
        }
    }
    LOG_DEBUG("Scanlines read %d", scanlines_sum);

    // Finish decompression and release memory.
    LOG_DEBUG("Finish decompression");
    (void) jpeg_finish_decompress(&dinfo);
    jpeg_destroy_decompress(&dinfo);

    // Flip R and B if BGR should be created.
    LOG_DEBUG("Flip R and B if BGR should be created");
    if(output_flip_rgb) {
        switchRB(dst_size, dst);
    }
}

J_COLOR_SPACE JpegConversion::getJpegColorspace(base::samples::frame::frame_mode_t const frame_mode, 
            bool& flip_rgb, bool& is_jpeg) {
    using namespace base::samples::frame;
    switch(frame_mode) {
        case MODE_GRAYSCALE: {
            LOG_DEBUG("Return JCS_GRAYSCALE");
            return JCS_GRAYSCALE;
        }
        case MODE_RGB: {
            LOG_DEBUG("Return JCS_RGB");
            return JCS_RGB;
        }
        case MODE_BGR: {// Channels have to be flipped manually.
            LOG_DEBUG("Return JCS_RGB and activate flip-rgb");
            flip_rgb = true;
            return JCS_RGB;
        }
        case MODE_JPEG: {
            LOG_DEBUG("Return JCS_UNKNOWN and report a JPEG");
            is_jpeg = true;
            return JCS_UNKNOWN;
        }
        default: {
            LOG_DEBUG("Return JCS_UNKNOWN");
            return JCS_UNKNOWN;
        }
    }
}

bool JpegConversion::storeFrame(std::string filename,
        base::samples::frame::Frame const& frame, std::string* used_filename) {

    if(frame.getNumberOfBytes() == 0) {
        std::cerr << "Frame is empty." << std::endl;
        return false;
    }

    char header[128];
    int written = 0;
    bool is_jpeg = false;
    int width = frame.getWidth();
    int height = frame.getHeight();

    switch (frame.getFrameMode()) {
        case base::samples::frame::MODE_BGR:
        case base::samples::frame::MODE_RGB:
            filename += ".ppm";
            written = snprintf(header, 128, "%c%c%c%d %d\n%d\n", 'P', '3', '\n', width, height, 255);
            break;
        case base::samples::frame::MODE_GRAYSCALE:
            filename += ".pgm";
            written = snprintf(header, 128, "%c%c%c%d %d\n%d\n", 'P', '2', '\n', width, height, 255);
            break;
        case base::samples::frame::MODE_JPEG:
        case base::samples::frame::MODE_PJPG:
            filename += ".jpeg";
            is_jpeg = true;
            break;
        default: 
            LOG_ERROR("Frame color mode %d can not be written.", frame.getFrameMode());
            return false;
    }

    FILE* pFile  = fopen(filename.c_str(), "wb");
    if(pFile == NULL) {
        LOG_ERROR("File %s could not be opened", filename.c_str());
        return false;
    }

    // Pass back filename.
    if(used_filename != NULL)
        *used_filename = filename;

    // Write header.
    fwrite(header, 1, written, pFile); 

    unsigned int bytes_written = 0;
    if(is_jpeg) {
        bytes_written = fwrite(frame.getImageConstPtr(), 1, frame.getNumberOfBytes(), pFile);
    } else {
        unsigned char const* data = frame.getImageConstPtr();
        unsigned char const* end = frame.getLastConstByte() + 1;

        int counter = 0;
        //int mod_value = frame.getWidth() * frame.getPixelSize();
        for(;data < end; data++) {
            fprintf(pFile, "%d ", (int)*data);
            counter++;
        }
        bytes_written = counter;
    }

    if(bytes_written != frame.getNumberOfBytes()) {
        LOG_ERROR("Only %d of %d could be written", bytes_written, frame.getNumberOfBytes());
        return false;
    }

    LOG_INFO("%d bytes have been written", bytes_written);
    fclose(pFile);
    return true;
}

bool JpegConversion::loadJpeg(std::string const& filename, uint32_t const width, 
        uint32_t const height, base::samples::frame::Frame& frame) {

    // Filename contains jpg or jpeg?
    /*
    if(filename.find("jpeg") == std::string::npos &&
            filename.find("jpg") == std::string::npos &&
            filename.find("JPEG") == std::string::npos &&
            filename.find("JPG") == std::string::npos) {
        std::cerr << "No file loaded, seems not to be a jpeg file." << std::endl;
        return false;
    }
    */

    FILE* pFile  = fopen(filename.c_str(), "rb");
    if(pFile == NULL) {
        LOG_ERROR("File %s could not be opened", filename.c_str());
        return false;
    }

    // Obtain file size.
    fseek (pFile , 0 , SEEK_END);
    long jpeg_size = ftell (pFile);
    rewind (pFile);

    frame.init(width, height, 8, base::samples::frame::MODE_JPEG, 0, jpeg_size);
    
    // Copy the file into the frame buffer.
    long result = fread (frame.getImagePtr(), 1, jpeg_size, pFile);
    if (result != jpeg_size) {
        LOG_ERROR("Only %d of %d bytes could be read", result, jpeg_size);
        fclose(pFile);
        return false;
    }
    fclose(pFile);
    return true;
}

// PRIVATE
void JpegConversion::switchRB(uint32_t const img_size, uint8_t const* input_img, uint8_t* output_img) {
    if(input_img == NULL || output_img == NULL) {
        LOG_WARN("Input and/or output image are NULL, no channels flipped.");
        return;
    }

    uint8_t const* p_r = input_img;
    uint8_t const* p_g = input_img + 1;
    uint8_t const* p_b = input_img + 2;
    uint8_t const* p_last = input_img + img_size;

    for(; p_b < p_last; p_r += 3, p_g += 3, p_b += 3, output_img += 3) {
        *output_img = *p_b;
        *(output_img + 1) = *p_g;
        *(output_img + 2) = *p_r;
    } 
}

void JpegConversion::switchRB(uint32_t const img_size, uint8_t* input_img) {
    if(input_img == NULL) {
        LOG_WARN("Input image is NULL, no channels flipped.");
        return;
    }

    uint8_t* p_r = input_img;
    uint8_t* p_b = input_img + 2;
    uint8_t* p_last = input_img + img_size;
    uint8_t c_temp = 0;

    for(; p_b < p_last; p_r += 3, p_b += 3) {
        c_temp = *p_r;
        *p_r = *p_b;
        *p_b = c_temp;
    } 
}

} // end namespace conversion
