## Introduction
`stack-manuca-nn-people-counting` is a computer vision application written using the MANUCA OS tech stack. It uses an external camera module to capture images, performs machine-learning-based inference to count the number of people in the image, and then publishes the counts to the government-supported DECADA Cloud service. It is written in C++ for efficiency and is suitable for deployment on edge devices. 

For more information about MANUCA OS, click [here](https://siot.gov.sg/tech-stack/manuca/overview/). A detailed user manual is available [here](https://siot.gov.sg/files/MANUCA_User_Manual_V1.pdf). 

---
## Development Team
* Daniel Tan (dtch1997@stanford.edu)
* Lau Lee Hong (lau\_lee\_hong@tech.gov.sg)

---
## System Requirements for Development
 * Windows, MacOS or Ubuntu 18.04

---
## Hardware Requirements
 * ArduCAM 2MP Plus OV2640 camera module
 * [GovTech IoT Starter Kit](https://www.siot.gov.sg/starter-kit) or similar ARM Cortex-M7 boards

---
## Quick Start
 * Connect ArduCAM module to MANUCA OS board through the SPI2, I2C2 ports. 
 * Set up the development environment (https://siot.gov.sg/starter-kit/set-up-your-software-env/) 
 * Clone the repository onto local disk: 
    `git clone --recurse-submodules TODO_write_final_github_link`
 * Configure DECADA credentials (https://www.siot.gov.sg/starter-kit/build-and-flash-sw/#InputCredentials)
 * [Optionally] set trace level to debug in `mbed_app_.json`. ("mbed-trace.max-level" : ~~"TRACE_LEVEL_INFO"~~ "TRACE_LEVEL_DEBUG")
 * Compile the binary (https://siot.gov.sg/starter-kit/build-and-flash-sw/)
 * Copy the binary file (.bin) into the MANUCA DK via programmer (eg. stlink v3).

---
## Documentation

### Hardware Setup
The ArduCAM module must be connected to the MANUCA DK board by connecting the SPI and I2C interfaces. A detailed guide on how to connect the pins can be found [here](docs/Pins.md)

### Performing computer vision
stack-manuca-nn-people-counting uses a convolutional neural network (CNN) to perform inference. The CNN is trained in Python using Tensorflow. It takes an input of a 96 x 96 grayscale image and predicts a binary value of whether the image contains a person or not. The architecture used is a MobileNetV2 and the model is trained on the Visual Wake Words dataset, an open-source image benchmark for edge machine learning applications. In total, our model has 300kb of parameters (stored in flash) and requires an additional 100kb of static computation buffers (which require RAM). 

### TFLM Model abstraction
We provide a `sensors-lib/camera/model/TFLM_Model.cpp` class as a high-level, object-oriented wrapper aronud the base Tensorflow Lite for Microcontrollers C library. Using the neural network for prediction is as simple as `output_buffer = model.RunInference(input_buffer)`. 

### Image, Pixel abstraction
A small amount of image preprocessing is required to get the input images into the correct size and image format for the neural network. The Image and Pixel classes handle this resizing and reformatting in an extensible, object-oriented way. The documented source code is found in `sensors-lib/camera/image/Image.cpp` and `sensors-lib/camera/image/Pixel.cpp` respectively. 

### Person counting logic
The application divides an image into a grid of squares (with configurable length), uses some hand-written image processing code to resize each square to 96 x 96, and predicts whether that square contains a person or not. The total count prediction is the number of positive detections. 
 
---
## Extending the Code
### Using a different camera module
The stack-manuca-nn-people-counting code uses an ArduCAM 2MP Plus OV2640, but this can be replaced with any camera module that supports the SPI2 and I2C communication protocols. To use stack-manuca-nn-people-counting with a different camera module, you will need to write or install an appropriate driver for performing low-level read and write operations to the camera module. The stack-manuca-nn-people-counting driver code is found in `lib/ArduCAM/ArduCAM/` and is adapted from the [official implementation](https://github.com/ArduCAM/Arduino/tree/master/ArduCAM). It has been modified to work with Mbed instead of Arduino or ESP32. 

This driver can be interfaced with the core MANUCA OS code by writing a high-level wrapper that implements the SensorType interface found in `sensors-lib/sensor_type.h`. The SensorType subclass used by stack-manuca-nn-people-counting is found in `sensors_lib/camera/Ardu_Camera.h`. An example of how to use this subclass in the main application can be found in `threads/sensor_thread.cpp`. 

### Modifying default values
The application uses some default parameters for controlling computer vision. The parameters are hardcoded in `sensors-lib/camera/Ardu_Camera.h` as static class variables. 

- Sliding window length controls the size of the square. The default value of 80 is set to work with a 320x240 RGB QVGA camera image format. We recommend tuning this parameter for the specific deployment use-case so that each square fits exactly one person. 
- Tensor arena size determines how much memory is allocated for the intermediate computations used by the model. Allocating too little memory will result in a out-of-memory error. Detailed instructions for determining the required buffer size are included in the source code. 

### Using a different model
The model data is contained as a byte array `g_person_detect_model_data` in `sensors-lib/camera/model_data/person_detection_int8/model_data.cc`. To replace the model:

1. Create a folder (e.g. `my_folder`) in `sensors-lib/camera/model_data` 
2. Add the trained and exported Tensorflow model as a byte array (e.g `my_model_data`) in `sensors-lib/camera/model_data/my_folder/model_data.cc`. 
3. In `sensors-lib/camera/Ardu_Camera.cpp`, include the new model data and replace the old model data:
```
# include "camera/model_data/my_folder/model_data.h"  	// <--- This line was changed

Ardu_Camera::Ardu_Camera(PinName cam_cs, 
                PinName cam_spi_mosi, PinName cam_spi_miso, PinName cam_spi_sclk,
                PinName cam_i2c_data, PinName cam_i2c_sclk):
    image_(cam_img_height, cam_img_width, cam_img_fmt, camera_buf),   
    arducam_(cam_cs, 
        cam_spi_mosi, cam_spi_miso, cam_spi_sclk, 
        cam_i2c_data, cam_i2c_sclk, OV2640, RAW),
    model(my_model_data,  				// <--- This line was also changed
        tensor_arena_size, 
        tensor_arena,
        false)
```

If using a different model architecture, you may also need to configure `Ardu_Camera::model_arena_size`. Detailed instructions for determining the required buffer size are included in `sensors-lib/camera/Ardu_Camera.h`. 

The code to train a TensorFlow model in Python, quantize, and export it to a C format is available here: https://github.com/dtch1997/tf-detect

### Using a different approach
The approach used in this work differs from that used in mainstream crowd counting literature. We also attempted an object-detection based framework with a Mobilenet-SSD architecture. However, the architectures trained with that approach were either too large to fit on the microcontroller, or did not perform well at object detection. Nonetheless, it is possible that future innovations will make this approach more practical. We welcome third-party contributions in this regard. 

---
## License and Contributions
The software is provided under the Apache-2.0 license (see https://www.siot.gov.sg/starter-kit/terms-and-conditions/). Contributions to this project repository are accepted under the same license. Please see [contributing.md](CONTRIBUTING.md) for more information. 
