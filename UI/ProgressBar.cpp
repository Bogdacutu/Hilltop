#include "UI/ProgressBar.h"


namespace Hilltop {
namespace UI {

Hilltop::UI::ProgressBar::ProgressBar() : Element() {}

void Hilltop::UI::ProgressBar::handleDraw(Console::BufferedConsoleRegion &region) const {
    Console::ConsoleColor c = make_bg_color(color);

    float v = value;
    if (inverted)
        v = 1 - v;
    int start = 0;
    int end = width;
    if (!inverted)
        end = (int)(end * value);
    else
        start = end - (int)(width * value);

    for (int i = start; i < end; i++)
        for (int j = 0; j < height; j++)
            region.set(j, i, L' ', c, Console::BACKGROUND_COLOR);
}

std::shared_ptr<ProgressBar> Hilltop::UI::ProgressBar::create() {
    return std::shared_ptr<ProgressBar>(new ProgressBar());
}

}
}
