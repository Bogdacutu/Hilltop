#pragma once

#include "BufferedConsole.h"
#include <vector>


namespace Hilltop {
namespace Console {

class SnapshotConsole : public BufferedConsole {
private:
    std::vector<pixel_t> buffer = std::vector<pixel_t>(width * height);

protected:
    SnapshotConsole(BufferedConsole &console);

public:
    const std::shared_ptr<BufferedConsole> console;

    static std::shared_ptr<SnapshotConsole> create(BufferedConsole &console);

    virtual pixel_t get(unsigned short x, unsigned short y) const override;
    virtual void set(unsigned short x, unsigned short y, wchar_t ch, ConsoleColor color) override;
    virtual void set(unsigned short x, unsigned short y, wchar_t ch, ConsoleColor color,
        ConsoleColorType colorMask) override;
    virtual void clear(ConsoleColor color) override;

    virtual void commit() const override;
};

}
}
