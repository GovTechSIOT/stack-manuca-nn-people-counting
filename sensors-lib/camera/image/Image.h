# ifndef IMAGE_H
# define IMAGE_H

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>

#include "image/Pixel.h"
#include "mbed.h"

typedef struct {
    size_t height;
    size_t width;
    size_t channels;
    size_t num_bytes() const {
        return height * width * channels;
    }
} image_size_t;

/** Image class.
 *  @brief  Used to perform various image processing operations on raw byte data.
            GetBuffer() method provides access to the raw bytes. 
            This allows it to interface with the BSP_Camera and Tensorflow-Lite-Micro libraries.
 *
 *  Example:
 *  @code{.cpp}
 *  #include "mbed.h"
 *  #include "Image.h"
 *
 *  int main() 
 *  {
        constexpr size_t BUF_SIZE = 2048;
        static uint8_t img_buf_a[BUF_SIZE];
        static uint8_t img_buf_b[BUF_SIZE];

        // Create a 24 x 24 RGB image
        Image image = Image(24, 24, Pixel::RGB888, img_buf_a);
        assert (image.GetBufferSize() <= BUF_SIZE);

        // Write to img_buf_a to load an image

        // Flip the image vertically and horizontally
        image.VerticalFlip();
        image.HorizontalFlip();
        // Crop out the 12 x 12 center square
        Image image = image.Crop(6, 6, 12, 12, img_buf_b);

        // Preprocessed image bytes now live in img_buf_b
 *  }
 *  @endcode
 */

class Image {

    public:

        Image(size_t height, size_t width, Pixel::Format fmt, uint8_t* buf = nullptr);
        
        void ClearMemory(void);
        ~Image(void); 
        size_t GetHeight() const;
        size_t GetWidth() const;
        const Pixel::Format GetFormat() const;
        uint8_t* GetBuffer() const;
        size_t GetBufferSize() const;
        size_t GetChannels(void) const;
        const image_size_t GetSize(void) const;
        Pixel GetPixel(size_t row, size_t col) const;
        void SetPixel(size_t row, size_t col, Pixel pixel);
        Image Reformat(Pixel::Format new_fmt, uint8_t* buffer = nullptr) const;
        Image Crop(size_t top, size_t left, size_t height, size_t width, uint8_t* buffer = nullptr) const;
        Image Resize(size_t new_height, size_t new_width, uint8_t* buffer = nullptr) const;
        Image VerticalFlip(void);
        Image HorizontalFlip(void);

        std::string ToString() const;
        static Image FromString(std::string string);

    private:
        const size_t height_;
        const size_t width_;
        const Pixel::Format format_;
        uint8_t* buffer_;
        bool new_buffer_was_allocated_;

        size_t GetPixelLocation(size_t row, size_t col) const;
        Image ResizeNearest(size_t new_height, size_t new_width, uint8_t* buffer) const;
        void AllocateBuffer(uint8_t* buf = nullptr);
        void DeallocateBuffer(void);
};

# endif
