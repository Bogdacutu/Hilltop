#pragma once

#include "BufferedConsole.h"
#include <string>


namespace Hilltop {
namespace Console {

typedef struct {
    unsigned short lines, cols;
} TextBoxSize;

enum TextAlignment {
    LEFT,
    CENTER,
    RIGHT,
};

TextBoxSize printText(BufferedConsole *buffer, unsigned short x, unsigned short y, unsigned short width,
    unsigned short height, std::string text, ConsoleColor color, TextAlignment align = LEFT,
    bool wordWrap = true);

}
}
