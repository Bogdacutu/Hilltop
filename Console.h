#pragma once

#include <boost/serialization/access.hpp>
#include <memory>
#include <vector>

namespace Hilltop {
    namespace Console {

        enum ConsoleColor : unsigned char {
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

        enum ConsoleColorType : unsigned char {
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

        bool is_bright_color(ConsoleColor color);
        bool is_dark_color(ConsoleColor color);


        class Console : public std::enable_shared_from_this<Console> {
        protected:
            Console(unsigned short width, unsigned short height);

        public:
            const unsigned short width, height;

            virtual ~Console() {}

            virtual void set(unsigned short x, unsigned short y, wchar_t ch, ConsoleColor color) = 0;
            virtual void clear(ConsoleColor color);

            virtual void commit() const {}
        };


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


        class BufferedNativeConsole : public BufferedConsole {
        protected:
            BufferedNativeConsole(unsigned short width, unsigned short height);

        public:
            virtual void configure();
        };


        class BufferedConsoleRegion : public BufferedConsole {
        protected:
            BufferedConsoleRegion(BufferedConsole &console, unsigned short width, unsigned short height,
                unsigned short x, unsigned short y);

        public:
            const std::shared_ptr<BufferedConsole> console;
            const unsigned short x, y;

            bool enforceBounds = true;

            static std::shared_ptr<BufferedConsoleRegion> create(BufferedConsole &console,
                unsigned short width, unsigned short height, unsigned short x, unsigned short y);

            virtual pixel_t get(unsigned short x, unsigned short y) const override;
            virtual void set(unsigned short x, unsigned short y, wchar_t ch, ConsoleColor color) override;
            virtual void set(unsigned short x, unsigned short y, wchar_t ch, ConsoleColor color,
                ConsoleColorType colorMask) override;
        };


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
