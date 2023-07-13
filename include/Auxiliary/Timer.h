#ifndef AUXILIARY_TIMER
#define AUXILIARY_TIMER

#include <cstddef>
#include <sys/time.h>

namespace wcp {

namespace Auxiliary {

class Timer {
public:
    Timer() : start_(Timestamp()) {}

    void Restart() { start_ = Timestamp(); }

    double ElapsedMilliseconds() {
        double cur = Timestamp();
        return cur - start_;
    }

private:
    /** Returns a timestamp ('now') in milliseconds (incl. a fractional part). */
    double Timestamp() {
        timeval tp;
        gettimeofday(&tp, nullptr);
        double ms = double(tp.tv_usec) / 1000.;
        return tp.tv_sec * 1000 + ms;
    }
private:
    double start_;
};

} // namespace Auxiliary

} // namespace wcp

#endif
