#include <iostream>
#include "cuda_math.h"

int const NUM = 1280000;
float const MIN = 0.0f;
float const MAX = 10.0f;
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
        if (ulp_diff > 2)
            printf("cuda_math::tanhf(%e) = %e, tanhf(%e) = %e, ulpdiff = %d \n", ss, udata.f32, ss, cdata.f32, ulp_diff);
    }

    // test_erff
    std::cout << "test_erff...\n";
    for (int i = 0; i < NUM; i++) {
        float ss = MIN + i * (MAX - MIN) / NUM;
        union32 udata = { 0 }, cdata = { 0 };
        udata.f32 = cuda_math::erff(ss);
        cdata.f32 = (float)erf((double)ss);
        int ulp_diff = abs(udata.i32 - cdata.i32);
        if (ulp_diff > 1)
            printf("cuda_math::erff(%e) = %e, erff(%e) = %e, ulpdiff = %d \n", ss, udata.f32, ss, cdata.f32, ulp_diff);
    }
    // test_expf
    std::cout << "test_expf...\n";
    for (int i = 0; i < NUM; i++) {
        float ss = MIN + i * (MAX - MIN) / NUM;
        union32 udata = { 0 }, cdata = { 0 };
        udata.f32 = cuda_math::expf(ss);
        cdata.f32 = exp(ss);
        int ulp_diff = abs(udata.i32 - cdata.i32);
        if (ulp_diff > 2)
            printf("cuda_math::expf(%e) = %e, expf(%e) = %e, ulpdiff = %d \n", ss, udata.f32, ss, cdata.f32, ulp_diff);
    }

    // compare boundary
    float boundary[16] = { -INFINITY, -88.8f, -1.0f, -1e-10f, -0.0f, 0.0f, 1e-10f, 1.0f, 88.8f, INFINITY, NAN, 2.66e-39f };
    printf("Input:   ");
    for (int i = 0; i < 12; i++) {
        printf("%8.2e  ", boundary[i]);
    }
    // tanhf
    printf("\ncuda_tanhf: ");
    for (int i = 0; i < 12; i++) {
        float mydata = cuda_math::tanhf(boundary[i]);
        printf("%8.2e  ", mydata);
    }
    printf("\n   c tanhf: ");
    for (int i = 0; i < 12; i++) {
        float cdata = tanhf(boundary[i]);
        printf("%8.2e  ", cdata);
    }
    printf("\n");
    // erf
    printf("\n cuda_erf: ");
    for (int i = 0; i < 12; i++) {
        double mydata = cuda_math::erf(boundary[i]);
        printf("%8.2e  ", mydata);
    }
    printf("\n    c erf: ");
    for (int i = 0; i < 12; i++) {
        float cdata = erf(boundary[i]);
        printf("%8.2e  ", cdata);
    }
    printf("\n");
    // erff
    printf("\ncuda_erff: ");
    for (int i = 0; i < 12; i++) {
        float mydata = cuda_math::erff(boundary[i]);
        printf("%8.2e  ", mydata);
    }
    printf("\n   c erff: ");
    for (int i = 0; i < 12; i++) {
        float cdata = erff(boundary[i]);
        printf("%8.2e  ", cdata);
    }
    printf("\n");
    // log
    printf("\n cuda_log: ");
    for (int i = 0; i < 12; i++) {
        double mydata = cuda_math::log(boundary[i]);
        printf("%8.2e  ", mydata);
    }
    printf("\n    c log: ");
    for (int i = 0; i < 12; i++) {
        float cdata = log(boundary[i]);
        printf("%8.2e  ", cdata);
    }
    printf("\n");
    // logf
    printf("\ncuda_logf: ");
    for (int i = 0; i < 12; i++) {
        float mydata = cuda_math::logf(boundary[i]);
        printf("%8.2e  ", mydata);
    }
    printf("\n   c logf: ");
    for (int i = 0; i < 12; i++) {
        float cdata = logf(boundary[i]);
        printf("%8.2e  ", cdata);
    }
    printf("\n");
    // expf
    printf("\ncuda_expf: ");
    for (int i = 0; i < 12; i++) {
        float mydata = cuda_math::expf(boundary[i]);
        printf("%8.2e  ", mydata);
    }
    printf("\n   c expf: ");
    for (int i = 0; i < 12; i++) {
        float cdata = expf(boundary[i]);
        printf("%8.2e  ", cdata);
    }
    printf("\n");
}