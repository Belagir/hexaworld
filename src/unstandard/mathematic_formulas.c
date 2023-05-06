
#include <unstandard.h>
#include <math.h>

#define PI 3.14159265359f

// -------------------------------------------------------------------------------------------------
f32 sigmoid(f32 x, f32 weight) {
    return 1.0f / (1.0f + expf(-x * weight)); 
}

// -------------------------------------------------------------------------------------------------
f32 normal_distribution(f32 x, f32 mean, f32 variance) {
    return (1.0f / (variance * sqrt(2.0f * (PI)))) * exp(-0.5f * powf((x - mean) / variance, 2.0f));
}
