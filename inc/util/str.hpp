#pragma once

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdlib>

namespace clia {
    namespace util {
        namespace str {
            template<typename Integral>
            inline std::size_t convert(char *outbuf, const int size, const Integral value) noexcept;
        }
    }
}

template<typename Integral>
inline std::size_t clia::util::str::convert(char *outbuf, const int size, const Integral value) noexcept {
/* 
18446744073709551615
*/
    if (size < 32) {
        return 0; // Not enough space to convert
    }
    static_assert(std::is_integral<Integral>::value, "T must be an integral type");
    constexpr char DIGITS[] = "0123456789";
    char* p = outbuf;
    Integral i = value;
    do {
        const int lsd = static_cast<int>(i % 10);
        *p++ = DIGITS[lsd];
        i /= 10;
    } while (i != 0);
    if (value < 0) {
        *p++ = '-';
    }
    *p = '\0';
    std::reverse(outbuf, p);
    return p - outbuf;
}