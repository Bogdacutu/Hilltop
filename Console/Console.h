#pragma once

#include "ConsoleColor.h"
#include <memory>


namespace Hilltop {
namespace Console {

class Console : public std::enable_shared_from_this<Console> {
protected:
    Console(unsigned short width, unsigned short height);

public:
    const unsigned short width, height;

    virtual ~Console() {}

    virtual void set(unsigned short x, unsigned short y, wchar_t ch, ConsoleColor color) = 0;
    virtual void clear(ConsoleColor color);

    virtual void commit() const {}
};

}
}
