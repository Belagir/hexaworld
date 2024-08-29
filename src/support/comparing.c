
#include <unstandard.h>

#include <stdlib.h>

// -------------------------------------------------------------------------------------------------
u32 float_equal(f32 val1, f32 val2, u32 max_ulps_diff) {
    u32 difference = 0u;

    i32 repr_val1 = *((i32*) &val1);
    i32 repr_val2 = *((i32*) &val2);

    if ((repr_val1 < 0) != (repr_val2 < 0)) {
        // direct comparison for +0.0f and -0.0f
        return (val1 == val2);
    }

    return (abs(repr_val1 - repr_val2) <= max_ulps_diff);
}
