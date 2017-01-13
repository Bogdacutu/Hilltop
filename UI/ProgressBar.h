#pragma once

#include "UI/Element.h"


namespace Hilltop {
namespace UI {

class ProgressBar : public Element {
protected:
    ProgressBar();

    virtual void handleDraw(Console::BufferedConsoleRegion &region) const override;

public:
    float value = 0.5;
    bool inverted = false;
    Console::ConsoleColor color = Console::ConsoleColor::WHITE;

    static std::shared_ptr<ProgressBar> create();
};

}
}
