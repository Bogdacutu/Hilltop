#pragma once

#include "UI/Element.h"
#include "UI/ElementCollection.h"
#include <functional>
#include <vector>
#include <Windows.h>


namespace Hilltop {
namespace UI {

class Form {
public:
    static bool findBounds(std::shared_ptr<const Element> needle,
        std::shared_ptr<const Element> haystack, unsigned short *x, unsigned short *y);

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

    struct mapping_t {
        int top = NO_ACTION;
        int bottom = NO_ACTION;
        int left = NO_ACTION;
        int right = NO_ACTION;
    };

    struct event_args_t {
        Form *form = nullptr;
        int position;
        EventType type;
        KEY_EVENT_RECORD record = {};

        event_args_t(Form *form = nullptr) : form(form) {}
    };

    std::vector<mapping_t> mapping;
    std::vector<std::function<bool(event_args_t)>> actions;
    std::vector<std::shared_ptr<Element>> elements;
    std::function<void(int, int)> observer;

    unsigned long long tickCounter = 0;
    int currentPos = 0;
    bool isFocused = false;

    Form(int numElements);

    bool doAction(KEY_EVENT_RECORD record);
    bool doAction(bool focused);
    static bool doDefaultAction(KEY_EVENT_RECORD record, std::function<void(event_args_t)> action);
    void doDirectionSwitch(KEY_EVENT_RECORD record, Direction direction);
    void switchCurrent(int destination);
    void handleKeyEvent(bool active, KEY_EVENT_RECORD record,
        std::function<void(event_args_t)> defaultAction);

    void tick(bool active = true, std::function<void(event_args_t)> defaultAction =
        std::function<void(event_args_t)>());
    void draw(Console::BufferedConsole &console, ElementCollection &elements);

    static void configureSimpleForm(Form &form);
    static void configureMatrixForm(Form &form, int lines, int cols);
};

}
}
