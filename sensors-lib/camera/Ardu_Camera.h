# ifndef ARDU_CAMERA_H
# define ARDU_CAMERA_H

#include "sensor_type.h"
#include "camera/image/Image.h"
#include "lib/ArduCAM/ArduCAM/ArduCAM.h" // base driver
# include "camera/model/TFLM_Model.h"

/** Create a Ardu_Camera object using the specified I2C object
 * @param sda - mbed I2C interface pin
 * @param scl - mbed I2C interface pin
 * @param I2C Frequency (in Hz)
 *
 * @return none
 */

class Ardu_Camera : public SensorType {
    public:
        Ardu_Camera(PinName cam_cs, 
                PinName cam_spi_mosi, PinName cam_spi_miso, PinName cam_spi_sclk,
                PinName cam_i2c_data, PinName cam_i2c_sclk);
        std::string GetName();
        int GetData(std::vector<std::pair<std::string, std::string>>&);
        void Enable();
        void Disable();
        // void Configure();   // To be done in SENP-286
        void Reset();

    private:
        ArduCAM arducam_;
        Image image_;
        void Initialize();
        void Capture();
        void ReadImage();

        /*  Sliding window length is set to 80 to fit with the common 320x240 QVGA image format.
            For best performance, tune it so that each square is big enough for exactly 1 person.
         */ 
        static constexpr int sliding_window_length = 80;
        /*  Sliding window frequency controls the interval at which inference is applied. 
            If this is smaller than sliding_window_length then there will be overlap in the
            inference squares. 
         */
        static constexpr int sliding_window_freq = sliding_window_length;

        /*  Model-specific parameters
            If you train a different neural network, these should be modified accordingly. 
         */ 
        static constexpr int cnn_img_height = 96;
        static constexpr int cnn_img_width = 96;
        static constexpr Pixel::Format cnn_img_fmt = Pixel::GRAYSCALE;
        static constexpr int cnn_channels = 1;


        /*  The model_arena_size given is for the default model. 
            In general this must be determined by trial and error because
            Tensorflow does not support compile-time introspection.  

            To calculate the arena size for a different model:
            
            1. Increase model_arena_size to an excess. 
            2. In mbed_app.json, configure "mbed-trace.max-level" to "TRACE_LEVEL_DEBUG"
            3. Compile and run the application on hardware. 
            4. TFLM_Model will print a debug statment, "__ bytes used for tensor arena". 
            5. The model arena size can be set to the printed value. 
         */
        static constexpr int model_arena_size = 109796;
        static constexpr int extra_arena_size = 500;
        static constexpr int tensor_arena_size = model_arena_size + extra_arena_size;
        alignas(16) uint8_t tensor_arena[tensor_arena_size];

        static constexpr size_t cam_img_height = 240;
        static constexpr size_t cam_img_width = 320;
        static constexpr size_t cam_channels = 2;
        static constexpr size_t img_buf_size = cnn_img_height * cnn_img_width * cam_channels;
        uint8_t img_buf_a[img_buf_size];
        uint8_t img_buf_b[img_buf_size];
        static constexpr Pixel::Format cam_img_fmt = Pixel::RGB565;
        uint8_t camera_buf[cam_img_height * cam_img_width * cam_channels];
        TFLM_Model model;
};

# endif // ARDUCAM_CAMERA_H