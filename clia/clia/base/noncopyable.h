#ifndef CLIA_BASE_NOCOPYABLE_H_
#define CLIA_BASE_NOCOPYABLE_H_

namespace clia {
    class Noncopyable {
    public:
        Noncopyable(const Noncopyable&) = delete;
        void operator=(const Noncopyable&) = delete;
    protected:
        Noncopyable() = default;
        ~Noncopyable() = default;
    };
}

#endif