#pragma once

#include "Console.h"
#include <functional>
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
            bool active = false;

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


        class TextBox : public Element {
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


        class Button final : public TextBox {
        protected:
            Button();

            virtual void handleDraw(Console::BufferedConsoleRegion &region) override;

        public:
            std::function<void()> handler;

            Console::ConsoleColor backgroundColor = Console::ConsoleColor::BLACK;

            static std::shared_ptr<Button> create();

            void onClick();
        };
    }
}
