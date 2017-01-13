#pragma once

#include "UI/TextBox.h"


namespace Hilltop {
namespace UI {

class Button : public TextBox {
protected:
    Button();

    virtual void handleDraw(Console::BufferedConsoleRegion &region) const override;

public:
    Console::ConsoleColor backgroundColor = Console::ConsoleColor::BLACK;

    static std::shared_ptr<Button> create();
};

}
}
