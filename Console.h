#pragma once

#include <memory>
#include <vector>

namespace Hilltop {
    namespace Console {

        enum ConsoleColor {
            BLACK = 0,
            DARK_BLUE = 1,
            DARK_GREEN = 2,
            BROWN = 3,
            PURPLE = 4,
            ORANGE = 5,
            INDIGO = 6,
            DARK_GRAY = 7,
            GRAY = 8,
            BLUE = 9,
            GREEN = 10,
            RED = 11,
            PINK = 12,
            YELLOW = 13,
            PEACH = 14,
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
        ConsoleColor calc_masked_color(ConsoleColor old, ConsoleColor color, ConsoleColorType mask);


        class Console {
        protected:
            Console(unsigned short width, unsigned short height);

        public:
            const unsigned short width, height;

            virtual ~Console() {}

            virtual void set(unsigned short x, unsigned short y, wchar_t ch, ConsoleColor color,
                ConsoleColorType colorMask = (ConsoleColorType)(BACKGROUND_COLOR | FOREGROUND_COLOR)) = 0;
            virtual void clear(ConsoleColor color);

            virtual void commit() const {}
        };


        class BufferedConsole : public Console {
        protected:
            BufferedConsole(unsigned short width, unsigned short height);

        public:
            typedef struct {
                wchar_t ch;
                ConsoleColor color;
            } pixel_t;

            virtual pixel_t get(unsigned short x, unsigned short y) const = 0;
        };


        class BufferedConsoleRegion final : public BufferedConsole {
        private:
            bool enforceBounds = true;
            std::shared_ptr<BufferedConsole> console;
            unsigned short x, y;

        public:
            BufferedConsoleRegion(std::shared_ptr<BufferedConsole> console, unsigned short width, unsigned short height, unsigned short x, unsigned short y);

            virtual pixel_t get(unsigned short x, unsigned short y) const override;
            virtual void set(unsigned short x, unsigned short y, wchar_t ch, ConsoleColor color,
                ConsoleColorType colorMask = (ConsoleColorType)(BACKGROUND_COLOR | FOREGROUND_COLOR)) override;

            bool enforcingBounds() const;
            void setEnforceBounds(bool value);
        };

        
        class DoublePixelBufferedConsole {
        private:
            std::vector<uint8_t> buffer;

            static const uint8_t BIT_MASKS[2];
            static const int BIT_SHIFTS[2];

        public:
            const unsigned short width, height;

            DoublePixelBufferedConsole(unsigned short width, unsigned short height);

            ConsoleColor get(unsigned short x, unsigned short y) const;
            void set(unsigned short x, unsigned short y, ConsoleColor color);
            void clear(ConsoleColor color);

            void commit(Console &buffer) const;
        };
    }
}