#include "shell/CommandAPI.h"
#include <ncurses.h>
#include <algorithm>
#include <sstream>
#include <string>
#include <vector>
#include <csignal>

namespace shell {

// Flag for handling Ctrl+C in editor
namespace {
    volatile sig_atomic_t g_texed_sigint = 0;
    
    extern "C" void texed_sigint_handler(int) {
        g_texed_sigint = 1;
    }
}

enum class Mode { NORMAL, INSERT, COMMAND };

// ncurses enumerators (like OK) clash with SysResult enumerators.
constexpr ::shell::SysResult SYS_OK = static_cast<::shell::SysResult>(0);
constexpr ::shell::SysResult SYS_NOTFOUND = static_cast<::shell::SysResult>(2);

struct Editor {
    std::vector<std::string> lines{1, ""};
    std::string filename;
    std::string statusMsg;
    std::string cmdline;
    bool isFileChanged = false;
    bool requestClose = false;

    int cy = 0;
    int cx = 0;                 // cursor coords. 0,0 = top-left
    int rowOff = 0;
    int colOff = 0;
    bool showNumbers = true;   // vim equivalent to ":set number"
    Mode mode = Mode::NORMAL;

    int screenRows = 0;
    int screenCols = 0;

    void setStatus(const std::string& s) { statusMsg = s; }

    void loadFromSys(::shell::SysApi& sys) {
        if (filename.empty()) {
            lines.assign(1, "");
            setStatus("New file");
            return;
        }

        std::string content;
        auto res = sys.readFile(filename, content);
        if (res != SYS_OK) {
            lines.assign(1, "");
            isFileChanged = false;
            setStatus("New file");
            return;
        }

        lines.clear();
        std::istringstream iss(content);
        std::string line;
        while (std::getline(iss, line)) {
            if (!line.empty() && line.back() == '\r')
                line.pop_back();
            lines.push_back(line);
        }

        if (lines.empty())
            lines.push_back("");

        isFileChanged = false;
        setStatus(std::string("Opened ") + filename);
    }

    ::shell::SysResult saveToSys(::shell::SysApi& sys, const std::string& outname) {
        std::ostringstream oss;
        for (size_t i = 0; i < lines.size(); ++i) {
            oss << lines[i];
            if (i + 1 < lines.size())
                oss << '\n';
        }

        std::string content = oss.str();
        auto res = sys.writeFile(outname, content);
        if (res == SYS_NOTFOUND) {
            auto c = sys.createFile(outname);
            if (c != SYS_OK)
                return c;
            res = sys.writeFile(outname, content);
        }

        if (res == SYS_OK) {
            isFileChanged = false;
            filename = outname;
            setStatus(std::string("Wrote ") + outname);
        } else {
            setStatus(std::string("Write failed: ") + ::shell::toString(res));
        }

        return res;
    }

    int clamp(int v, int lo, int hi) {
        if (v < lo) return lo;
        if (v > hi) return hi;
        return v;
    }

    void moveCursor(int dy, int dx) {
        cy = clamp(cy + dy, 0, (int)lines.size() - 1);
        cx = clamp(cx + dx, 0, (int)lines[cy].size());
        // when moving cursor vertically, adjust cx coord of cursor if new line is shorter
        cx = std::min(cx, (int)lines[cy].size());
    }

    void toLineStart() { cx = 0; }
    void toLineEnd()   { cx = (int)lines[cy].size(); }

    void insertChar(int ch) {
        lines[cy].insert(lines[cy].begin() + cx, (char)ch);
        ++cx;
        isFileChanged = true;
    }

    void backspace() {
        if (cx > 0) {
            lines[cy].erase(lines[cy].begin() + cx - 1);
            --cx;
            isFileChanged = true;
        } else if (cy > 0) {
            // merge with previous line
            int prevLen = (int)lines[cy - 1].size();
            lines[cy - 1] += lines[cy];
            lines.erase(lines.begin() + cy);
            --cy;
            cx = prevLen;
            isFileChanged = true;
        }
    }

    void newline() {
        std::string rest = lines[cy].substr(cx);
        lines[cy].erase(cx);
        lines.insert(lines.begin() + cy + 1, rest);
        ++cy;
        cx = 0;
        isFileChanged = true;
    }

    void deleteCharUnderCursor() {
        if (cx < (int)lines[cy].size()) {
            lines[cy].erase(lines[cy].begin() + cx);
            isFileChanged = true;
        } else if (cy + 1 < (int)lines.size()) {
            lines[cy] += lines[cy + 1];
            lines.erase(lines.begin() + cy + 1);
            isFileChanged = true;
        }
    }

