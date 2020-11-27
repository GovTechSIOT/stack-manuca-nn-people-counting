# include "image/Pixel.h"

/*  @brief  Initialize a new Pixel instance. 
            Wrapper around static method of same name
    @author Daniel Tan
    @param  fmt:    A Pixel::Format instance
            bytes:  A pointer to a uint8_t[] array containing GetChannels() consecutive pixel bytes
 */
Pixel::Pixel(Format fmt, uint8_t* bytes) : format_(fmt) {
    for (size_t ch = 0; ch < this->GetChannels(); ch++) {
        this->bytes_[ch] = bytes[ch];
    }
};

/*  @brief  Get the number of channels of a Pixel() instance. 
            Wrapper around static method of same name
    @author Daniel Tan
    @param  void
    @return A size_t between [0, Pixel::MAX_PIXEL_BYTES]
 */
const size_t Pixel::GetChannels(void) const {
    return Pixel::GetChannels(this->format_);
}

/*  @brief  Static method to get the number of channels corresponding to a pixel format. 
    @author Daniel Tan
    @param  A Pixel::Format enum variable
    @return A size_t between [0, Pixel::MAX_PIXEL_BYTES]
 */
const size_t Pixel::GetChannels(Format fmt) {
    switch (fmt) {
        case GRAYSCALE: return 1;
        case RGB565: return 2;
        case RGB888: return 3;
    }
}

/*  @brief  Reformat a Pixel() to a different format. 
            See Pixel::Format for supported formats. 
    @author Daniel Tan
    @param  Pixel::Format, format to convert pixel to
    @return A Pixel() instance of the new format
 */

Pixel Pixel::Reformat(Format new_fmt) const {
    Format old_fmt = this->format_;
    if (old_fmt == new_fmt) {
        return Pixel(old_fmt, (uint8_t*)this->bytes_);
    }

    if (old_fmt == RGB888) {
        if (new_fmt == GRAYSCALE) {
            return this->ConvertRgb888ToGrayscale();
        } 
        else // new_fmt == RGB565
        {
            return this->ConvertRgb888ToRgb565();
        }
    }
    else if (old_fmt == RGB565) 
    {
        if (new_fmt == GRAYSCALE) {
            return this->ConvertRgb565ToRgb888().ConvertRgb888ToGrayscale();
        }
        else // new_fmt == RGB888
        { 
            return this->ConvertRgb565ToRgb888();
        }
    }
    else // old_fmt == GRAYSCALE
    {
        if (new_fmt == RGB888) {
            return this->ConvertGrayscaleToRgb888();
        }
        else // new_fmt == RGB565 
        {  
            return this->ConvertGrayscaleToRgb888().ConvertRgb888ToRgb565();
        }
    }
}

/*  @brief  Get the format of a Pixel() instance
    @author Daniel Tan
    @return A Pixel::Format enum variable
 */
Pixel::Format Pixel::GetFormat() const {
    return this->format_;
}

/*  @brief  Get the bytes corresponding to the pixel. 
    @author Daniel Tan
    @return Pointer to an array of uint8_t no longer than Pixel::MAX_PIXEL_BYTES
 */
uint8_t* Pixel::GetBytes() const {
    return (uint8_t*) this->bytes_;
}
        
/*  @brief  Convert a Pixel() from RGB888 to RGB565
    @author Daniel Tan
    @return A new Pixel() of type RGB565
 */
Pixel Pixel::ConvertRgb888ToRgb565(void) const {

    uint8_t red = this->bytes_[0];
    uint8_t green = this->bytes_[1];
    uint8_t blue = this->bytes_[2];

    // The high pixel should contain 5 largest bits of red and largest 3 bits of green.
    // The low pixel should contain 4th-6th bits of green and 5 largest bits of blue. 
    uint8_t pix_hi = (red & 0xf8); // 5 largest bits of red
    pix_hi = pix_hi | (green >> 5); // 3 largest bits of green
    uint8_t pix_lo = (green << 3) & 0xE0; // 4th-6th bits of green
    pix_lo = pix_lo | ((blue & 0xf8) >> 3); // 5 largest bits of blue

    // The bytes collectively represent a uint16_t t
    // This should be stored in little-endian format. 
    uint8_t new_bytes[MAX_PIXEL_BYTES];
    new_bytes[0] = pix_lo;
    new_bytes[1] = pix_hi;

    return Pixel(RGB565, new_bytes); 
};

/*  @brief  Convert a Pixel() from RGB565 to RGB888
    @author Daniel Tan
    @return A new Pixel() of format RGB888
 */
Pixel Pixel::ConvertRgb565ToRgb888(void) const {

    // Read in a pixel stored in little-endian format
    uint8_t pix_lo = this->bytes_[0];
    uint8_t pix_hi = this->bytes_[1];

    // convert RGB565 to RGB888
    uint8_t red = (0xF8 & pix_hi); 
    uint8_t green = ((0x07 & pix_hi)<<5) | ((0xE0 & pix_lo)>>3);
    uint8_t blue = (0x1F & pix_lo) << 3;

    uint8_t new_bytes[MAX_PIXEL_BYTES];
    new_bytes[0] = red;
    new_bytes[1] = green;
    new_bytes[2] = blue;

    return Pixel(RGB888, new_bytes);
};

/*  @brief  Convert a Pixel() from RGB888 to grayscale
    @author Daniel Tan
    @return A new Pixel() of format GRAYSCALE
 */
Pixel Pixel::ConvertRgb888ToGrayscale(void) const {

    uint8_t red = this->bytes_[0];
    uint8_t green = this->bytes_[1];
    uint8_t blue = this->bytes_[2];

    uint8_t new_bytes[MAX_PIXEL_BYTES];
    new_bytes[0] = (red / 3) + (green / 3) + (blue / 3); 
    return Pixel(GRAYSCALE, new_bytes);
};

/*  @brief  Convert a Pixel() from grayscale to RGB888
    @author Daniel Tan
    @return A new Pixel() of type grayscale
 */

Pixel Pixel::ConvertGrayscaleToRgb888(void) const {

    uint8_t gray = this->bytes_[0];
    uint8_t new_bytes[MAX_PIXEL_BYTES];
    new_bytes[0] = gray;
    new_bytes[1] = gray;
    new_bytes[2] = gray;

    return Pixel(RGB888, new_bytes);
};

/*  @brief  Compare two Pixels by value. 
    @author Daniel Tan
    @return True if all fields are equal, False otherwise
 */
bool Pixel::operator==(const Pixel& pixel) const {
    bool equal = (this->GetFormat() == pixel.GetFormat());
    for (size_t ch = 0; ch < this->GetChannels(); ch ++) {
        if (this->GetBytes()[ch] != pixel.GetBytes()[ch]) 
        {
            equal = false;
        };
    }
    return equal;
    
}