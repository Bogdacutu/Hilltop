#pragma once

#include "UI/Element.h"
#include <vector>


namespace Hilltop {
namespace UI {

class ElementCollection : public Element {
protected:
    ElementCollection();

    std::vector<std::shared_ptr<Element>> children;

    virtual void handleDraw(Console::BufferedConsoleRegion &region) const override;

public:
    bool drawBackground = false;
    Console::ConsoleColor backgroundColor = Console::ConsoleColor::BLACK;

    const std::vector<std::shared_ptr<Element>> &getChildren() const;
    void addChild(Element &element);
    void removeChild(Element &element);

    static std::shared_ptr<ElementCollection> create();
};

}
}
