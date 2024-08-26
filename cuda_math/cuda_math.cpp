// cuda_math.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
#include "cuda_math.h"
#include <cmath>    // std::fma


namespace cuda_math {

    float logf(float x) {
     /* */ 
        union32 r0 = {.f32 = x};
        bool p0 = r0.f32 >= 1.175494350822287508e-38f;
        if (!p0)   r0.f32 = r0.f32 * 8388608;
        union32 r4 = { .u32 = r0.u32 - 0x3f2aaaab };
        union32 r5 = { .u32 = r4.u32 & 0xff800000 };
        r4.i32 = r0.i32 - r5.i32;
        float r6 = r4.f32 - 1.0f;
        r4.i32 = p0 ? 0 : 23;
        float r7 = 0.130188569f;
        r7 = r6 * (-1 * r7) + 0.14084610342979431152f;
        r7 = r6 * r7 + -0.12148627638816833496f;
        r7 = r6 * r7 + 0.13980610668659210205f;
        r7 = r6 * r7 + -0.16684235632419586182f;
        r7 = r6 * r7 + 0.20012299716472625732f;
        r7 = r6 * r7 + -0.24999669194221496582f;
        r7 = r6 * r7 + 0.33333182334899902344f;
        r7 = r6 * r7 + -0.5f;
        r7 = r6 * r7;
        r7 = r6 * r7 + r6;
        r4.f32 = r5.i32 * 1.1920928955078125e-07f + r4.f32;
        r4.f32 = r4.f32 * 0.69314718246459960938f + r7;
        bool p1 = (r0.u32 >= 2139095040);
        if (p1) {
            float r9 = INFINITY;
            r4.f32 = r0.f32 * r9 + INFINITY;
        }
        p0 = r0.f32 != 0;
        r5.f32 = p0 ? r4.f32 : -INFINITY;
        return r5.f32;
       
        /*
        union32 min = { .u32 = 0x00800000 };
        union32 c1 = { .u32 = 0x4B000000 };
        union32 c2 = { .u32 = 0xC1B80000 };
        union32 c3 = { .u32 = 0x00000000 };
        union32 f1, f3, f4;
        f1.f32 = x < min.f32 ? x * c1.f32 : x;
        float f7 = x < min.f32 ? c2.f32 : c3.f32;
        f3.i32 = (f1.i32 - 1059760811) & -8388608;
        f4.i32 = f1.i32 - f3.i32;                 

        float f11 = f3.i32 * 1.19209289e-7f + f7;
        float f12 = f4.f32 - 1.0f;
        float f15 = -0.130188569f * f12 + 0.1408461f;
        float f17 = f15 * f12 + -0.121486276f;
        float f19 = f17 * f12 + 0.1398061f;
        float f21 = f19 * f12 + -0.16684235f;
        float f23 = f21 * f12 + 0.20012299f;
        float f25 = f23 * f12 + -0.24999669f;
        float f27 = f25 * f12 + 0.33333182f;
        float f29 = f27 * f12 + -0.5f;
        float f30 = f29 * f12;
        float f31 = f30 * f12 + f12;
        float f35 = f11 * 0.69314718f + f31;
        if (f1.u32 >= 2139095040) {
            float f33 = INFINITY;
            f35 = f1.f32 * f33 + f33;
        }

        bool p3 = f1.f32 == 0;
        float f34 = p3 ? -INFINITY : f35;
        return f34;
        */
    }

