#ifndef CLIA_BASE_COPYABLE_H_
#define CLIA_BASE_COPYABLE_H_

namespace clia {
    class Copyable {
    protected:
        inline Copyable() = default;
        inline ~Copyable() = default;
        inline Copyable(const Copyable&) = default;
        inline Copyable& operator=(const Copyable&) = default;
    };
}

#endif