    void scrollToCursor() {
        int contentCols = screenCols - gutterWidth();
        if (cy < rowOff) rowOff = cy;
        if (cy >= rowOff + screenRows - 1)
            rowOff = cy - (screenRows - 2);
        rowOff = std::max(0, rowOff);

        if (cx < colOff) colOff = cx;
        if (contentCols < 1) contentCols = 1;
        if (cx >= colOff + contentCols)
            colOff = cx - (contentCols - 1);
        colOff = std::max(0, colOff);
    }

    void runColonCommand(::shell::SysApi& sys) {
        std::string s = cmdline;
        cmdline.clear();

        if (s == "q") {
            if (isFileChanged) {
                setStatus("No write since last change (:q! to force)");
                return;
            }
            requestClose = true;
            return;
        } else if (s == "q!") {
            requestClose = true;
            return;
        } else if (s == "w") {
            saveToSys(sys, filename);
        } else if (s == "set number") {
            showNumbers = true;
            setStatus("number");
        } else if (s == "set nonumber") {
            showNumbers = false;
            setStatus("nonumber");
        } else if (s == "wq") {
            saveToSys(sys, filename);
            requestClose = true;
            return;         
        } else if (s == "help") {
            setStatus("Commands: :w, :q, :q!, :wq, :set number|nonumber | Keys: h j k l i x 0 $ Esc");
        } else {
            setStatus("Not an editor command: :" + s);
        }
    }

    void drawStatusLine() {
        attron(A_REVERSE);
        std::string modeStr = (mode == Mode::INSERT
                                   ? "-- INSERT --"
                                   : mode == Mode::COMMAND ? ":" + cmdline
                                                           : "-- NORMAL --");

        std::ostringstream right;
        right << (filename.empty() ? "[No Name]" : filename)
              << (isFileChanged ? " +" : "  ") << "  " << (cy + 1) << "," << (cx + 1);

        std::string left = modeStr;
        int pad = screenCols - (int)left.size() - 1 - (int)right.str().size();
        if (pad < 1) pad = 1;

        std::string line = left + std::string(pad, ' ') + right.str();
        if ((int)line.size() > screenCols)
            line.resize(screenCols);

        mvaddnstr(screenRows - 1, 0, line.c_str(), screenCols);
        attroff(A_REVERSE);

        move(screenRows - 2, 0);
        clrtoeol();
        if (!statusMsg.empty() && mode == Mode::NORMAL)
            mvaddnstr(screenRows - 2, 0, statusMsg.c_str(), screenCols);
    }

    void drawRows() {
        int gut = gutterWidth();
        int contentCols = std::max(1, screenCols - gut);
        for (int y = 0; y < screenRows - 1; ++y) {
            int fileRow = y + rowOff;
            move(y, 0);
            clrtoeol();

            if (gut > 0) {
                if (fileRow < (int)lines.size()) {
                    char buf[32];
                    int width = std::min(gut - 1, 10);  // Clamp width to reasonable value to avoid overflow
                    int lineNum = std::min(fileRow + 1, 999999999);  // Clamp line number to avoid overflow
                    std::snprintf(buf, sizeof(buf), "%*d ", width, lineNum);
                    attron(A_DIM);
                    addnstr(buf, gut);
                    attroff(A_DIM);
                } else {
                    for (int i = 0; i < gut; ++i)
                        addch(' ');
                }
            }

            if (fileRow >= (int)lines.size())
                continue;

            const std::string& s = lines[fileRow];
            if ((int)s.size() <= colOff)
                continue;

            std::string slice = s.substr(colOff, contentCols);
            mvaddnstr(y, gut, slice.c_str(), contentCols);
        }
    }

    int gutterWidth() const {
        if (!showNumbers)
            return 0;
        int n = (int)lines.size();
        int digits = 1;
        while (n >= 10) {
            n /= 10;
            ++digits;
        }
        return std::max(2, digits) + 1;
    }

