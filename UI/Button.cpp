#include "UI/Button.h"


namespace Hilltop {
namespace UI {

Button::Button() : TextBox() {
    alignment = Console::CENTER;
}

void Button::handleDraw(Console::BufferedConsoleRegion &region) const {
    region.clear(backgroundColor);

    TextBox::handleDraw(region);
}

std::shared_ptr<Button> Button::create() {
    return std::shared_ptr<Button>(new Button());
}

}
}
