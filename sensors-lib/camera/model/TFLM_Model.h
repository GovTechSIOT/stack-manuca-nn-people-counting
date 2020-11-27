# ifndef TFLM_MODEL_H
# define TFLM_MODEL_H

#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/version.h"
#include "tensorflow/lite/micro/testing/micro_test.h"

#include "model/Base_Model.h"

class TFLM_Model : Base_Model
{
    public:
        TFLM_Model( const unsigned char* model_data, 
                    const size_t kTensorArenaSize,
                    uint8_t* tensor_arena,
                    bool verbose = false):
            kTensorArenaSize(kTensorArenaSize),
            model(tflite::GetModel(model_data)),
            tensor_arena(tensor_arena),
            verbose(verbose)
        {
            this->error_reporter = nullptr;
            this->interpreter = nullptr;
            this->input = nullptr;
            this->output = nullptr;
            this->inference_count = 0;
        };
        ~TFLM_Model() {
            ClearMemory();
        }

        /* @brief: Delete all objects created with 'new'
         */
        void ClearMemory(void) 
        {

        };

        void Initialize(void);
        uint8_t* RunInference(uint8_t* input_buf);

    private:
        tflite::ErrorReporter* error_reporter;
        const tflite::Model* model;
        tflite::MicroInterpreter* interpreter;
        TfLiteTensor* input;
        TfLiteTensor* output;
        int inference_count;

        // Create an area of memory to use for input, output, and intermediate arrays.
        // Minimum arena size, at the time of writing. After allocating tensors
        // you can retrieve this value by invoking interpreter.arena_used_bytes().
        // Extra headroom for model + alignment + future interpreter changes.
        const size_t kTensorArenaSize;
        uint8_t* tensor_arena;
        const bool verbose;
};

#endif //TFLM_MODEL_H