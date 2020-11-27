# include "camera/image/Image.h"
# include <cassert>
# include <sstream>
# include <iomanip>

/*  @brief  Initialize a new Image() instance. 
    @author Daniel Tan
    @param  height: Number of rows in the image. 
            width:  Number of cols in the image.
            fmt:    A Pixel::Format instance. 
            buf:    A uint8_t[] buffer containing the raw image bytes. 
                    See Image.AllocateBuffer() for usage. 

 */
Image::Image(size_t height, size_t width, Pixel::Format fmt, uint8_t* buf):
    height_(height),
    width_(width),
    format_(fmt) 
{
    AllocateBuffer(buf);
};

/*  @brief  Allocate a uint8_t buffer if necessary.
            If buffer was allocated, set a tracking flag so that it can be deallocated later. 
    @author Daniel Tan
    @param  buf:    If it is nullptr, a correctly-sized uint8_t[] array will be allocated on the heap. 
                    Otherwise, it should be a pointer to a statically allocated uint8_t[] array.
                    Image class assumes that size of the provided buffer is at least Image.GetBufferSize().
                    A too-small buffer may result in SEGFAULT errors due to illegal memory access.
                    
 */
void Image::AllocateBuffer(uint8_t* buf) {
    if (buf == nullptr) 
    {
        new_buffer_was_allocated_ = true;
        buffer_ = new uint8_t[this->GetBufferSize()];
    }
    else 
    {
        new_buffer_was_allocated_ = false;
        buffer_ = buf;
    }
}

/*  @brief  Deallocate the internal buffer if necessary
            Tracking flag prevents delete[]-ing a statically allocated array
    @author Daniel Tan
 */
void Image::DeallocateBuffer(void) {
    if (new_buffer_was_allocated_) 
    {
        delete[] buffer_;
    }
}

/*  @brief  Clear the memory used by the image, e.g. internal buffer if it was dynamically allocated
    @author Daniel Tan
 */
void Image::ClearMemory(void)
{
    this->DeallocateBuffer();
}

/*  @brief  Overload the destructor to clear memory if necessary. 
    @author Daniel Tan
 */
Image::~Image(void) 
{
    ClearMemory();
}


/*  @brief  Get number of rows in the image. 
    @author Daniel Tan
 */
    
size_t Image::GetHeight() const 
{
    return this->height_;
}

/*  @brief  Get number of columns in the image. 
    @author Daniel Tan
 */
size_t Image::GetWidth() const 
{
    return this->width_;    
}

/*  @brief  Get the image format. 
    @author Daniel Tan
    @return A Pixel::Format enum variable. 
 */
const Pixel::Format Image::GetFormat() const {
    return this->format_;
}

/*  @brief  Get the pointer to the buffer used to store the image bytes.
    @author Daniel Tan
    @return A pointer to a uint8_t[] array. 
 */
uint8_t* Image::GetBuffer() const {
    return buffer_;
}

/*  @brief  Get the number of bytes needed to contain all the image data.
            Number of bytes is inferred from image height, width, and format. 
    @author Daniel Tan
 */
size_t Image::GetBufferSize() const {
    return this->GetSize().num_bytes();
}

/*  @brief  Get the number of channels in each pixel of the image. 
            See Pixel::GetChannels() for details.
    @author Daniel Tan
 */
size_t Image::GetChannels() const {
    //serial_->printf("GetChannels() called \n");
    return Pixel::GetChannels(this->GetFormat());
}

/*  @brief  Get a struct describing the image size information. 
    @author Daniel Tan
    @return An image_size_t containing image size information. 
            See Image.h for definition of image_size_t. 
 */
const image_size_t Image::GetSize() const {
    image_size_t sz = {this->GetHeight(), this->GetWidth(), this->GetChannels()};
    return sz;
}

/*  @brief  Convert a (row, col) index into an index into the internal buffer used to store the image. 
    @author Daniel Tan
    @return A pointer into the internal uint8_t[] buffer. 
            Pointer points to the bytes corresponding to the pixel at (row, col). 
 */
size_t Image::GetPixelLocation(size_t row, size_t col) const {
    // Assume that pixels are indexed in row-major order. 
    // E.g. in a 3x3 image, row 0 contains pixels 0-2, row 1 contains pixels 3-5, etc. 

    // TODO: Support column-major order or other such orderings. 
    return (row * this->GetWidth() + col) * this->GetChannels();
}

/*  @brief  Get the pixel at (row, col)
    @author Daniel Tan
    @return A Pixel() instance
 */
Pixel Image::GetPixel(size_t row, size_t col) const {
    size_t pix_loc = this->GetPixelLocation(row, col);
    uint8_t* pixel_bytes = this->buffer_ + pix_loc;
    return Pixel(this->GetFormat(), pixel_bytes);
}

