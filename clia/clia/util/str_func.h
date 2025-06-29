#ifndef CLIA_UTIL_STR_FUNC_H_
#define CLIA_UTIL_STR_FUNC_H_

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdlib>

namespace clia {
    namespace util {
        namespace str_func {
            template<typename Integral>
            inline std::size_t convert(char *outbuf, const int size, const Integral value) noexcept;
        }
    }
}

template <typename Integer>
inline std::size_t clia::util::str_func::convert(char *outbuf, const int size, const Integer value) noexcept {
    if (size < 32) {
        return 0; // Not enough space to convert
    }
    static_assert(std::is_integral<Integer>::value, "T must be an integral type");
    constexpr char kDigits[] = "0123456789";
    char* p = outbuf;
    Integer i = value;
    do {
        const int lsd = static_cast<int>(i % 10);
        *p++ = kDigits[lsd];
        i /= 10;
    } while (i != 0);
    if (value < 0) {
        *p++ = '-';
    }
    *p = '\0';
    std::reverse(outbuf, p);
    return p - outbuf;
}

#endif