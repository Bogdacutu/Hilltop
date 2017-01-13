#include "UI/ElementCollection.h"


namespace Hilltop {
namespace UI {

ElementCollection::ElementCollection() : Element() {}

void ElementCollection::handleDraw(Console::BufferedConsoleRegion &region) const {
    Element::handleDraw(region);

    if (drawBackground)
        region.clear(backgroundColor);

    for (const std::shared_ptr<Element> &el : children)
        el->draw(region);
}

const std::vector<std::shared_ptr<Element>> &ElementCollection::getChildren() const {
    return children;
}

void ElementCollection::addChild(Element &element) {
    children.push_back(element.shared_from_this());
}

void ElementCollection::removeChild(Element &element) {
    std::shared_ptr<Element> el = element.shared_from_this();
    std::vector<std::shared_ptr<Element>>::iterator it = std::find(children.begin(), children.end(), el);
    if (it != children.end())
        children.erase(it);
}

std::shared_ptr<ElementCollection> ElementCollection::create() {
    return std::shared_ptr<ElementCollection>(new ElementCollection());
}

}
}
