// EPOS Math Utility Declarations

#ifndef __math_h
#define __math_h

#include <system/config.h>

__BEGIN_UTIL

static const float E = 2.71828183;

template <typename T>
inline T logf(T num, float base = E, float epsilon = 1e-12)
{
    if(num == 0) return 1;

    if(num < 1  && base < 1) return 0;

    T integer = 0;
    while(num < 1) {
        integer--;
        num *= base;
    }

    while(num >= base) {
        integer++;
        num /= base;
    }

    T partial = 0.5;
    num *= num;
    T decimal = 0.0;
    while(partial > epsilon) {
        if(num >= base) {
            decimal += partial;
            num /= base;
        }
        partial *= 0.5;
        num *= num;
    }

    return integer + decimal;
}

template <typename T>
inline T sqrt(T x)
{
    T res = 0;

    // "one" starts at the highest power of four <= than the argument.
    T one = static_cast<T>(1) << (sizeof(T) * 8 - 2);
    while(one > x)
        one >>= 2;

    while(one != 0) {
        if(x >= res + one) {
            x -= (res + one);
            res += 2 * one;
        }
        res >>= 1;
        one >>= 2;
    }
    return res;
}

template <typename T>
inline T pow(const T & x, unsigned int e)
{
    if(e == 0) 
        return 1;
    else if(e == 1)
        return x;
    else if(e & 1)
        return pow(x * x, e / 2);
    else
        return x * pow(x * x, (e - 1) / 2);
}

inline float fast_log2(float val)
{
    int * const exp_ptr = reinterpret_cast <int *> (&val);
    int x = *exp_ptr;
    const int log_2 = ((x >> 23) & 255) - 128;
    x &= ~(255 << 23);
    x += 127 << 23;
    (*exp_ptr) = x;

    val = ((-1.0f/3) * val + 2) * val - 2.0f/3;

    return (val + log_2);
}

inline float fast_log(float val)
{
    static const float ln_2 = 0.69314718;
    return (fast_log2(val) * ln_2);
}

template <typename T>
const T & min(const T & x, const T & y)
{
    return (x <= y) ? x : y;
}

template <typename T>
const T & max(const T & x, const T & y)
{
    return (x > y) ? x : y;
}

template <typename T>
T abs(const T & x)
{
    return (x > 0) ? x : -x;
}

template <typename T>
T largest(const T array[], int size)
{
    T result = array[0];
    for(int i = 1; i < size; i++)
        if(array[i] > result)
          result = array[i];
    return result;
}

template <typename T>
T smallest(const T array[], int size)
{
    T result = array[0];
    for(int i = 1; i < size; i++)
        if(array[i] < result)
          result = array[i];
    return result;
}

template <typename T>
T mean(const T array[], int size)
{
    T sum = 0;
    for(int i = 0; i < size; i++)
        sum += array[i];
    return sum / size;
}

template <typename T>
T variance(const T array[], int size, const T & mean)
{
    T var = 0;
    for(int i = 0; i < size; i++) {
        T tmp = mean - array[i];
        var = var + (tmp * tmp);
    }
    return var / (size - 1);
}

__END_UTIL

#endif
