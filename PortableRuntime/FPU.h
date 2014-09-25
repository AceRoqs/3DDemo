#ifndef FPU_H
#define FPU_H

namespace PortableRuntime
{

class Scoped_FPU_exception_control
{
    unsigned int m_control;
    unsigned int m_exception_mask;

public:
    Scoped_FPU_exception_control(unsigned int exception_mask);
    ~Scoped_FPU_exception_control();
    void enable(unsigned int fpu_exceptions);
    void disable(unsigned int fpu_exceptions);
};

}

#endif