/*  @brief  Set the pixel at (row, col).
            Will throw an error if given pixel format is not the same as the image format. 
    @author Daniel Tan
    @param  A Pixel() instance with the same format as the image. 
 */
void Image::SetPixel(size_t row, size_t col, Pixel pixel) {
    //serial_->printf("SetPixel called \n");
    //serial_->printf("Arguments valid \n");
    size_t channels = this->GetChannels();
    size_t pix_loc = this->GetPixelLocation(row, col);
    for (size_t ch= 0; ch < channels; ch++) {
        this->buffer_[pix_loc + ch] = pixel.bytes_[ch];
    }
    return;
}

/*  @brief  Convert image to new format. 
            A statically allocated buffer can optionally be provided for the new image.
            Otherwise, a new buffer will be dynamically allocated. 
            See Pixel.h for the supported formats. 
    @author Daniel Tan
    @param  new_fmt:A Pixel::Format enum variable.
            buffer: See Image.AllocateBuffer() for usage. 
    @return A new Image() instance of the specified format. 
 */
Image Image::Reformat(Pixel::Format new_fmt, uint8_t* buffer) const {
    Image new_img = Image(this->GetHeight(), this->GetWidth(), new_fmt, buffer);

    for (size_t row = 0; row < this->GetHeight(); row++) {
        for (size_t col = 0; col < this->GetWidth(); col++) {
            Pixel old_pixel = this->GetPixel(row, col);
            Pixel new_pixel = old_pixel.Reformat(new_fmt);
            new_img.SetPixel(row, col, new_pixel);
        }
    }

    return new_img;
}

/*  @brief  Crop a rectangular subset of an image. 
            A statically allocated buffer can optionally be provided for the new image.
            Otherwise, a new buffer will be dynamically allocated. 
    @author Daniel Tan
    @param  top:    First row of the new image (0-indexed)
            left:   First col of the new image (0-indexed)
            height: Height of the new image. Rows from (row) to (row + height - 1) will be included.
            width:  Width of the new image. Cols from (col) to (col + width - 1) will be included.
            buffer: See Image.AllocateBuffer() for usage. 
    @return A new Image() instance containing the cropped image. 
 */
Image Image::Crop(size_t top, size_t left, size_t height, size_t width, uint8_t* buffer) const {
    image_size_t new_sz = {height, width, this->GetChannels()};
    Pixel::Format new_fmt = this->GetFormat();
    Image new_img = Image(height, width, new_fmt, buffer);

    for (size_t row = top; row < top+height; row++) {
        for (size_t col = left; col < left+width; col++) {
            Pixel pixel = this->GetPixel(row, col);
            new_img.SetPixel(row-top, col-left, pixel);
        }
    }

    return new_img;
}

/*  @brief  Helper function for bilinear interpolation. 
            Calculates a pixel value in the new image as 
            a weighted average of four pixel values in the original image. 
    @author Daniel Tan
    @return New pixel value. 
 */
float bilinear_interpolation(
                            float top_offset,
                            float left_offset,  
                            float top_left_val, 
                            float top_right_val,
                            float bottom_left_val,
                            float bottom_right_val) {
    return 
        top_left_val    *   (1 - left_offset)   * (1 - top_offset) + 
        top_right_val   *   left_offset         * (1 - top_offset) +
        bottom_left_val *   (1 - left_offset)   * (top_offset) +
        bottom_right_val *  left_offset         * top_offset;
}

/*  @brief  Resize an image using bilinear interpolation
    @author Daniel Tan
    @param  new_height: Height of new image
            new_width:  Width of new image
            buffer:     See Image.AllocateBuffer() for usage. 
    @return A new Image() instance
 */
Image Image::Resize(size_t new_height, size_t new_width, uint8_t* buffer) const {
    Image new_img = Image(new_height, new_width, this->GetFormat(), buffer);
    for (size_t row = 0; row < new_height; row++) {
        for (size_t col = 0; col < new_width; col++) {
            size_t orig_row = (row * this->GetHeight()) / new_height;
            size_t orig_col = (col * this->GetWidth()) / new_width;

            Pixel top_left_pixel = this->GetPixel(orig_row, orig_col); 
            Pixel top_right_pixel = this->GetPixel(orig_row, orig_col+1); 
            Pixel bottom_left_pixel = this->GetPixel(orig_row+1, orig_col); 
            Pixel bottom_right_pixel = this->GetPixel(orig_row+1, orig_col+1); 
            
            uint8_t new_pixel_bytes[Pixel::MAX_PIXEL_BYTES];
            for (size_t ch = 0; ch < this->GetChannels(); ch ++) {
                float top_offset = ((float) (row * this->GetHeight()) / (float) new_height) - float(orig_row);
                float left_offset = ((float) (row * this->GetWidth()) / (float) new_width) - float(orig_row);
                new_pixel_bytes[ch] = (uint8_t) bilinear_interpolation(
                    top_offset,
                    left_offset,
                    (float) top_left_pixel.bytes_[ch],
                    (float) top_right_pixel.bytes_[ch],
                    (float) bottom_left_pixel.bytes_[ch],
                    (float) bottom_right_pixel.bytes_[ch]
                );
            }
            Pixel new_pixel = Pixel(this->GetFormat(), new_pixel_bytes);
            new_img.SetPixel(row, col, new_pixel);
        }
    }
    return new_img;
}

