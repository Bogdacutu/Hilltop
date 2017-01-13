#include "UI/TextBox.h"


namespace Hilltop {
namespace UI {

TextBox::TextBox() : Element() {}

void TextBox::handleDraw(Console::BufferedConsoleRegion &region) const {
    Element::handleDraw(region);

    Console::TextBoxSize size = printText(nullptr, 0, 0, width, height, text, color, alignment, wordWrap);
    printText(&region, (height - size.lines) / 2, 0, width, height, text, color, alignment, wordWrap);
}

std::shared_ptr<TextBox> TextBox::create() {
    return std::shared_ptr<TextBox>(new TextBox());
}

}
}