    double log(double x) {
        union64 fd56 = { .f64 = x };
        int r24 = fd56.st64.hi32;
        int r25 = fd56.st64.lo32;

        bool p1 = r24 > 1048575; // 0xfffff
        uint32_t r26 = -1023;
        if (!p1) {                // handle subnormal
            fd56.f64 = fd56.f64 * 18014398509481984.0;  // 2^54
            r24 = fd56.st64.hi32;
            r25 = fd56.st64.lo32;
            r26 = -1077;
        }

        int r13 = r24 - 1;
        bool p2 = (uint32_t)r13 < 2146435071;
        int r27 = 0;
        union64 fd57;
        if (p2) {
            int r15 = (uint32_t)r24 >> 20;
            r27 = r26 + r15;
            int r16 = r24 & -2146435073;
            int r17 = r16 | 1072693248;
            fd57.st64.hi32 = r17;
            fd57.st64.lo32 = r25;

            bool p4 = r17 < 1073127583;
            if (!p4) {
                int r18 = fd57.st64.lo32;
                int r19 = fd57.st64.hi32;
                int r20 = r19 - 1048576;
                fd57.st64.lo32 = r18;
                fd57.st64.hi32 = r20;
                r27 = r27 + 1;
            }
        }
        else {
            double fd10 = INFINITY;
            double fd11 = fd56.f64 * fd10 + fd10;
            union32 r14 = { .i32 = fd56.st64.hi32 };
            bool p3 = r14.f32 == 0;
            double fd58 = p3 ? -INFINITY : fd11;
            return fd58;
        }

        double fd12 = fd57.f64 + 1.0;
        double fd13 = 1.0;
        double fd14 = 1 / (fd12);
        double fd15 = -fd12;
        double fd16 = fd15 * fd14 + fd13;
        double fd17 = fd16 * fd16 + fd16;
        double fd18 = fd17 * fd14 + fd14;
        double fd19 = fd57.f64 + -1.0;
        double fd20 = fd19 * fd18;
        double fd21 = fd19 * fd18 + fd20;
        double fd22 = fd21 * fd21;
        double fd23 = 0.000004036488625710656;
        double fd24 = 0.0000010263276909458365;
        double fd25 = fd24 * fd22 + fd23;
        double fd26 = 0.000018784407018835667;
        double fd27 = fd25 * fd22 + fd26;
        double fd28 = 0.00008877807196497797;
        double fd29 = fd27 * fd22 + fd28;
        double fd30 = 0.00043402779293201976;
        double fd31 = fd29 * fd22 + fd30;
        double fd32 = 0.0022321428567665586;
        double fd33 = fd31 * fd22 + fd32;
        double fd34 = 0.012500000000004514;
        double fd35 = fd33 * fd22 + fd34;
        double fd36 = 0.08333333333333331;
        double fd37 = fd35 * fd22 + fd36;
        double fd38 = fd19 - fd21;
        double fd39 = fd38 + fd38;
        double fd40 = -fd21;
        double fd41 = fd40 * fd19 + fd39;
        double fd42 = fd18 * fd41;
        double fd43 = fd22 * fd37;
        double fd44 = fd43 * fd21 + fd42;
        int r21 = r27 ^ -2147483648;
        uint32_t r22 = -2147483648;
        uint32_t r23 = 1127219200;
        union64 fd45, fd46;
        fd45.st64.lo32 = r21;
        fd45.st64.hi32 = r23;
        fd46.st64.lo32 = r22;
        fd46.st64.hi32 = r23;

        double fd47 = fd45.f64 - fd46.f64;
        double fd48 = 0.6931471805599453;
        double fd49 = fd47 * fd48 + fd21;
        double fd50 = -fd47;
        double fd51 = fd50 * fd48 + fd49;
        double fd52 = fd51 - fd21;
        double fd53 = fd44 - fd52;
        double fd54 = 2.3190468138462996e-17;
        double fd55 = fd47 * fd54 + fd53;
        double fd58 = fd49 + fd55;
        return fd58;
    }


    float tanhf(float x) {
        float f1 = x;
        float f2 = fabsf(x);
        float f24 = 0;
        if (f2 < 0.6) {
            float f14 = f1 * f1;
            float f17 = 0.01573968306183815f * f14 + -0.05230396240949631f;
            float f19 = f17 * f14 + 0.13315297663211823f;
            float f21 = f19 * f14 + -0.33332768082618713f;
            float f23 = f21 * f14 + 0;
            f24 = f23 * f1 + f1;
        } else {
            float f6 = f2 * 2.88539f;
            float f7 = exp2f(f6);
            float f8 = f7 + 1.0f;
            float f10 = 1/(f8);
            float f12 = f10 * (- 2.0f)  + 1.0f;
            float f13 = f2 >= 9.010913848876953f ? 1.0f : f12;
            union32 r2 = { .f32 = f13 };
            union32 r3 = { .f32 = f1 };
            union32 r4 = { .u32 = r3.u32 & (uint32_t)- 2147483648};
            union32 r5 = { .u32 = r4.u32 | r2.u32 };
            f24 = r5.f32;
        }
        return f24;
    }

    double tanh(double x) {
        return x;
    }

    float erff(float x) {
        float f1 = x;
        float f5 = fabsf(f1);
        bool p1 = f5 < 1.002959966659546f;
        bool p2 = f5 >= 1.002959966659546f;
        float f6 = f1 * f1;
        float f7 = p2 ? f5 : f6;
        float f8 = p2 ? 0.00011219871521461755f : 0.00008483494457323104f;
        float f9 = p2 ? -0.0013275252422317863f : -0.0008213091641664505f;
        float f10 = f8 * f7 + f9;
        float f11 = p2 ? 0.00839653518050909f : 0.005213488824665546f;
        float f12 = f10 * f7 + f11;
        float f13 = p2 ? -0.04024658352136612f : -0.026868773624300957f;
        float f14 = f12 * f7 + f13;
        float f15 = p2 ? 0.15950430929660797f : 0.11284004896879196f;
        float f16 = f14 * f7 + f15;
        float f17 = p2 ? 0.9129176735877991f : -0.37612664699554443f;
        float f18 = f16 * f7 + f17;
        float f19 = p2 ? 0.6290600299835205f : 0.12837915122509003f;
        float f20 = f18 * f7 + f19;
        float f21 = -f5;
        float f22 = p2 ? f21 : f1;
        float f26 = f20 * f22 + f22;
        if (p1)
            return f26;
        float f23 = exp2f(f26);
        float f25 = 1.0f - f23;
        union32 r2 = { .f32 = f25 };
        union32 r3 = { .f32 = f1 };
        union32 r4 = { .u32 = r3.u32 & (uint32_t) - 2147483648 };
        union32 r5 = { .u32 = r4.u32 | r2.u32 };
        f26 = r5.f32;
        return f26;
    }

