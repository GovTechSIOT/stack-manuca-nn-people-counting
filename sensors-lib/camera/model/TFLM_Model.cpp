#include "model/TFLM_Model.h"
#include "mbed_trace.h"

# define TRACE_GROUP "TFLM_Model.cpp"

/*  @brief  Allocates memory for the TFLM model and initializes various components.
    @author Daniel Tan
    @return Void;
    */
void TFLM_Model::Initialize() 
{
    // Set up logging. Google style is to avoid globals or statics because of
    // lifetime uncertainty, but since this has a trivial destructor it's okay.

    tr_debug("TFLM_Model::Initialize() called");
    tr_debug("Initializing error reporter");
    static tflite::MicroErrorReporter micro_error_reporter;
    this->error_reporter = &micro_error_reporter;
    
    if (verbose)
    {
        TF_LITE_REPORT_ERROR(error_reporter, "Error reporter initialized \n");
    }

    // Map the model into a usable data structure. This doesn't involve any
    // copying or parsing, it's a very lightweight operation.
    if (model->version() != TFLITE_SCHEMA_VERSION) {
        TF_LITE_REPORT_ERROR(error_reporter,
                            "Model provided is schema version %d not equal "
                            "to supported version %d\n.",
                            model->version(), TFLITE_SCHEMA_VERSION);
        return;
    }

    // This pulls in all the operation implementations we need.
    tr_debug("Initializing ops resolver");
    static tflite::AllOpsResolver resolver;
    if (verbose)
    {
        TF_LITE_REPORT_ERROR(error_reporter, "Resolver initialized \n");
    }

    // Build an interpreter to run the model with.
    tr_debug("Initializing static interpreter");
    static tflite::MicroInterpreter static_interpreter(
        model, resolver, tensor_arena, kTensorArenaSize, error_reporter);
    this->interpreter = &static_interpreter;
    if (verbose)
    {
        TF_LITE_REPORT_ERROR(error_reporter, "Interpreter initialized \n");
    }

    // Allocate memory from the tensor_arena for the model's tensors.
    tr_debug("Allocating tensor memory");
    TfLiteStatus allocate_status = interpreter->AllocateTensors();
    tr_debug("Memory allocation status: %d", allocate_status);
    if (allocate_status != kTfLiteOk) 
    {
        TF_LITE_REPORT_ERROR(error_reporter, "AllocateTensors() failed\n");
        return;
    }
    
    int arena_used_bytes = interpreter->arena_used_bytes();
    tr_debug("%d bytes used for tensor arena", arena_used_bytes);

    // For debug purposes, print how much memory the model needs
    if (verbose) 
    {
        TF_LITE_REPORT_ERROR(error_reporter, "Model requires %d bytes of tensor arena", arena_used_bytes);
    }

    if (arena_used_bytes > this->kTensorArenaSize)
    {
        TF_LITE_REPORT_ERROR(error_reporter, "Expected kTensorArenaSize >= %d; received %d\n",
            arena_used_bytes,
            this->kTensorArenaSize);
        return;
    }

    // Obtain pointers to the model's input and output tensors.
    input = interpreter->input(0);
    output = interpreter->output(0);

    // Log input shape. 
    TF_LITE_REPORT_ERROR(error_reporter, "Input shape is %d %d %d %d", 
        input->dims->data[0],
        input->dims->data[1],
        input->dims->data[2],
        input->dims->data[3]);

    // Keep track of how many inferences we have performed.
    inference_count = 0;
    tr_debug("TFLM_Model::Initialize() resolved");
}

/*  @brief  Helper function to get the number of bytes in a tensor. Assumes tensor is type uint8. 
    @author Daniel Tan
    @return Void;
    */
size_t compute_uint8_tensor_size(TfLiteTensor* tensor) 
{
    size_t num_dims = tensor->dims->size;
    size_t tensor_size = 1;
    for (size_t i = 0; i < num_dims; i++) 
    {
        tensor_size = tensor_size * tensor->dims->data[num_dims];
    }
    return tensor_size * sizeof(tensor->data.uint8[0]);
}

/*  @brief  Performs inference on data in input_buf and copies output to output_buf. 
    @author Daniel Tan
    @return Void;
    */
uint8_t* TFLM_Model::RunInference(uint8_t* input_buf)
{
    // Copy input buffer to model input. 
    if (verbose) 
    {
        TF_LITE_REPORT_ERROR(error_reporter, "Copying data to input tensor");
    }
    for (size_t i = 0; i < input->bytes; ++i) {
        input->data.uint8[i] = input_buf[i];
    }

    // Run inference, and report any error
    if (verbose) 
    {
        TF_LITE_REPORT_ERROR(error_reporter, "Invoking model on input tensor");
    }
    TfLiteStatus invoke_status = interpreter->Invoke();

    if (invoke_status != kTfLiteOk) 
    {
        TF_LITE_REPORT_ERROR(error_reporter, "Invoke failed. A total of %d successful inferences\n", inference_count);
        return nullptr;
    }
    inference_count++;

    return output->data.uint8;
}
