# ifndef PIXEL_H
# define PIXEL_H

# include <cstdint>
# include <cstddef>
# include <cassert>

//Remark:   Different pixel formats might be better implemented as subclasses of a base Pixel class. 
//          This may become more ideal if more pixel formats are supported in the future. 
            
/** Pixel class.
 *  @brief  Used to describe pixel formats of raw uint8_t* buffers and convert between formats.
            It is intended as a low-level API to be used by the Image.cpp class. 
 *
 *  Example:
 *  @code{.cpp}
 *  #include "mbed.h"
 *  #include "Pixel.h"
 *
 *  int main() 
 *  {
 *      uint8_t pixel_bytes[4] = {224, 224, 224, 0}; 
        Pixel rgb888_pixel = Pixel(Pixel::RGB888, pixel_bytes);
        Pixel rgb565_pixel = rgb888_pixel.Reformat(Pixel::RGB565);
 *  }
 *  @endcode
 */
class Pixel {    
    public: 
        enum Format {GRAYSCALE, RGB565, RGB888};
        static const size_t MAX_PIXEL_BYTES = 4;
        Format format_;
        uint8_t bytes_[MAX_PIXEL_BYTES];    

        Pixel(Format fmt, uint8_t* bytes);
        static const size_t GetChannels(Format fmt);
        const size_t GetChannels(void) const;
        Pixel Reformat(Format new_fmt) const;
        Format GetFormat() const;
        uint8_t* GetBytes() const;
        bool operator==(const Pixel& pixel) const;

    private: 
        Pixel ConvertRgb888ToRgb565(void) const;
        Pixel ConvertRgb565ToRgb888(void) const;
        Pixel ConvertRgb888ToGrayscale(void) const;
        Pixel ConvertGrayscaleToRgb888(void) const;
};

# endif // PIXEL_H