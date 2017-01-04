#include "UI.h"
#include <algorithm>

using namespace Hilltop::Console;
using namespace Hilltop::UI;



//
// Element
//

Hilltop::UI::Element::Element() {}

void Hilltop::UI::Element::handleDraw(Console::BufferedConsoleRegion &region) {}

std::shared_ptr<Element> Hilltop::UI::Element::create() {
    return std::shared_ptr<Element>(new Element());
}

void Hilltop::UI::Element::draw(Console::BufferedConsole &console) {
    std::shared_ptr<BufferedConsoleRegion> region =
        BufferedConsoleRegion::create(console, width, height, x, y);
    handleDraw(*region);
}



//
// ElementCollection
//

Hilltop::UI::ElementCollection::ElementCollection() : Element() {}

void Hilltop::UI::ElementCollection::handleDraw(Console::BufferedConsoleRegion &region) {
    Element::handleDraw(region);

    if (drawBackground)
        region.clear(backgroundColor);

    for (std::shared_ptr<Element> &el : children)
        el->draw(region);
}

const std::vector<std::shared_ptr<Element>> &Hilltop::UI::ElementCollection::getChildren() {
    return children;
}

void Hilltop::UI::ElementCollection::addChild(Element &element) {
    children.push_back(element.shared_from_this());
}

void Hilltop::UI::ElementCollection::removeChild(Element &element) {
    std::shared_ptr<Element> el = element.shared_from_this();
    std::vector<std::shared_ptr<Element>>::iterator it = std::find(children.begin(), children.end(), el);
    if (it != children.end())
        children.erase(it);
}

std::shared_ptr<ElementCollection> Hilltop::UI::ElementCollection::create() {
    return std::shared_ptr<ElementCollection>(new ElementCollection());
}



//
// TextBox
//

Hilltop::UI::TextBox::TextBox() : Element() {}

void Hilltop::UI::TextBox::handleDraw(Console::BufferedConsoleRegion &region) {
    Element::handleDraw(region);

    TextBoxSize size = printText(nullptr, 0, 0, width, height, text, color, alignment, wordWrap);
    printText(&region, (height - size.lines) / 2, 0, width, height, text, color, alignment, wordWrap);
}

std::shared_ptr<TextBox> Hilltop::UI::TextBox::create() {
    return std::shared_ptr<TextBox>(new TextBox());
}



//
// Button
//

Hilltop::UI::Button::Button() : TextBox() {
    alignment = CENTER;
}

void Hilltop::UI::Button::handleDraw(Console::BufferedConsoleRegion &region) {
    region.clear(backgroundColor);

    TextBox::handleDraw(region);
}

std::shared_ptr<Button> Hilltop::UI::Button::create() {
    return std::shared_ptr<Button>(new Button());
}



//
// ProgressBar
//

Hilltop::UI::ProgressBar::ProgressBar() : Element() {}

void Hilltop::UI::ProgressBar::handleDraw(Console::BufferedConsoleRegion &region) {
    ConsoleColor c = make_bg_color(color);

    float v = value;
    if (inverted)
        v = 1 - v;
    int start = 0;
    int end = width;
    if (!inverted)
        end *= value;
    else
        start = end - width * value;

    for (int i = start; i < end; i++)
        for (int j = 0; j < height; j++)
            region.set(j, i, L' ', c, BACKGROUND_COLOR);
}

std::shared_ptr<ProgressBar> Hilltop::UI::ProgressBar::create() {
    return std::shared_ptr<ProgressBar>(new ProgressBar());
}
