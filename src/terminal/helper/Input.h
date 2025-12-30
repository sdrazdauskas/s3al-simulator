#pragma once
#include <string>
#include <mutex>
#include <atomic>
#include <functional>

namespace terminal {

    class Input {
    public:
        using PromptCallback = std::function<std::string()>;
        
        Input() = default;
        
        // Set the prompt callback
        void setPromptCallback(PromptCallback cb);
        
        // Update the internal state (buffer and cursor position)
        void update(const std::string& buffer, size_t cursor);
        
        // Display the buffer with prompt and cursor at correct position
        void display(const std::string& buffer, size_t cursor);
        
        // Redraw the current prompt and buffer (used after log output)
        void redraw();
        
        // Clear the current line
        void clearLine();
        
        // Mark reading state
        void startReading();
        void stopReading();
        bool isCurrentlyReading() const;
        
        // Get current state (thread-safe)
        std::string getBuffer() const;
        size_t getCursor() const;
        
        // Input handlers - modify buffer/cursor and update display
        bool handleBackspace(std::string& buffer, size_t& cursor);
        bool handleCursorMovement(char key, size_t& cursor, size_t bufferSize);
        void handleCharInput(char c, std::string& buffer, size_t& cursor);
        
    private:
        mutable std::mutex mutex;
        std::string buffer;
        size_t cursor{0};
        std::atomic<bool> isReading{false};
        PromptCallback promptCb;
    };

} // namespace terminal
