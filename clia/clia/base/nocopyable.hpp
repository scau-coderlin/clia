#pragma once

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