#pragma once

#include <vector>

namespace Hilltop {
    namespace Console {

        enum ConsoleColor {
            BLACK = 0,
            DARK_BLUE = 1,
            DARK_GREEN = 2,
            DARK_CYAN = 3,
            DARK_RED = 4,
            DARK_MAGENTA = 5,
            DARK_YELLOW = 6,
            GRAY = 7,
            DARK_GRAY = 8,
            BLUE = 9,
            GREEN = 10,
            CYAN = 11,
            RED = 12,
            MAGENTA = 13,
            YELLOW = 14,
            WHITE = 15,
        };

        enum ConsoleColorType {
            FOREGROUND_COLOR = 0xf,
            BACKGROUND_COLOR = 0xf0,
            FOREGROUND_SHIFT = 0,
            BACKGROUND_SHIFT = 4,
            MAX_COLOR_VALUE = FOREGROUND_COLOR | BACKGROUND_COLOR
        };

        ConsoleColor make_color(ConsoleColor background, ConsoleColor foreground);
        ConsoleColor make_bg_color(ConsoleColor background);
        ConsoleColor make_fg_color(ConsoleColor foreground);


        class ConsoleBuffer {
        private:
            typedef struct {
                char ch;
                uint8_t color;
            } pixel_t;

            std::vector<pixel_t> buffer;

        public:
            const unsigned short width, height;

            ConsoleBuffer(unsigned short width, unsigned short height);

            void set(unsigned short x, unsigned short y, char ch, ConsoleColor color,
                ConsoleColorType colorMask = (ConsoleColorType)(BACKGROUND_COLOR | FOREGROUND_COLOR));
            void clear(ConsoleColor color);

            void commit();
        };

        typedef ConsoleBuffer BufferedConsole;
    }
}
