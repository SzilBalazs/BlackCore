#ifndef BLACKCORE_UCI_H
#define BLACKCORE_UCI_H

#include <iostream>

inline void out() {
    std::cout << std::endl;
}

template<typename T, typename... Args>
inline void out(T a, Args... args) {
    std::cout << a << " ";
    out(args...);
}

void uciLoop();

#endif //BLACKCORE_UCI_H
