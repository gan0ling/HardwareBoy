#include "Highlighter.hpp"
#include "HardwareBoy/utils.h"
#include "pretty.hpp"

using namespace Upp;

namespace Seven{

CommonHighlighter::CommonHighlighter()
{
    loadSetting();
}

void CommonHighlighter::Enable(bool enable)
{
    EQ &q = EVGetGlobalQueue();
    if (enable) {
        //
        m_handle = q.appendListener(EventType::evTextLine, THISBACK(Highlight));
    } else {
        q.removeListener(EventType::evTextLine, m_handle);
    }
}

#if 0
static int ConvertColor(String color)
{
    // COLOR_BLACK = 0,
    // COLOR_RED,
    // COLOR_GREEN,
    // COLOR_YELLOW,
    // COLOR_BLUE,
    // COLOR_MAGENTA,
    // COLOR_CYAN,
    // COLOR_WHITE,
    // COLOR_LTBLACK,
    // COLOR_LTRED,
    // COLOR_LTGREEN,
    // COLOR_LTYELLOW,
    // COLOR_LTBLUE,
    // COLOR_LTMAGENTA,
    // COLOR_LTCYAN,
    // COLOR_LTWHITE,
    // COLOR_INK,
    // COLOR_INK_SELECTED,
    // COLOR_PAPER,
    // COLOR_PAPER_SELECTED,
    // MAX_COLOR_COUNT
    color = TrimBoth(ToLower(color));
    if (color == "black") {
        return TerminalCtrl::Colors::COLOR_BLACK;
    } 
    if (color == "red") {
        return TerminalCtrl::Colors::COLOR_RED;
    } 
    if (color == "green") {
        return TerminalCtrl::Colors::COLOR_GREEN;
    }
    if (color == "yellow") {
        return TerminalCtrl::Colors::COLOR_YELLOW;
    }
    if (color == "blue") {
        return TerminalCtrl::Colors::COLOR_BLUE;
    }
    if (color == "magenta") {
        return TerminalCtrl::Colors::COLOR_MAGENTA;
    }
    if (color == "cyan") {
        return TerminalCtrl::Colors::COLOR_CYAN;
    }
    if (color == "white") {
        return TerminalCtrl::Colors::COLOR_WHITE;
    }
    // COLOR_LTBLACK,
    if (color == "ltblack") {
        return TerminalCtrl::Colors::COLOR_LTBLACK;
    }
    if (color == "ltred") {
        return TerminalCtrl::Colors::COLOR_LTRED;
    } 
    if (color == "ltgreen") {
        return TerminalCtrl::Colors::COLOR_LTGREEN;
    }
    if (color == "ltyellow") {
        return TerminalCtrl::Colors::COLOR_LTYELLOW;
    }
    if (color == "ltblue") {
        return TerminalCtrl::Colors::COLOR_LTBLUE;
    }
    if (color == "ltmagenta") {
        return TerminalCtrl::Colors::COLOR_LTMAGENTA;
    }
    if (color == "ltcyan") {
        return TerminalCtrl::Colors::COLOR_LTCYAN;
    }
    if (color == "ltwhite") {
        return TerminalCtrl::Colors::COLOR_LTWHITE;
    }
    // COLOR_INK,
    if (color == "ink") {
        return TerminalCtrl::Colors::COLOR_INK;
    }
    // COLOR_INK_SELECTED,
    if (color == "ink_selected") {
        return TerminalCtrl::Colors::COLOR_INK_SELECTED;
    }
    // COLOR_PAPER,
    if (color == "paper") {
        return TerminalCtrl::Colors::COLOR_PAPER;
    }
    // COLOR_PAPER_SELECTED,
    if (color == "paper_selected") {
        return TerminalCtrl::Colors::COLOR_PAPER_SELECTED;
    }

    return TerminalCtrl::Colors::COLOR_WHITE;
}
#endif

void CommonHighlighter::loadSetting()
{
    MyTextSettings &settings = GetGlobalSetting();
    const VectorMap<String, String> &mySetting = settings.GetAllInGroup("highlighter_setting");

    // case_sensitive = false
    // enable_bracket = true
    // enable_colon = true
    // enable_comma = true
    String v = TrimBoth(ToLower(mySetting.Get("case_sensitive")));
    if (v.GetCount()) {
        m_casesensitive = v == "1" || v == "yes" || v == "true" || v == "y";
    } else {
        m_casesensitive = false;
    }
    v.Clear();

    v = TrimBoth(ToLower(mySetting.Get("enable_bracket")));
    if (v.GetCount()) {
        m_enable_bracket = v == "1" || v == "yes" || v == "true" || v == "y";
    } else {
        m_enable_bracket = true;
    }

    v = TrimBoth(ToLower(mySetting.Get("enable_colon")));
    if (v.GetCount()) {
        m_enable_colon = v == "1" || v == "yes" || v == "true" || v == "y";
    } else {
        m_enable_colon = true;
    }

    v = TrimBoth(ToLower(mySetting.Get("enable_comma")));
    if (v.GetCount()) {
        m_enable_comma = v == "1" || v == "yes" || v == "true" || v == "y";
    } else {
        m_enable_comma = true;
    }

    //create color map
    const VectorMap<String, String> &colorMap = settings.GetAllInGroup("highlighter_color");
    if (colorMap.IsEmpty()) {
        return;
    }
    for (String key : colorMap.GetKeys()) {
        m_colorMap(key, colorMap.Get(key));
    }
}

Vector<String> CommonHighlighter::SplitWords(const String &s)
{
    Vector<String> ret;

    String tmp;    
    for (char c: s) {
        if ((c == '[') || (c == ']') || (c == ' ') || (c == ',')) {
            //word
            if (tmp.GetCount()) {
                ret << tmp;
                tmp.Clear();
            }
            //push delim
            tmp << c;
            ret << tmp;
            tmp.Clear();
        } else {
            tmp << c;
        }
    }
    if (!tmp.GetCount()) {
        ret << tmp;
    }
    return ret;
}

void CommonHighlighter::Highlight(const EventPointer &ev)
{
    //接收evTextLine， 输出evTextHighlight
    EQ &q = EVGetGlobalQueue();

    const TextLineEvent* event = static_cast<const TextLineEvent*>(ev.get());

    if (m_colorMap.IsEmpty())
    {
        //empty keywords, push event back 
        q.enqueue(EventType::evTextHighlight, std::make_shared<TextHighlightEvent>(event->Line()));
        return;
    }

    String ret; 

    Vector<String> words = SplitWords(event->Line());
    DUMP(words);
    const Index<String>& colorIndex = m_colorMap.GetIndex();
    for (auto w : words) {
        //TODO: case sensitive
        String ww = ToLower(w);
        DUMP(ww);
        int idx = colorIndex.Find(ww);
        if (idx >= 0) {
            //found
            // int color = m_colorMap.Get(idx);
            ret << pretty::paint(w, m_colorMap[idx]);
        } else {
            //not found
            ret << w;
        }
    }
    ret << "\r\n";
    DUMP(ret);
    q.enqueue(EventType::evTextHighlight, std::make_shared<TextHighlightEvent>(ret));
}

CommonHighlighter::~CommonHighlighter()
{
    Enable(false);
}

};
