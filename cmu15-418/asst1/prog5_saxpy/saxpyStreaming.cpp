#include <immintrin.h>  // For AVX-512 intrinsics
#include <assert.h>
#include <stdint.h>

// Serial version of SAXPY (for comparison)
extern void saxpySerial(int N, float scale, float X[], float Y[], float result[]);

void saxpyStreaming(int N, float scale, float X[], float Y[], float result[]) {
    assert(N % 16 == 0);  // Ensure N is a multiple of 16 for AVX-512 alignment

    // Broadcast the scalar 'scale' to all elements of an AVX-512 register
    __m512 scalar = _mm512_set1_ps(scale);

    for (int i = 0; i < N; i += 16) {
        // Load 16 floats from X and Y into AVX-512 registers
        __m512 x = _mm512_loadu_ps(&X[i]);  // Unaligned load for safety
        __m512 y = _mm512_loadu_ps(&Y[i]);

        // Perform the SAXPY operation: result[i:i+15] = scale * X[i:i+15] + Y[i:i+15]
        __m512 product = _mm512_mul_ps(scalar, x);  // Multiply: scale * X
        __m512 sum = _mm512_add_ps(product, y);     // Add: (scale * X) + Y

        // Store the result back to memory using streaming store
        _mm512_stream_ps(&result[i], sum);
    }

    // Ensure all streaming stores are completed
    _mm_sfence();
}
