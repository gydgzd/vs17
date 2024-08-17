#pragma once
#include <stdint.h>
#include <math.h>

union union64 {
    double f64;
    int64_t i64;
    uint64_t u64;
    struct st64 {
        int lo32;
        int hi32;
    }st64;
};

union union32 {
    float f32;
    int  i32;
    uint32_t u32;
};

namespace cuda_math {

	float  logf(float x);
	double log(double x);

	float tanhf(float x);
    double tanh(double x);

    float erff(float x);
    double erf(double x);
}