/*  @brief  Flip the image horizontally. 
            Swaps pixel at (row, col) with pixel at (row, WIDTH-col). 
            Modifies Image in place to avoid need for another buffer. 
    @author Daniel Tan
    @return Pointer to self. 
 */
Image Image::HorizontalFlip(void) {
    for (size_t row = 0; row < this->GetHeight(); row ++) {
        // Iterate column up to WIDTH / 2, to avoid swapping the same pair two times.
        // If WIDTH = 2k, the col pairs are (0, 2k-1), ... , (k - 1, k)
        // If WIDTH = 2k+1, the col pairs are (0, 2k), ... , (k-1, k+1)
        for (size_t col = 0; col < this->GetWidth() / 2; col ++) {
            // Swap the two corresponding pixels
            Pixel pixel1 = this->GetPixel(row, col);
            Pixel pixel2 = this->GetPixel(row, this->GetWidth() - col);
            this->SetPixel(row, this->GetWidth() - col, pixel1);
            this->SetPixel(row, col, pixel2);
        }
    }
    return *this;
}

/*  @brief  Flip the image horizontally. 
            Swaps pixel at (row, col) with pixel at (HEIGHT - row, col). 
            Modifies Image in place to avoid need for another buffer. 
    @author Daniel Tan
    @return Pointer to self. 
 */
Image Image::VerticalFlip(void) {
    // Iterate row up to HEIGHT / 2, to avoid swapping the same pair two times.
    // If HEIGHT = 2k, the row pairs are (0, 2k-1), ... , (k - 1, k)
    // If HEIGHT = 2k+1, the row pairs are (0, 2k), ... , (k-1, k+1)
    for (size_t row = 0; row < this->GetHeight() / 2; row ++) {
        for (size_t col = 0; col < this->GetWidth(); col ++) {
            Pixel pixel1 = this->GetPixel(row, col);
            Pixel pixel2 = this->GetPixel(this->GetHeight() - row, col);
            this->SetPixel(this->GetHeight() - row, col, pixel1);
            this->SetPixel(row, col, pixel2);
        }
    }
    return *this;
}

/*  @brief  Resize an image by selecting the nearest pixel. 
            This is expected to be worse than the default method of bilinear interpolation but is included for completeness
    @author Daniel Tan
    @param  new_height: Height of new image
            new_width:  Width of new image
            buffer:     See Image.AllocateBuffer() for usage. 
    @return A new Image() instance
 */
Image Image::ResizeNearest(size_t new_height, size_t new_width, uint8_t* buffer) const {
    Image new_img = Image(new_height, new_width, this->GetFormat(), buffer);

    for (size_t row = 0; row < new_height; row++) {
        for (size_t col = 0; col < new_width; col++) {
            uint8_t pixel_bytes[this->GetChannels()];
            size_t orig_nearest_row = (row * this->GetHeight()) / new_height;
            size_t orig_nearest_col = (row * this->GetWidth()) / new_width;
            Pixel pixel = this->GetPixel(orig_nearest_row, orig_nearest_col); 
            new_img.SetPixel(row, col, pixel);
        }
    }

    return new_img;
}

/*  @brief  Convert an Image to a std::string
            String consists of image bytes in sequence. 
            Does not contain information about image height, width, or format. 
            This enables an Image to be returned as a value by a SensorType subclass

    @author Daniel Tan
 */

std::string Image::ToString() const {
    std::stringstream strbuf;
    // Height encoded as a 16-bit integer 
    strbuf << "height:"
        << std::setfill('0') 
        << std::setw(16)
        << std::to_string(this->GetHeight());
    // Width encoded as an 16-bit integer
    strbuf << "width:" 
        << std::setfill('0') 
        << std::setw(16)
        << std::to_string(this->GetHeight());

    // Format encoded as an 8-bit integer
    strbuf << "format:"
        << std::setfill('0')
        << std::setw(8)
        << std::to_string(this->GetFormat());

    for (size_t i = 0; i < this->GetBufferSize(); i++) {
        strbuf << this->GetBuffer()[i];
    }
    return strbuf.str();
};


/*  @brief  Convert a string back to an Image
            User must already know the image height, width, and format. 
            Inverse of Image::ToString
    @author Daniel Tan

static const Image Image::FromString(std::string str) {

    for (size_t i = 0; i < this->GetBufferSize(); i ++) {
        this->buffer_[i] = string[i];
    }
    return;
}
 */
