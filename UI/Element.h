#pragma once

#include "Console/BufferedConsoleRegion.h"
#include <memory>


namespace Hilltop {
namespace UI {

class Element : public std::enable_shared_from_this<Element> {
protected:
    Element();

    virtual void handleDraw(Console::BufferedConsoleRegion &region) const;

public:
    unsigned short width, height;
    unsigned short x, y;

    virtual ~Element();
    static std::shared_ptr<Element> create();

    void draw(Console::BufferedConsole &console) const;
};

}
}
