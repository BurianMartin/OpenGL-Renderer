#pragma once
#include <iostream>
#include <sstream>
#include <stdexcept>

/*
 * This is an include for general purpouse macros, constants and defines
 */

#ifndef NDEBUG
#define debug_info(x)                                                   \
    do                                                                  \
    {                                                                   \
        std::cout << "\x1B[33m" << "Dbg - Info: " << x << "\033[0m\n";  \
    } while (0)
#define debug_warn(x)                                                      \
    do                                                                     \
    {                                                                      \
        std::cout << "\x1B[35m" << "Dbg - Warning: " << x << "\033[0m\n";  \
    } while (0)
#define debug_error(x)                                                          \
    do                                                                         \
    {                                                                          \
        std::ostringstream _dbg_oss;                                           \
        _dbg_oss << x;                                                         \
        std::string _dbg_msg = _dbg_oss.str();                                 \
        std::cout << "\x1B[31m" << "Dbg - Error: " << _dbg_msg << "\033[0m\n"; \
        throw std::runtime_error(_dbg_msg);                                    \
    } while (0)
#else
#define debug_info(x)
#define debug_warn(x)
#define debug_error(x)                                  \
    do                                                  \
    {                                                   \
        std::cout << x << std::endl;                    \
        throw std::runtime_error("An error occurred");  \
    } while (0)
#endif

#ifdef LOG_EVENTS
#define debug_event(x)                                                   \
    do                                                                   \
    {                                                                    \
        std::cout << "\x1B[36m" << "Dbg - Event: " << x << "\033[0m\n";  \
    } while (0)
#else
#define debug_event(x)
#endif

#define n_rgb(x) (static_cast<float>(x) / 255.0f)
