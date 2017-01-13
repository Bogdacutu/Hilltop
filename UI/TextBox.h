#pragma once

#include "Console/Text.h"
#include "UI/Element.h"
#include <string>


namespace Hilltop {
namespace UI {

class TextBox : public Element {
protected:
    TextBox();

    virtual void handleDraw(Console::BufferedConsoleRegion &region) const override;

public:
    std::string text;
    Console::TextAlignment alignment = Console::TextAlignment::LEFT;
    Console::ConsoleColor color = Console::ConsoleColor::WHITE;
    bool wordWrap = false;

    static std::shared_ptr<TextBox> create();
};

}
}
