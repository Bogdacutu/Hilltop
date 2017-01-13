#include "UI/Element.h"


namespace Hilltop {
namespace UI {

Element::Element() {}

void Element::handleDraw(Console::BufferedConsoleRegion &region) const {}

Element::~Element() {}

std::shared_ptr<Element> Element::create() {
    return std::shared_ptr<Element>(new Element());
}

void Element::draw(Console::BufferedConsole &console) const {
    std::shared_ptr<Console::BufferedConsoleRegion> region =
        Console::BufferedConsoleRegion::create(console, width, height, x, y);
    handleDraw(*region);
}

}
}
