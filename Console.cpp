#include "Console.h"
#include <algorithm>
#include <deque>
#include <sstream>

using namespace Hilltop::Console;


ConsoleColor Hilltop::Console::make_color(ConsoleColor background, ConsoleColor foreground) {
    return (ConsoleColor)(((background << BACKGROUND_SHIFT) & BACKGROUND_COLOR)
        | ((foreground << FOREGROUND_SHIFT) & FOREGROUND_COLOR));
}

ConsoleColor Hilltop::Console::make_bg_color(ConsoleColor background) {
    return make_color(background, (ConsoleColor)0);
}

ConsoleColor Hilltop::Console::make_fg_color(ConsoleColor foreground) {
    return make_color((ConsoleColor)0, foreground);
}

ConsoleColor Hilltop::Console::calc_masked_color(ConsoleColor old, ConsoleColor color, ConsoleColorType mask) {
    return (ConsoleColor)((old & ~mask) | (color & mask));
}

bool Hilltop::Console::is_bright_color(ConsoleColor color) {
    return color >= GRAY;
}

bool Hilltop::Console::is_dark_color(ConsoleColor color) {
    return color <= DARK_GRAY;
}



//
// Console
//

Hilltop::Console::Console::Console(unsigned short width, unsigned short height)
    : width(width), height(height) {}

void Hilltop::Console::Console::clear(ConsoleColor color) {
    color = make_bg_color(color);
    for (int i = 0; i < height; i++)
        for (int j = 0; j < width; j++)
            set(i, j, ' ', color);
}



//
// BufferedConsole
//

Hilltop::Console::BufferedConsole::BufferedConsole(unsigned short width, unsigned short height)
    : Console(width, height) {}

void Hilltop::Console::BufferedConsole::set(unsigned short x, unsigned short y, wchar_t ch, ConsoleColor color) {
    set(x, y, ch, color, (ConsoleColorType)(BACKGROUND_COLOR | FOREGROUND_COLOR));
}



//
// BufferedConsoleRegion
//

Hilltop::Console::BufferedConsoleRegion::BufferedConsoleRegion(BufferedConsole &console,
    unsigned short width, unsigned short height, unsigned short x, unsigned short y)
    : BufferedConsole(width, height), x(x), y(y)
    , console(std::static_pointer_cast<BufferedConsole>(console.shared_from_this())) {}

std::shared_ptr<BufferedConsoleRegion> Hilltop::Console::BufferedConsoleRegion::create(BufferedConsole& console,
    unsigned short width, unsigned short height, unsigned short x, unsigned short y) {
    return std::shared_ptr<BufferedConsoleRegion>(new BufferedConsoleRegion(console, width, height, x, y));
}

BufferedConsole::pixel_t Hilltop::Console::BufferedConsoleRegion::get(unsigned short x, unsigned short y) const {
    if (enforceBounds)
        if (x >= height || y >= width)
            return pixel_t();

    return console->get(this->x + x, this->y + y);
}

void Hilltop::Console::BufferedConsoleRegion::set(unsigned short x, unsigned short y, wchar_t ch,
    ConsoleColor color, ConsoleColorType colorMask) {
    if (enforceBounds)
        if (x >= height || y >= width)
            return;

    console->set(this->x + x, this->y + y, ch, color, colorMask);
}



//
// DoublePixelBufferedConsole
//

const uint8_t Hilltop::Console::DoublePixelBufferedConsole::BIT_MASKS[2] = { 0xf0, 0xf };
const int Hilltop::Console::DoublePixelBufferedConsole::BIT_SHIFTS[2] = { 4, 0 };

Hilltop::Console::DoublePixelBufferedConsole::DoublePixelBufferedConsole(unsigned short width, unsigned short height)
    : width((width + 1) / 2 * 2), height((height + 1) / 2 * 2) {
    buffer = std::vector<uint8_t>(this->width * (this->height) / 2);
}

ConsoleColor Hilltop::Console::DoublePixelBufferedConsole::get(unsigned short x, unsigned short y) const {
    if (x >= height || y >= width)
        return ConsoleColor();

    const unsigned int idx = x * width + y;
    return (ConsoleColor)((buffer[idx / 2] & BIT_MASKS[idx & 1]) >> BIT_SHIFTS[idx & 1]);
}

void Hilltop::Console::DoublePixelBufferedConsole::set(unsigned short x, unsigned short y, ConsoleColor color) {
    if (x >= height || y >= width)
        return;

    const unsigned int idx = x * width + y;
    buffer[idx / 2] = (buffer[idx / 2] & BIT_MASKS[!(idx & 1)]) | ((color << BIT_SHIFTS[idx & 1]) & BIT_MASKS[idx & 1]);
}

void Hilltop::Console::DoublePixelBufferedConsole::clear(ConsoleColor color) {
    uint8_t value = 0;
    for (int i = 0; i < sizeof(BIT_MASKS) / sizeof(*BIT_MASKS); i++)
        value |= (color << BIT_SHIFTS[i]) & BIT_MASKS[i];
    buffer.assign(buffer.size(), value);
}

void Hilltop::Console::DoublePixelBufferedConsole::commit(Console &buffer) const {
    for (unsigned short i = 0; i < height / 2; i++) {
        for (unsigned short j = 0; j < width; j++) {
            const ConsoleColor x = get(i * 2, j);
            const ConsoleColor y = get(i * 2 + 1, j);
            const wchar_t c = x == y ? ' ' : L'▄';
            buffer.set(i, j, c, make_color(x, y));
        }
    }
}



TextBoxSize Hilltop::Console::printText(BufferedConsole *buffer, unsigned short x, unsigned short y,
    unsigned short width, unsigned short height, std::string text, ConsoleColor color, TextAlignment align,
    bool wordWrap) {
    std::istringstream input(text);
    std::string word, lineIn, lineOut;
    std::vector<std::string> lines;
    
    while (std::getline(input, lineIn)) {
        std::deque<std::string> words;
        {
            std::istringstream lineInput(lineIn);
            while (std::getline(lineInput, word, ' '))
                words.push_back(word);
        }
        bool hadWords = words.size() > 0;
        bool firstWord = true;

        while (words.size()) {
            word = words.front();
            words.pop_front();

            if (word.size() > width) {
                words.push_front(word.substr(width));
                word = word.substr(0, width);
            }
            
            if (wordWrap && lineOut.length() && word.length() + lineOut.length() + 1 > width) {
                lines.push_back(lineOut);
                if (lines.size() == height)
                    break;
                lineOut.clear();
            }

            if (!firstWord) {
                lineOut += ' ';
            } else {
                firstWord = false;
            }
            lineOut += word;
        }

        if (lines.size() == height)
            break;

        if (lineOut.length() || !hadWords) {
            lines.push_back(lineOut);
            if (lines.size() == height)
                break;
            lineOut.clear();
        }
    }

    if (buffer) {
        std::shared_ptr<BufferedConsoleRegion> region = BufferedConsoleRegion::create(*buffer, width, height, x, y);
        for (int i = 0; i < lines.size(); i++) {
            int offset = 0;
            if (align == CENTER)
                offset = (int)(width - lines[i].length()) / 2;
            else if (align == RIGHT)
                offset = (int)(width - lines[i].length());

            for (int j = 0; j < lines[i].length(); j++)
                region->set(i, offset + j, lines[i][j], color, FOREGROUND_COLOR);
        }
    }

    TextBoxSize ret;
    ret.lines = (unsigned short)lines.size();
    ret.cols = 0;
    for (int i = 0; i < lines.size(); i++)
        ret.cols = std::max(ret.cols, (unsigned short)lines[i].length());
    return ret;
}
