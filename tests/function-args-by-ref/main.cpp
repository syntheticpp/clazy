#include <QtCore/QDebug>
#include <QtGui/QColor>



struct Trivial {
    int a;
};

struct NonTrivial {
    NonTrivial() {}
    NonTrivial(const NonTrivial &) {}
    void constFunction() const {};
    void nonConstFunction() {};
    int a;
};

extern void by_pointer(NonTrivial*);
extern void by_const_pointer(const NonTrivial*);
extern void by_ref(NonTrivial&);
extern void by_constRef(const NonTrivial&);

int foo1(const Trivial nt) // Test #1: No warning
{
    return nt.a;
}

int foo2(const NonTrivial nt) // Test #2: Warning
{
    nt.constFunction();
    return nt.a;
}

int foo3(NonTrivial nt) // Test #3: Warning
{
    return nt.a;
}

int foo4(NonTrivial nt) // Test #4: Warning
{
    return nt.a;
}

int foo5(NonTrivial nt) // Test #5: No warning
{
    by_pointer(&nt);
    return nt.a;
}


int foo6(NonTrivial nt) // Test #6: No warning
{
    by_ref(nt);
    return nt.a;
}

int foo7(NonTrivial nt) // Test #7: No warning
{
    nt.nonConstFunction();
    return nt.a;
}

int foo8(NonTrivial nt) // Test #8: Warning
{
    nt.constFunction();
    return nt.a;
}

int foo9(NonTrivial nt) // Test #9: Warning
{
    by_constRef(nt);
    return nt.a;
}

int foo10(NonTrivial nt) // Test #10: Warning
{
    by_const_pointer(&nt);
    return nt.a;
}

int foo11(QColor) // Test #11: No warning
{
    return 1;
}

int foo12(NonTrivial nt) // Test #12: No warning
{
    nt = NonTrivial();
    return 1;
}
