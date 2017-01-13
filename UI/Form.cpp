#include "UI/Form.h"


namespace Hilltop {
namespace UI {

bool Form::findBounds(std::shared_ptr<const Element> needle,
    std::shared_ptr<const Element> haystack, unsigned short *x, unsigned short *y) {
    if (needle == haystack) {
        *x = needle->x;
        *y = needle->y;
        return true;
    }

    std::shared_ptr<const ElementCollection> col =
        std::dynamic_pointer_cast<const ElementCollection>(haystack);
    if (col) {
        for (const std::shared_ptr<Element> &el : col->getChildren()) {
            if (findBounds(needle, el, x, y)) {
                *x += haystack->x;
                *y += haystack->y;
                return true;
            }
        }
    }

    return false;
}

static void drawThinRectangle(Console::BufferedConsole &console, unsigned short width, unsigned short height,
    unsigned short x, unsigned short y, Console::ConsoleColor color) {
    Console::ConsoleColor c = make_bg_color(color);

    for (int i = 0; i < width; i++)
        console.set(x, y + i, L'▄', color, Console::FOREGROUND_COLOR);
    for (int i = 1; i < height - 1; i++) {
        console.set(x + i, y, L' ', c);
        console.set(x + i, y + width - 1, L' ', c);
    }
    for (int i = 0; i < width; i++)
        console.set(x + height - 1, y + i, L'▀', color, Console::FOREGROUND_COLOR);
}

static void drawThinOuterRectangle(Console::BufferedConsole &console, unsigned short width, unsigned short height,
    unsigned short x, unsigned short y, Console::ConsoleColor color) {
    Console::ConsoleColor c = make_bg_color(color);

    for (int i = 1; i < width - 1; i++)
        console.set(x, y + i, L'▀', color, Console::FOREGROUND_COLOR);
    for (int i = 0; i < height; i++) {
        console.set(x + i, y, L' ', c);
        console.set(x + i, y + width - 1, L' ', c);
    }
    for (int i = 1; i < width - 1; i++)
        console.set(x + height - 1, y + i, L'▄', color, Console::FOREGROUND_COLOR);
}

Form::Form(int numElements)
    : mapping(numElements), actions(numElements), elements(numElements) {}

bool Form::doAction(KEY_EVENT_RECORD record) {
    event_args_t args(this);

    args.type = KEY;
    args.position = currentPos;
    args.record = record;

    if (actions[currentPos])
        return actions[currentPos](args);

    return false;
}

bool Form::doAction(bool focused) {
    event_args_t args(this);

    args.type = focused ? FOCUS : BLUR;
    args.position = currentPos;

    if (actions[currentPos])
        return actions[currentPos](args);

    return false;
}

bool Form::doDefaultAction(KEY_EVENT_RECORD record, std::function<void(event_args_t)> action) {
    event_args_t args;

    args.type = KEY;
    args.record = record;

    if (action) {
        action(args);
        return true;
    }

    return false;
}

void Form::doDirectionSwitch(KEY_EVENT_RECORD record, Direction direction) {
    int destination = NO_ACTION;
    switch (direction) {
    case UP:
        destination = mapping[currentPos].top;
        break;
    case DOWN:
        destination = mapping[currentPos].bottom;
        break;
    case LEFT:
        destination = mapping[currentPos].left;
        break;
    case RIGHT:
        destination = mapping[currentPos].right;
        break;
    }

    if (destination < 0) {
        if (destination == ACT_WITHOUT_FOCUS) {
            doAction(record);
        }
    } else {
        switchCurrent(destination);
    }
}

void Form::switchCurrent(int destination) {
    if (observer)
        observer(currentPos, destination);

    currentPos = destination;
}

void Form::handleKeyEvent(bool active, KEY_EVENT_RECORD record,
    std::function<void(event_args_t)> defaultAction) {
    if (!record.bKeyDown)
        return;

    if (!active) {
        doDefaultAction(record, defaultAction);
        return;
    }

    if (isFocused) {
        switch (record.wVirtualKeyCode) {
        case VK_BACK:
            if (doAction(record))
                break;
        case VK_ESCAPE:
            isFocused = false;
            doAction(false);
            break;
        default:
            doAction(record);
        }
    } else {
        Direction direction = NONE;

        switch (record.wVirtualKeyCode) {
        case VK_UP:
            if (!direction)
                direction = UP;
        case VK_DOWN:
            if (!direction)
                direction = DOWN;
        case VK_LEFT:
            if (!direction)
                direction = LEFT;
        case VK_RIGHT:
            if (!direction)
                direction = RIGHT;
            doDirectionSwitch(record, direction);
            break;
        case VK_SPACE:
        case VK_RETURN:
            isFocused = true;
            doAction(true);
            break;
        default:
            doDefaultAction(record, defaultAction);
        }
    }
}

void Form::tick(bool active, std::function<void(event_args_t)> defaultAction) {
    tickCounter++;

    HANDLE handle = GetStdHandle(STD_INPUT_HANDLE);

    while (true) {
        DWORD numEvents = 0;
        GetNumberOfConsoleInputEvents(handle, &numEvents);
        if (numEvents <= 0)
            break;

        INPUT_RECORD input = {};
        ReadConsoleInput(handle, &input, 1, &numEvents);
        switch (input.EventType) {
        case KEY_EVENT:
            handleKeyEvent(active, input.Event.KeyEvent, defaultAction);
            break;
        }
    }
}

void Form::draw(Console::BufferedConsole &console, ElementCollection &col) {
    unsigned short x = 0;
    unsigned short y = 0;
    if (findBounds(elements[currentPos], col.shared_from_this(), &x, &y)) {
        if (isFocused) {
            drawThinRectangle(console, elements[currentPos]->width + 2,
                elements[currentPos]->height + 2, x - 1, y - 1, Console::WHITE);
        } else {
            Console::ConsoleColor c = tickCounter % (BLINK_TICKS * 2) < BLINK_TICKS ?
                Console::GRAY : Console::DARK_GRAY;
            drawThinOuterRectangle(console, elements[currentPos]->width + 4,
                elements[currentPos]->height + 2, x - 1, y - 2, c);
        }
    }
}

void Form::configureSimpleForm(Form &form) {
    for (int i = 0; i < form.elements.size(); i++) {
        if (i > 0)
            form.mapping[i].left = form.mapping[i].top = i - 1;
        if (i < form.elements.size() - 1)
            form.mapping[i].right = form.mapping[i].bottom = i + 1;
    }
}

void Form::configureMatrixForm(Form &form, int lines, int cols) {
    for (int i = 0; i < lines; i++) {
        const unsigned int idx = i * cols;
        if (i > 0)
            for (int j = 0; j < cols; j++)
                form.mapping[idx + j].top = idx + j - cols;
        if (i < lines - 1)
            for (int j = 0; j < cols; j++)
                form.mapping[idx + j].bottom = idx + j + cols;
        for (int j = 1; j < cols; j++)
            form.mapping[idx + j].left = idx + j - 1;
        for (int j = 0; j < cols - 1; j++)
            form.mapping[idx + j].right = idx + j + 1;
    }
}

}
}
