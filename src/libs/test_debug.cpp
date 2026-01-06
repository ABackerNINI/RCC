#include <cassert>
#include <climits>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <type_traits>
#include <vector>

using namespace std;

#define DEBUG

#ifdef DEBUG

#define cdbg __cdbg()
#define cdbg_ex __cdbg_ex()

ostream &__cdbg() {
    cerr << "[debug] ";
    return cerr;
}

ostream &__cdbg_ex() {
    return cerr;
}

#else

class null_ostream {
  public:
    template <typename T> inline null_ostream &operator<<(const T &x) {
        (void)x;
        return *this;
    }
    inline null_ostream &operator<<(ostream &(*f)(ostream &)) {
        (void)f;
        return *this;
    }
};

inline null_ostream &__cdbg() {
    static null_ostream nul;
    return nul;
}

inline null_ostream &__cdbg_ex() {
    static null_ostream nul;
    return nul;
}

#define cdbg __cdbg()
#define cdbg_ex __cdbg_ex()

#endif

int main() {
    int a = 10;
    cdbg << "what" << a << endl;
    cdbg_ex << "what" << a << endl;

    return 0;
}
