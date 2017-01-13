#pragma once

#include "BufferedConsole.h"


namespace Hilltop {
namespace Console {

class BufferedNativeConsole : public BufferedConsole {
protected:
    BufferedNativeConsole(unsigned short width, unsigned short height);

public:
    virtual void configure();
};

}
}
