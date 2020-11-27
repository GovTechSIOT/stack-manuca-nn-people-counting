# include "Ardu_Camera.h"
# include "lib/ArduCAM/ArduCAM/ArduCAM.h" 
# include "mbed_trace.h"
# include "conversions.h"
# include "camera/model_data/person_detection_int8/model_data.h"

# define TRACE_GROUP "Ardu_Camera.cpp"

Ardu_Camera::Ardu_Camera(PinName cam_cs, 
                PinName cam_spi_mosi, PinName cam_spi_miso, PinName cam_spi_sclk,
                PinName cam_i2c_data, PinName cam_i2c_sclk):
    image_(cam_img_height, cam_img_width, cam_img_fmt, camera_buf),   
    arducam_(cam_cs, 
        cam_spi_mosi, cam_spi_miso, cam_spi_sclk, 
        cam_i2c_data, cam_i2c_sclk, OV2640, RAW),
    model(g_person_detect_model_data,
        tensor_arena_size, 
        tensor_arena,
        false) // verbose
{    
    tr_debug("Ardu_Camera::Ardu_Camera() called");
    Initialize();
};

std::string Ardu_Camera::GetName(){
    return "ArduCamera";
};

/*  @brief: Return True if all image pixels are black. 
            Used to sanity check the image. 
 */
bool is_all_black(const Image& image) {
    bool all_black = true;
    // Initialize a black pixel of same format as image
    uint8_t zero_bytes[Pixel::MAX_PIXEL_BYTES];
    for (size_t pos = 0; pos < Pixel::MAX_PIXEL_BYTES; pos++) {
        zero_bytes[pos] = 0;
    }
    Pixel black_pixel = Pixel(Pixel::RGB888, zero_bytes).Reformat(image.GetFormat());
    
    for (size_t row = 0; row < image.GetHeight(); row++) {
        for (size_t col = 0; col < image.GetWidth(); col++) {
            if (!(image.GetPixel(row, col) == black_pixel)) {
                all_black = false;
            } 
        }
    }

    return all_black;
};

int Ardu_Camera::GetData(std::vector<std::pair<std::string, std::string>>& data_list) {
    // TODO: Add checks for disconnect and camera type
    if (false) 
    {
        return DISCONNECT;
    }
    this->Capture();
    this->ReadImage();
    tr_debug("Image size: %d bytes", this->arducam_.read_fifo_length());

    // Sanity check the image
    if (is_all_black(this->image_)) 
    {
        tr_warn("Black image detected; camera may be faulty");
    } 
    else 
    {
        tr_debug("Valid image detected");
    }

    Image camera_image = this->image_;
    int num_people = 0;
    Watchdog &watchdog = Watchdog::get_instance();
    // Run inference on model
    for (size_t row = 0; row + sliding_window_length <= camera_image.GetHeight(); row += sliding_window_length) {
        for (size_t col = 0; col + sliding_window_length <= camera_image.GetWidth(); col += sliding_window_freq) {
            tr_debug("Running inference at (%d, %d)", row, col);
            Image cropped_image = camera_image.Crop(row, col, sliding_window_length, sliding_window_length, img_buf_a);
            Image formatted_image = cropped_image.Reformat(cnn_img_fmt, img_buf_b);
            Image resized_image = formatted_image.Resize(cnn_img_height, cnn_img_width, img_buf_a);
            uint8_t* output_buf = model.RunInference(resized_image.GetBuffer());

            // For the default model: 
            // - output_buf[0] is unused
            // - output_buf[1] corresponds to a score for a person
            // - output_buf[2] corresponds to the score for no person
            bool person_detected = output_buf[1] >= output_buf[2];
            if (person_detected) {
                num_people += 1;
            }
            // Kick the watchdog after every inference because application will time out otherwise
            watchdog.kick();
        }
    } 
    tr_info("%d people detected in total", num_people);
    data_list.push_back(std::make_pair("num_people_in_image", IntToString(num_people)));
    tr_debug("Payload value: %s", data_list[0].second.c_str());
    return DATA_OK;
}

void Ardu_Camera::Initialize() {
    tr_debug("Ardu_Camera::Initialize() called");
    arducam_.InitCAM();
    arducam_.clear_fifo_flag();
    arducam_.write_reg(ARDUCHIP_TIM, VSYNC_LEVEL_MASK);
    arducam_.write_reg(ARDUCHIP_FRAMES,0x00); 
    tr_debug("Initializing TFLM model...");
    this->model.Initialize();
    tr_debug("Ardu_Camera::Initialize() resolved");
}

/*  @brief: Signal the camera module to take a new image
    @author: Daniel Tan
 */
void Ardu_Camera::Capture() {
    // Clear flag to allow next capture to proceed
    arducam_.clear_fifo_flag();
    arducam_.start_capture();
    // Block thread until capture finishes
    while (!(arducam_.get_bit(ARDUCHIP_TRIG, CAP_DONE_MASK))){
      ThisThread::sleep_for(50);
    }
    return;
}

void Ardu_Camera::Enable() {
    return;
}

void Ardu_Camera::Disable() {
    return;
}

void Ardu_Camera::Reset() {
    return;
}

/*  @brief: Copy the captured image into in-memory buffer
 */
void Ardu_Camera::ReadImage() {
    arducam_.flush_fifo();
    uint8_t bytes[Pixel::MAX_PIXEL_BYTES];
    for (size_t row = 0; row < this->image_.GetHeight(); row++) {
        for (size_t col = 0; col < this->image_.GetWidth(); col++) {
            for (size_t ch = 0; ch < this->image_.GetChannels(); ch++) {
                bytes[ch] = arducam_.read_fifo();
            }
            Pixel pixel(this->image_.GetFormat(), bytes);
            this->image_.SetPixel(row, col, pixel);
        }
    }
    return;
}