#pragma once

#include <Windows.h>
#include "Console.h"

namespace Hilltop {
	namespace Console {

		class WindowsConsole final : public BufferedConsole {
		private:
			std::vector<CHAR_INFO> buffer = std::vector<CHAR_INFO>(width * height);

		public:
			const HANDLE handle;

			WindowsConsole(HANDLE handle, unsigned short width, unsigned short height, unsigned short minFont = 6);

			virtual pixel_t get(unsigned short x, unsigned short y) const;
			virtual void set(unsigned short x, unsigned short y, wchar_t ch, ConsoleColor color,
				ConsoleColorType colorMask = (ConsoleColorType)(BACKGROUND_COLOR | FOREGROUND_COLOR));

			virtual void commit() const override;
		};
	}
}