    void refreshScreen() {
        getmaxyx(stdscr, screenRows, screenCols);
        if (screenRows < 2)
            return;
        scrollToCursor();
        erase();
        drawRows();
        drawStatusLine();

        int gut = gutterWidth();
        int scrY = std::clamp(cy - rowOff, 0, screenRows - 2);
        int scrX = std::clamp(cx - colOff + gut, gut, screenCols - 1);
        move(scrY, scrX);
        refresh();
    }
};

class TexedCommand : public ICommand {
public:
    int execute(const std::vector<std::string>& args,
                const std::string&,
                std::ostream& out,
                std::ostream& err,
                ::shell::SysApi& sys) override
    {
        if (args.empty()) {
            err << "Usage: " << getUsage() << "\n";
            return 1;
        }

        Editor ed;
        ed.filename = args[0];
        ed.loadFromSys(sys);

        // Enter interactive mode (disables console logging)
        sys.beginInteractiveMode();
        
        // Install our own SIGINT handler to catch Ctrl+C
        auto prev_handler = std::signal(SIGINT, texed_sigint_handler);
        g_texed_sigint = 0;

        initscr();              // init ncurses mode
        raw();                  // pass keypresses DIRECTLY to program
        noecho();               // do not echo typed chars
        keypad(stdscr, true);
        set_escdelay(1);        //default- 1000ms
        curs_set(1);

        ed.setStatus("Press :help for help");

        while (!ed.requestClose) {
            // Handle Ctrl+C: return to normal mode
            if (g_texed_sigint) {
                g_texed_sigint = 0;
                ed.mode = Mode::NORMAL;
                ed.cmdline.clear();
                ed.setStatus("Interrupted");
                continue;
            }
            
            ed.refreshScreen();
            int ch = getch();
            if (ch == KEY_RESIZE)
                continue;

            switch (ed.mode) {
            case Mode::NORMAL:
                if (ch == 'h' || ch == KEY_LEFT)
                    ed.moveCursor(0, -1);
                else if (ch == 'j' || ch == KEY_DOWN)
                    ed.moveCursor(1, 0);
                else if (ch == 'k' || ch == KEY_UP)
                    ed.moveCursor(-1, 0);
                else if (ch == 'l' || ch == KEY_RIGHT)
                    ed.moveCursor(0, 1);
                else if (ch == '0')
                    ed.toLineStart();
                else if (ch == '$')
                    ed.toLineEnd();
                else if (ch == 'x')
                    ed.deleteCharUnderCursor();
                else if (ch == 'i') {
                    ed.mode = Mode::INSERT;
                    ed.setStatus("");
                } else if (ch == ':') {
                    ed.mode = Mode::COMMAND;
                    ed.cmdline.clear();
                } else if (ch == 'G') {
                    ed.cy = (int)ed.lines.size() - 1;
                    ed.cx = 0;
                } else if (ch == 'g') {
                    int ch2 = getch();
                    if (ch2 == 'g') {
                        ed.cy = 0;
                        ed.cx = 0;
                    }
                }
                break;

            case Mode::INSERT:
                if (ch == 27) { // ascii: 27=1B=ESC
                    ed.mode = Mode::NORMAL;
                } else if (ch == KEY_LEFT) {
                    ed.moveCursor(0, -1);
                } else if (ch == KEY_RIGHT) {
                    ed.moveCursor(0, 1);
                } else if (ch == KEY_UP) {
                    ed.moveCursor(-1, 0);
                } else if (ch == KEY_DOWN) {
                    ed.moveCursor(1, 0);
                } else if (ch == KEY_BACKSPACE || ch == 127 || ch == 8) {
                    ed.backspace();
                } else if (ch == '\n' || ch == '\r') {
                    ed.newline();
                } else if (isprint(ch) || (ch >= 32 && ch <= 126)) {
                    ed.insertChar(ch);
                }
                break;

            case Mode::COMMAND:
                if (ch == 27) {
                    ed.mode = Mode::NORMAL;
                } else if (ch == KEY_BACKSPACE || ch == 127 || ch == 8) {
                    if (!ed.cmdline.empty())
                        ed.cmdline.pop_back();
                } else if (ch == '\n' || ch == '\r') {
                    ed.runColonCommand(sys);
                    if (ed.mode == Mode::COMMAND)
                        ed.mode = Mode::NORMAL;
                } else if (isprint(ch)) {
                    ed.cmdline.push_back((char)ch);
                }
                break;
            }
        }

        // ends ncurses mode
        endwin();
        
        // Restore previous signal handler
        std::signal(SIGINT, prev_handler);

        // Exit interactive mode (restores console logging)
        sys.endInteractiveMode();

        return 0;
    }

    const char* getName() const override { return "texed"; }
    const char* getDescription() const override { return "Terminal text editor (ncurses)"; }
    const char* getUsage() const override { return "texed <fileName>"; }
    int getCpuCost() const override { return 10; }
};

std::unique_ptr<ICommand> create_texed_command() { return std::make_unique<TexedCommand>(); }

} // namespace shell