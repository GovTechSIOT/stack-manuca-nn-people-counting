# ifndef BASE_MODEL_H
# define BASE_MODEL_H

#include <cstdint>

class Base_Model {
    public:
        virtual void Initialize(void) = 0;
        virtual uint8_t* RunInference(uint8_t* input_buf) = 0;

};

# endif //BASE_MODEL_H