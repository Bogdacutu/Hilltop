#pragma once

#include "Console.h"
#include <memory>

namespace Hilltop {
    namespace UI {
        
        class Element : public std::enable_shared_from_this<Element> {
        protected:
            Element();

            virtual void handleDraw(Console::BufferedConsoleRegion &region);

        public:
            unsigned short width, height;
            unsigned short x, y;

            static std::shared_ptr<Element> create();

            void draw(Console::BufferedConsole &console);
        };


        class ElementCollection final : public Element {
        protected:
            ElementCollection();

            std::vector<std::shared_ptr<Element>> children;

            virtual void handleDraw(Console::BufferedConsoleRegion &region) override;

        public:
            bool drawBackground = false;
            Console::ConsoleColor backgroundColor = Console::ConsoleColor::BLACK;

            const std::vector<std::shared_ptr<Element>> &getChildren();
            void addChild(Element &element);
            void removeChild(Element &element);

            static std::shared_ptr<ElementCollection> create();
        };


        class TextBox final : public Element {
        protected:
            TextBox();

            virtual void handleDraw(Console::BufferedConsoleRegion &region) override;

        public:
            std::string text;
            Console::TextAlignment alignment = Console::TextAlignment::LEFT;
            Console::ConsoleColor color = Console::ConsoleColor::WHITE;
            bool wordWrap = true;

            static std::shared_ptr<TextBox> create();
        };
    }
}
