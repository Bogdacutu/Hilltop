#pragma once

#include "Console.h"
#include <boost/serialization/access.hpp>


namespace Hilltop {
namespace Console {

class BufferedConsole : public Console {
protected:
    BufferedConsole(unsigned short width, unsigned short height);

public:
    struct pixel_t {
    private:
        friend class boost::serialization::access;
        template<class Archive>
        void serialize(Archive &ar, const unsigned int version) {
            ar & ch;
            ar & color;
        }

    public:
        wchar_t ch;
        ConsoleColor color;
    };

    virtual pixel_t get(unsigned short x, unsigned short y) const = 0;
    virtual void set(unsigned short x, unsigned short y, wchar_t ch, ConsoleColor color) override;
    virtual void set(unsigned short x, unsigned short y, wchar_t ch, ConsoleColor color,
        ConsoleColorType colorMask) = 0;
};

}
}
