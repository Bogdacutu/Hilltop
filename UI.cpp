#include "UI.h"
#include <algorithm>
#include <Windows.h>

using namespace Hilltop::Console;
using namespace Hilltop::UI;



//
// Element
//

Hilltop::UI::Element::Element() {}

void Hilltop::UI::Element::handleDraw(Console::BufferedConsoleRegion &region) const {}

Hilltop::UI::Element::~Element() {}

std::shared_ptr<Element> Hilltop::UI::Element::create() {
    return std::shared_ptr<Element>(new Element());
}

void Hilltop::UI::Element::draw(Console::BufferedConsole &console) const {
    std::shared_ptr<BufferedConsoleRegion> region =
        BufferedConsoleRegion::create(console, width, height, x, y);
    handleDraw(*region);
}



//
// ElementCollection
//

Hilltop::UI::ElementCollection::ElementCollection() : Element() {}

void Hilltop::UI::ElementCollection::handleDraw(Console::BufferedConsoleRegion &region) const {
    Element::handleDraw(region);

    if (drawBackground)
        region.clear(backgroundColor);

    for (const std::shared_ptr<Element> &el : children)
        el->draw(region);
}

const std::vector<std::shared_ptr<Element>> &Hilltop::UI::ElementCollection::getChildren() const {
    return children;
}

void Hilltop::UI::ElementCollection::addChild(Element &element) {
    children.push_back(element.shared_from_this());
}

void Hilltop::UI::ElementCollection::removeChild(Element &element) {
    std::shared_ptr<Element> el = element.shared_from_this();
    std::vector<std::shared_ptr<Element>>::iterator it = std::find(children.begin(), children.end(), el);
    if (it != children.end())
        children.erase(it);
}

std::shared_ptr<ElementCollection> Hilltop::UI::ElementCollection::create() {
    return std::shared_ptr<ElementCollection>(new ElementCollection());
}



//
// TextBox
//

Hilltop::UI::TextBox::TextBox() : Element() {}

void Hilltop::UI::TextBox::handleDraw(Console::BufferedConsoleRegion &region) const {
    Element::handleDraw(region);

    TextBoxSize size = printText(nullptr, 0, 0, width, height, text, color, alignment, wordWrap);
    printText(&region, (height - size.lines) / 2, 0, width, height, text, color, alignment, wordWrap);
}

std::shared_ptr<TextBox> Hilltop::UI::TextBox::create() {
    return std::shared_ptr<TextBox>(new TextBox());
}



//
// Button
//

Hilltop::UI::Button::Button() : TextBox() {
    alignment = CENTER;
}

void Hilltop::UI::Button::handleDraw(Console::BufferedConsoleRegion &region) const {
    region.clear(backgroundColor);

    TextBox::handleDraw(region);
}

std::shared_ptr<Button> Hilltop::UI::Button::create() {
    return std::shared_ptr<Button>(new Button());
}



//
// ProgressBar
//

Hilltop::UI::ProgressBar::ProgressBar() : Element() {}

void Hilltop::UI::ProgressBar::handleDraw(Console::BufferedConsoleRegion &region) const {
    ConsoleColor c = make_bg_color(color);

    float v = value;
    if (inverted)
        v = 1 - v;
    int start = 0;
    int end = width;
    if (!inverted)
        end = (int)(end * value);
    else
        start = end - (int)(width * value);

    for (int i = start; i < end; i++)
        for (int j = 0; j < height; j++)
            region.set(j, i, L' ', c, BACKGROUND_COLOR);
}

std::shared_ptr<ProgressBar> Hilltop::UI::ProgressBar::create() {
    return std::shared_ptr<ProgressBar>(new ProgressBar());
}



//
// Form
//

static bool findBounds(std::shared_ptr<const Element> needle, std::shared_ptr<const Element> haystack,
    unsigned short *x, unsigned short *y) {
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

static void drawThinRectangle(BufferedConsole &console, unsigned short width, unsigned short height,
    unsigned short x, unsigned short y, ConsoleColor color) {
    ConsoleColor c = make_bg_color(color);

    for (int i = 0; i < width; i++)
        console.set(x, y + i, L'▄', color, FOREGROUND_COLOR);
    for (int i = 1; i < height - 1; i++) {
        console.set(x + i, y, L' ', c);
        console.set(x + i, y + width - 1, L' ', c);
    }
    for (int i = 0; i < width; i++)
        console.set(x + height - 1, y + i, L'▀', color, FOREGROUND_COLOR);
}

static void drawThinOuterRectangle(BufferedConsole &console, unsigned short width, unsigned short height,
    unsigned short x, unsigned short y, ConsoleColor color) {
    ConsoleColor c = make_bg_color(color);

    for (int i = 1; i < width - 1; i++)
        console.set(x, y + i, L'▀', color, FOREGROUND_COLOR);
    for (int i = 0; i < height; i++) {
        console.set(x + i, y, L' ', c);
        console.set(x + i, y + width - 1, L' ', c);
    }
    for (int i = 1; i < width - 1; i++)
        console.set(x + height - 1, y + i, L'▄', color, FOREGROUND_COLOR);
}

Hilltop::UI::Form::Form(int numElements)
    : mapping(numElements), actions(numElements), elements(numElements) {}

bool Hilltop::UI::Form::doAction(KEY_EVENT_RECORD record) {
    event_args_t args{ *this };

    args.type = KEY;
    args.position = currentPos;
    args.record = record;

    if (actions[currentPos])
        return actions[currentPos](args);

    return false;
}

bool Hilltop::UI::Form::doAction(bool focused) {
    event_args_t args{ *this };

    args.type = focused ? FOCUS : BLUR;

    if (actions[currentPos])
        return actions[currentPos](args);

    return false;
}

void Hilltop::UI::Form::doDirectionSwitch(KEY_EVENT_RECORD record, Direction direction) {
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

void Hilltop::UI::Form::switchCurrent(int destination) {
    if (observer)
        observer(currentPos, destination);

    currentPos = destination;
}

void Hilltop::UI::Form::handleKeyEvent(KEY_EVENT_RECORD record) {
    if (!record.bKeyDown)
        return;

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
        }
    }
}

void Hilltop::UI::Form::tick() {
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
            handleKeyEvent(input.Event.KeyEvent);
            break;
        }
    }
}

void Hilltop::UI::Form::draw(Console::BufferedConsole &console, ElementCollection &col) {
    unsigned short x = 0;
    unsigned short y = 0;
    if (findBounds(elements[currentPos], col.shared_from_this(), &x, &y)) {
        if (isFocused) {
            drawThinRectangle(console, elements[currentPos]->width + 2,
                elements[currentPos]->height + 2, x - 1, y - 1, WHITE);
        } else {
            ConsoleColor c = tickCounter % (BLINK_TICKS * 2) < BLINK_TICKS ? GRAY : DARK_GRAY;
            drawThinOuterRectangle(console, elements[currentPos]->width + 4,
                elements[currentPos]->height + 2, x - 1, y - 2, c);
        }
    }
}

void Hilltop::UI::Form::drainInputQueue() {
    HANDLE handle = GetStdHandle(STD_INPUT_HANDLE);

    while (true) {
        DWORD numEvents = 0;
        GetNumberOfConsoleInputEvents(handle, &numEvents);
        if (numEvents <= 0)
            break;

        INPUT_RECORD input = {};
        ReadConsoleInput(handle, &input, 1, &numEvents);
    }
}