    double erf(double x) {
        double fd1 = x;
        double fd2 = x >= 0 ? x : -x;    // abs(x)
        double fd5 = -3.642577040697121e-15 * fd2 + 1.636624707883456e-13;
        double fd7 = fd5 * fd2 + -3.4079297100747907e-12;
        double fd9 = fd7 * fd2 + 4.3555974045566826e-11;
        double fd11 = fd9 * fd2 + -3.806599039253438e-10;
        double fd13 = fd11 * fd2 + 2.389211325400646e-9;
        double fd15 = fd13 * fd2 + -1.0909760903049583e-8;
        double fd17 = fd15 * fd2 + 3.508901186822047e-8;
        double fd19 = fd17 * fd2 + -6.72032708005184e-8;
        double fd21 = fd19 * fd2 + -8.299332548682465e-9;
        double fd23 = fd21 * fd2 + 5.643145203798444e-7;
        double fd25 = fd23 * fd2 + -0.0000020278249778025215;
        double fd27 = fd25 * fd2 + 0.0000031218939658311085;
        double fd29 = fd27 * fd2 + 0.000003180046170354655;
        double fd31 = fd29 * fd2 + -0.000029583306728241582;
        double fd33 = fd31 * fd2 + 0.00006442483232470452;
        double fd35 = fd33 * fd2 + 0.00002590260570264615;
        double fd37 = fd35 * fd2 + -0.0005901318195328236;
        double fd39 = fd37 * fd2 + 0.001696207528729842;
        double fd41 = fd39 * fd2 + -0.00020919483164788562;
        double fd43 = fd41 * fd2 + -0.019128446995328407;
        double fd45 = fd43 * fd2 + 0.10277260330144233;
        double fd47 = fd45 * fd2 + 0.6366197723675876;
        double fd49 = fd47 * fd2 + 0.1283791670955126;
        double fd50 = fd49 * fd2 + fd2;
        double fd51 = fd2 - fd50;
        double fd52 = fd49 * fd2 + fd51;
        double fd53 = -fd50;
        double fd54 = -fd52;
        float f1 = (float)fd50;
        float f2 = f1 * -1.44269502f;
        float f3 = round(f2);  // rni : round to nearest integer, prefer even integer
        double fd55 = (double)f3;
        double fd56 = -fd55;
        double fd58 = fd56 * 0.6931471805599453 + fd53;
        double fd61 = 2.506210197447926e-8 * fd58 + 2.762626470761021e-7;
        double fd63 = fd61 * fd58 + 0.0000027557381798574225;
        double fd65 = fd63 * fd58 + 0.000024801504596442702;
        double fd67 = fd65 * fd58 + 0.00019841269746984988;
        double fd69 = fd67 * fd58 + 0.0013888888932264757;
        double fd71 = fd69 * fd58 + 0.008333333333377164;
        double fd73 = fd71 * fd58 + 0.041666666666573884;
        double fd75 = fd73 * fd58 + 0.16666666666666607;
        double fd77 = fd75 * fd58 + 0.5000000000000006;
        double fd78 = fd58 * fd77;
        double fd79 = fd78 * fd58 + fd54;
        double fd80 = fd58 + fd79;
        float f4 = 0;
        f4 = exp2f(f3);    // f4 = exp2f(f3);
        double fd81 = (double)f4;
        double fd83 = 1.0 - fd81;
        double fd84 = -fd80;
        double fd85 = fd84 * fd81 + fd83;
        double fd86 = fd2 >= 5.9215871957945065 ? 1.0 : fd85;
        union64 u1, u2;
        u1.f64 = fd1;
        u2.f64 = fd86;
        uint32_t r4 = 0, r5 = 0;
        r4 = u1.st64.hi32 & -2147483648;
        r5 = u2.st64.hi32 | r4;
        u2.st64.hi32 = r5;
        return u2.f64;
    }
    /*
    * e^x = 2^y 
    * --> y = log_2(e^x) = x * log_2(e)
    * --> e^x = 2^(x * log_2(e)) = 2^x * 2^log_2(e)
    */
    float expf(float x) {
        float f1 = x;
        float f4 = f1 * 0.00572498003f + 0.5f;
        float f7 = fminf(fmaxf(f4, 0.0f), 1.0f);
        float f9 = f7 * 252.0f + 12582913.0f;
        float f10 = f9 + -12583039.0f;                          
       float f12 = std::fmaf(f1, 1.44269502f, -f10);  // fma has high precision than multi-add
        float f14 = f1 * 1.925963033500011e-8 + f12;
        union32 r2 = { .f32 = f9 };
        union32 r3 = { .u32 = r2.u32 << 23 };
        float f15 = r3.f32;
        float f16 = exp2f(f14);
        float f17 = f16 * f15;
        return f17;

    }


} // namespace cuda_math