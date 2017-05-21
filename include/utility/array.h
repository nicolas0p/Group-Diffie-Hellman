// EPOS Array Utility Declarations

#ifndef __array_h
#define __array_h

#include <utility/ostream.h>
#include <utility/string.h>

__BEGIN_UTIL

template<typename T, unsigned int N_ELEMENTS>
class Array
{
    static const unsigned int SIZE = N_ELEMENTS * sizeof(T);

public:
    Array() {}

    Array(const void * data, unsigned int data_size) {
        copy_and_pad(data, data_size);
    }

    template <typename U>
    Array(const U & u) {
        copy_and_pad(&u, sizeof(U));
    }

    friend OStream & operator<<(OStream & out, const Array & array) {
        out << "[";
        for(unsigned int i = 0; i < N_ELEMENTS; i++) {
            out << array[i];
            if(i < N_ELEMENTS - 1)
                out << ",";
        }
        out << "]";
        return out;
    }

    friend Debug & operator<<(Debug & out, const Array & array) {
        out << "[";
        for(unsigned int i = 0; i < N_ELEMENTS; i++) {
            out << array[i];
            if(i < N_ELEMENTS - 1)
                out << ",";
        }
        out << "]";
        return out;
    }

    T & operator[](const size_t i) { return _data[i]; }
    const T & operator[](const size_t i) const { return _data[i]; }
    operator const T*() const { return _data; }
    operator T*() { return _data; }

    template <typename U>
    bool operator==(const U & u) const { return (SIZE == sizeof(U)) && (!memcmp(&u, _data, SIZE)); }
    template <typename U>
    bool operator!=(const U & u) const { return !(*this == u); }

private:
    void copy_and_pad(const void * data, unsigned int data_size) {
        if(SIZE <= data_size)
            memcpy(_data, data, SIZE);
        else {
            memset(_data, 0, SIZE);
            memcpy(_data, data, data_size);
        }
    }

private:
    T _data[N_ELEMENTS];
};

__END_UTIL

#endif
