#include "SnapshotConsole.h"

using namespace Hilltop::Console;


Hilltop::Console::SnapshotConsole::SnapshotConsole(BufferedConsole &console)
    : BufferedConsole(console.width, console.height)
    , console(std::static_pointer_cast<BufferedConsole>(console.shared_from_this())) {
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            const unsigned int idx = i * width + j;
            buffer[idx] = console.get(i, j);
        }
    }
}

std::shared_ptr<SnapshotConsole> Hilltop::Console::SnapshotConsole::create(BufferedConsole &console) {
    return std::shared_ptr<SnapshotConsole>(new SnapshotConsole(console));
}

BufferedConsole::pixel_t Hilltop::Console::SnapshotConsole::get(unsigned short x, unsigned short y) const {
    return console->get(x, y);
}

void Hilltop::Console::SnapshotConsole::set(unsigned short x, unsigned short y, wchar_t ch,
    ConsoleColor color) {
    console->set(x, y, ch, color);
}

void Hilltop::Console::SnapshotConsole::set(unsigned short x, unsigned short y, wchar_t ch,
    ConsoleColor color, ConsoleColorType colorMask) {
    console->set(x, y, ch, color, colorMask);
}

void Hilltop::Console::SnapshotConsole::clear(ConsoleColor color) {
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            const unsigned int idx = i * width + j;
            set(i, j, buffer[idx].ch, buffer[idx].color);
        }
    }
}

void Hilltop::Console::SnapshotConsole::commit() const {
    console->commit();
}
