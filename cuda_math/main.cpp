#include <iostream>
#include "cuda_math.h"

int const NUM = 1280;
float const MIN = 0.0f;
float const MAX = 2.0f;
int const ULP = 1;
int main()
{
    // test_tanhf
    std::cout << "test_tanhf...\n";
    for (int i = 0; i < NUM; i++) {
        float ss = MIN + i * (MAX - MIN) / NUM;
        union32 udata = { 0 }, cdata = { 0 };
        udata.f32 = cuda_math::tanhf(ss);
        cdata.f32 = tanhf(ss);
        int ulp_diff = abs(udata.i32 - cdata.i32);
        if (ulp_diff > ULP)
            printf("cuda_math::tanhf(%e) = %e, tanhf(%e) = %e, ulpdiff = %d \n", ss, udata.f32, ss, cdata.f32, ulp_diff);
    }
    
    // test_erff
    std::cout << "test_erff...\n";
    for (int i = 0; i < NUM; i++) {
        float ss = MIN + i * (MAX - MIN) / NUM;
        union32 udata = { 0 }, cdata = { 0 };
        udata.f32 = cuda_math::erff(ss);
        cdata.f32 = erff(ss);
        int ulp_diff = abs(udata.i32 - cdata.i32);
        if (ulp_diff > ULP)
            printf("cuda_math::erff(%e) = %e, erff(%e) = %e, ulpdiff = %d \n", ss, udata.f32, ss, cdata.f32, ulp_diff);
    }
}