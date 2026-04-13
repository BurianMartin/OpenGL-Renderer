#pragma once
#include <iostream>

/*
 * This is an include for general purpouse macros, constants and defines
 */

#define DEBUG

#ifdef DEBUG
#define debug_info(x)                                                  \
    {                                                                  \
        std::cout << "\x1B[33m" << "Dbg - Info: " << x << "\033[0m\n"; \
    }
#define debug_warn(x)                                                     \
    {                                                                     \
        std::cout << "\x1B[35m" << "Dbg - Warning: " << x << "\033[0m\n"; \
    }
#define debug_error(x)                                                  \
    {                                                                   \
        std::cout << "\x1B[31m" << "Dbg - Error: " << x << "\033[0m\n"; \
    }
#else
#define debug_info(x)
#define debug_warn(x)
#define debug_error(x)
#endif

#define n_rgb(x) (static_cast<float>(x) / 255.0f)