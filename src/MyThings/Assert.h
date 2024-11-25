#pragma once

#include <iostream>

#define ASSERT(X, msg) \
    if (!(X))\
    {\
        std::cerr << "File: \"" << __FILE__ << "\", line " << __LINE__ << ":\n";\
        std::cerr << "Assertion \"" << #X << "\" failed.\n";\
        std::cerr << msg << "\n";\
        std::exit(-1);\
    }
