#ifndef _HB_SEND_CHECK_H_
#define _HB_SEND_CHECK_H_

#include <CtrlLib/CtrlLib.h>
#include "EventQueue/EventQueue.h"

#define LAYOUTFILE "SendCheck.lay"
#include <CtrlCore/lay.h>

using namespace Upp;

namespace Seven {
    class SendCheckCtrl: public WithSendCheckLayout<TopWindow> {
    public :
        struct SendLineItem {
            bool hex;
            bool newline;
            int index;
            int cnt;
            double time;
            String send;
            String check;
        };
        typedef SendCheckCtrl CLASSNAME;
        SendCheckCtrl();
        ~SendCheckCtrl();
        void Close();
        void EnableListener(bool);
    private:

        FileSel m_fs;
        // EVHandle m_txtHandle;
        // EVHandle m_hexHandle;
        EVHandle m_cmdHandle;
        // String m_check;
        // bool   m_checkHex;
        Array<struct SendLineItem> m_lines;
        AsyncWork<void> m_worker;

        void LoadFile(void);
        void SaveFile(void);
        void SendAll(void);
        void SendSingle(void);
        void WhenMenu(Bar &bar);

        bool SendAndCheck(int row);

        void Check(const EventPointer &ev);
        void RecvWorkerEv(const EventPointer &ev);
    };
};

#endif
