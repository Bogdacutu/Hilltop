#include "Text.h"
#include "BufferedConsoleRegion.h"
#include <algorithm>
#include <deque>
#include <sstream>
#include <vector>


namespace Hilltop {
namespace Console {

TextBoxSize printText(BufferedConsole *buffer, unsigned short x, unsigned short y,
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

}
}
