#pragma once

#include <Windows.h>
#include "Console.h"

namespace Hilltop {
    namespace Console {

        class WindowsConsole final : public BufferedNativeConsole {
        private:
            std::vector<CHAR_INFO> buffer = std::vector<CHAR_INFO>(width * height);
            unsigned short chosenSize = 0;

        protected:
            WindowsConsole(HANDLE handle, unsigned short width, unsigned short height, unsigned short minFont,
                unsigned short maxFont);

        public:
            const HANDLE handle;
            const unsigned short minFont, maxFont;

            virtual void configure() override;

            static std::shared_ptr<WindowsConsole> create(HANDLE handle, unsigned short width,
                unsigned short height, unsigned short minFont = 6, unsigned short maxFont = 36);

            virtual pixel_t get(unsigned short x, unsigned short y) const;
            virtual void set(unsigned short x, unsigned short y, wchar_t ch, ConsoleColor color,
                ConsoleColorType colorMask = (ConsoleColorType)(BACKGROUND_COLOR | FOREGROUND_COLOR));

            virtual void commit() const override;
        };
    }
}
