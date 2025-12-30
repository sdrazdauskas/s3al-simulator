#include "terminal/helper/Input.h"
#include <iostream>

namespace terminal {

    void Input::setPromptCallback(PromptCallback cb) {
        promptCb = std::move(cb);
    }

    void Input::update(const std::string& newBuffer, size_t newCursor) {
        std::lock_guard<std::mutex> lock(mutex);
        buffer = newBuffer;
        cursor = newCursor;
    }

    void Input::display(const std::string& displayBuffer, size_t displayCursor) {
        std::cout << "\r\33[2K";  // Clear current line
        if (promptCb) std::cout << promptCb();
        std::cout << displayBuffer;
        // Move cursor to correct position
        if (displayCursor < displayBuffer.size() && promptCb) {
            std::cout << "\r\33[" << (displayCursor + promptCb().size()) << "C";
        }
        std::cout << std::flush;
    }

    void Input::redraw() {
        if (!isReading.load()) return;
        
        std::lock_guard<std::mutex> lock(mutex);
        // Redraw prompt and current input on a fresh line
        std::cout << "\r\33[2K";  // Clear current line
        if (promptCb) std::cout << promptCb();
        std::cout << buffer;
        // Move cursor to correct position
        if (promptCb && cursor < buffer.size()) {
            std::cout << "\r\33[" << (cursor + promptCb().size()) << "C";
        }
        std::cout << std::flush;
    }

    void Input::clearLine() {
        if (!isReading.load()) return;
        
        // Clear the current line before log output
        std::cout << "\r\33[2K" << std::flush;
    }

    void Input::startReading() {
        std::lock_guard<std::mutex> lock(mutex);
        buffer.clear();
        cursor = 0;
        isReading.store(true);
    }

    void Input::stopReading() {
        isReading.store(false);
    }

    bool Input::isCurrentlyReading() const {
        return isReading.load();
    }

    std::string Input::getBuffer() const {
        std::lock_guard<std::mutex> lock(mutex);
        return buffer;
    }

    size_t Input::getCursor() const {
        std::lock_guard<std::mutex> lock(mutex);
        return cursor;
    }

    bool Input::handleBackspace(std::string& buffer, size_t& cursor) {
        if (buffer.empty() || cursor == 0) return false;
        buffer.erase(cursor - 1, 1);
        --cursor;
        update(buffer, cursor);
        display(buffer, cursor);
        return true;
    }

    bool Input::handleCursorMovement(char key, size_t& cursor, size_t bufferSize) {
        switch (key) {
            case 'C': // RIGHT
                if (cursor >= bufferSize) return false;
                ++cursor;
                std::cout << "\033[C" << std::flush;
                break;
            case 'D': // LEFT
                if (cursor == 0) return false;
                --cursor;
                std::cout << "\033[D" << std::flush;
                break;
            default:
                return false;
        }
        
        update(getBuffer(), cursor);
        return true;
    }

    void Input::handleCharInput(char c, std::string& buffer, size_t& cursor) {
        buffer.insert(buffer.begin() + cursor, c);
        ++cursor;
        update(buffer, cursor);
        display(buffer, cursor);
    }

} // namespace terminal
