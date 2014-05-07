#ifndef CLOCK_H
#define CLOCK_H

namespace WindowsCommon
{

class Clock
{
private:
    LARGE_INTEGER m_frequency;
    LARGE_INTEGER m_last_counter;

public:
    Clock() NOEXCEPT;
    float ellapsed_milliseconds() NOEXCEPT;
};

}

#endif

