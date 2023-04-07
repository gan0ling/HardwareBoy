#ifndef _Highlighter_Highlighter_h
#define _Highlighter_Highlighter_h

#include <CtrlLib/CtrlLib.h>
#include "EventQueue/EventQueue.h"

using namespace Upp;

namespace Seven {
class CommonHighlighter {
    public:
        typedef CommonHighlighter CLASSNAME;
        CommonHighlighter();
        ~CommonHighlighter();
        void Enable(bool enable);
    
    private:
        bool m_casesensitive;
        bool m_enable_bracket;
        bool m_enable_colon;
        bool m_enable_comma;
        EVHandle m_handle;
        VectorMap<String, String> m_colorMap;

        void loadSetting();
        void Highlight(const EventPointer &ev);
        Vector<String> SplitWords(const String &s);
        String ToAnsi(String &value, String &style);
};

};

#endif
