
#include <unstandard.h>

// -------------------------------------------------------------------------------------------------
void bytewise_copy(void *dest, void *source, size_t nb_bytes) {
    char *byte_dest = (char *) dest;
    char *byte_source = (char *) source;
    char *byte_end = ((char *) source) + nb_bytes;

    while (byte_source != byte_end) {
        *(byte_dest++) = *(byte_source++);
    }
}

// -------------------------------------------------------------------------------------------------
u8 count_set_bits(u8 value)  {
    u8 counter = 0u;

    while (value) {
        counter += 1u;
        value &= value - 1u;
    }

    return counter;
}
