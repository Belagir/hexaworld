
#include <unstandard.h>

#include <math.h>

// -------------------------------------------------------------------------------------------------
vector_2d_cartesian_t vector2d_polar_to_cartesian(vector_2d_polar_t vec) {
    vector_2d_cartesian_t result = { 0u };

    result.v = cos(vec.angle) * vec.magnitude;
    result.w = sin(vec.angle) * vec.magnitude;

    return result;
}

// -------------------------------------------------------------------------------------------------
vector_2d_polar_t vector2d_cartesian_to_polar(vector_2d_cartesian_t vec) {
    vector_2d_polar_t result = { 0u };

    result.magnitude = sqrt(pow(vec.v, 2.0f) + pow(vec.w, 2.0f));
    result.angle = atan2f(vec.v, vec.w);

    return result;
}
