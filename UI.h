#pragma once

#include "Console.h"
#include <functional>
#include <memory>
#include <Windows.h>

namespace Hilltop {
    namespace UI {
        
        class Element : public std::enable_shared_from_this<Element> {
        protected:
            Element();

            virtual void handleDraw(Console::BufferedConsoleRegion &region) const;

        public:
            unsigned short width, height;
            unsigned short x, y;
            bool active = false;

            virtual ~Element();
            static std::shared_ptr<Element> create();

            void draw(Console::BufferedConsole &console) const;
        };


        class ElementCollection final : public Element {
        protected:
            ElementCollection();

            std::vector<std::shared_ptr<Element>> children;

            virtual void handleDraw(Console::BufferedConsoleRegion &region) const override;

        public:
            bool drawBackground = false;
            Console::ConsoleColor backgroundColor = Console::ConsoleColor::BLACK;

            const std::vector<std::shared_ptr<Element>> &getChildren() const;
            void addChild(Element &element);
            void removeChild(Element &element);

            static std::shared_ptr<ElementCollection> create();
        };


        class TextBox : public Element {
        protected:
            TextBox();

            virtual void handleDraw(Console::BufferedConsoleRegion &region) const override;

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

            virtual void handleDraw(Console::BufferedConsoleRegion &region) const override;

        public:
            Console::ConsoleColor backgroundColor = Console::ConsoleColor::BLACK;

            static std::shared_ptr<Button> create();
        };


        class ProgressBar final : public Element {
        protected:
            ProgressBar();

            virtual void handleDraw(Console::BufferedConsoleRegion &region) const override;

        public:
            float value = 0.5;
            bool inverted = false;
            Console::ConsoleColor color = Console::ConsoleColor::WHITE;

            static std::shared_ptr<ProgressBar> create();
        };



        class Form {
        public:
            enum Direction {
                NONE = 0,
                UP,
                DOWN,
                LEFT,
                RIGHT,
            };

            enum EventType {
                FOCUS,
                BLUR,
                KEY,
            };

            static const int NO_ACTION = -1;
            static const int ACT_WITHOUT_FOCUS = -2;

            static const int BLINK_TICKS = 7;

            typedef struct {
                int top = NO_ACTION;
                int bottom = NO_ACTION;
                int left = NO_ACTION;
                int right = NO_ACTION;
            } mapping_t;

            typedef struct {
                EventType type;
                int position;
                KEY_EVENT_RECORD record;
            } event_args_t;

            std::vector<mapping_t> mapping;
            std::vector<std::function<void(event_args_t)>> actions;
            std::vector<std::shared_ptr<Element>> elements;
            std::function<void(int, int)> observer;

            unsigned long long tickCounter = 0;
            int currentPos = 0;
            bool isFocused = false;

            Form(int numElements);

            void doAction(KEY_EVENT_RECORD record);
            void doAction(bool focused);
            void doDirectionSwitch(KEY_EVENT_RECORD record, Direction direction);
            void switchCurrent(int destination);
            void handleKeyEvent(KEY_EVENT_RECORD record);

            void tick();
            void draw(Console::BufferedConsole &console, ElementCollection &elements);
        };
    }
}
