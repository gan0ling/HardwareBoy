#include "SendCheck.h"
#include "Highlighter/pretty.hpp"
#include <plugin/pcre/Pcre.h>

using namespace Upp;

namespace Seven {

static Id ID_ENABLE("Enable");
static Id ID_HEX("Hex");
static Id ID_DESC("Desc");
static Id ID_SEND("Send");
static Id ID_CHECK("Check");
static Id ID_CNT("Count");
static Id ID_TIME("Time");
static Id ID_NEWLINE("NewLine");
static Id ID_CMD("SenCheckCtrl/Worker");
static Semaphore g_semaphore;

static void SendWorker(Array<struct SendCheckCtrl::SendLineItem> &lines);

SendCheckCtrl::SendCheckCtrl() 
{
    // m_checkHex = false;
    CtrlLayout(*this, "SendCheck Plugin");
    Sizeable().MaximizeBox().MinimizeBox();
    m_testContent.AddColumn(ID_ENABLE, " ").Ctrls<Option>();
    m_testContent.AddColumn(ID_HEX, "Hex").Ctrls<Option>();
    m_testContent.AddColumn(ID_NEWLINE, "\\r\\n").Ctrls<Option>();
    m_testContent.AddColumn(ID_DESC, "Desc").Ctrls<EditString>();
    m_testContent.AddColumn(ID_SEND, "Send").Ctrls<EditString>();
    m_testContent.AddColumn(ID_CHECK, "Check").Ctrls<EditString>();
    m_testContent.AddColumn(ID_CNT, "Count").Ctrls<EditInt>();
    m_testContent.AddColumn(ID_TIME, "Time(s)").Ctrls<EditDouble>();
    m_testContent.ColumnWidths("10 15 15 80 120 120 20 20"); //TODO: adjust
    
    m_fs.Type("Xml file", "*.xml");

    btnLoad << THISBACK(LoadFile);
    btnSave << THISBACK(SaveFile);
    btnSend << THISBACK(SendAll);

    // m_testContent.WhenLeftDouble = THISBACK(SendSingle);
    m_testContent.WhenBar = THISBACK(WhenMenu);

    //TODO: add context menu
    LoadFromFile(m_testContent, ConfigFile("SendCheck.bin"));
}

void SendCheckCtrl::EnableListener(bool enable)
{
  EQ &q = EVGetGlobalQueue();
  if (enable) {
    // m_txtHandle = q.appendListener(EventType::evTextLine, THISBACK(Check));
    // m_hexHandle = q.appendListener(EventType::evRawHexInput, THISBACK(Check));
    m_cmdHandle = q.appendListener(EventType::evCmd, THISBACK(RecvWorkerEv));
  } else {
    // q.removeListener(EventType::evTextLine, m_txtHandle);
    // q.removeListener(EventType::evRawHexInput, m_hexHandle);
    q.removeListener(EventType::evCmd, m_cmdHandle);
  }
}
SendCheckCtrl::~SendCheckCtrl()
{
}

void SendCheckCtrl::WhenMenu(Bar &bar)
{
    m_testContent.StdBar(bar);
    bar.Add("Send", THISBACK(SendSingle));
}
void SendCheckCtrl::SendSingle()
{
    int row = m_testContent.GetClickRow();
    if (row >= 0) {
        //do send
        String send = m_testContent.Get(row, ID_SEND);
        LOG("send:");
        DUMP(send);
    }
}

void SendCheckCtrl::Close()
{
    //save to file
    StoreToFile(m_testContent, ConfigFile("SendCheck.bin"));
    TopWindow::Close();
}
void SendCheckCtrl::LoadFile()
{
    if (!m_fs.ExecuteOpen()) {
        return;
    }
    String filename = m_fs;
    m_testContent.Clear();

    try {
        String d = Upp::LoadFile(filename);
        XmlParser p(d);
        while (!p.IsTag()) {
            p.Skip();
        }
        p.PassTag("SendCheck");
        while (!p.IsEof()) {
            if (p.Tag("SendCheckItem")) {
                bool enable;
                bool hex;
                bool newline;
                String desc;
                String send;
                String check;
                int count;
                double time;
                while (!p.End()) {
                    if (p.Tag("Enable")) {
                        enable = ToLower(p.ReadText()) == "true" ? true : false;
                    } else if (p.Tag("Hex")) {
                        hex = ToLower(p.ReadText()) == "true" ? true : false;
                    } else if (p.Tag("NewLine")) {
                        newline = ToLower(p.ReadText()) == "true" ? true: false;
                    } else if (p.Tag("Desc")) {
                        desc = p.ReadText();
                    } else if (p.Tag("Send")) {
                        send = p.ReadText();
                    } else if (p.Tag("Check")) {
                        check = p.ReadText();
                    } else if (p.Tag("Count")) {
                        count = atoi(p.ReadText());
                    } else if (p.Tag("Time")) {
                        time = atof(p.ReadText());
                    } else {
                        p.Skip();
                        continue;
                    }
                    p.PassEnd();
                }
                m_testContent.Add(enable, hex, newline, desc, send, check, count, time);
            } else {
                p.Skip();
            }
        }
    } catch(XmlError) {
        Exclamation("Error reading input file!");
    }
}

void SendCheckCtrl::SaveFile()
{
    if (!m_fs.ExecuteSaveAs()) {
        return;
    }
    String filename = m_fs;
    String xml;
    for (int i = 0; i < m_testContent.GetCount(); i++) {
        bool enable = m_testContent.Get(i, ID_ENABLE);
        bool hex = m_testContent.Get(i, ID_HEX);
        bool  newline = m_testContent.Get(i, ID_NEWLINE);
        int cnt = m_testContent.Get(i, ID_CNT);
        double time = m_testContent.Get(i, ID_TIME);
        xml << 
            XmlTag("SendCheckItem") (
                XmlTag("Enable").Text(AsString(enable)) +
                XmlTag("Hex").Text(AsString(hex)) +
                XmlTag("NewLine").Text(AsString(newline)) +
                XmlTag("Desc").Text(m_testContent.Get(i, ID_DESC)) +
                XmlTag("Send").Text(m_testContent.Get(i, ID_SEND)) +
                XmlTag("Check").Text(m_testContent.Get(i, ID_CHECK)) + 
                XmlTag("Count").Text(AsString(cnt)) +
                XmlTag("Time").Text(AsString(time))
            );
    }
    if (!Upp::SaveFile(filename, XmlDoc("SendCheck", xml))) {
        Exclamation("Error saving the file!");
    } else {
        Exclamation("Save Successed");
    }
}

void SendCheckCtrl::SendAll()
{
    //send or stop
    if (btnSend.GetLabel() == "Send") {
        //send
        EnableListener(true);
        //setup send data;
        m_lines.Clear();
        for (int i = 0; i < m_testContent.GetCount(); i++) {
            bool enable = m_testContent.Get(i, ID_ENABLE);
            if (enable) {
                struct SendLineItem l; 
                l.index = i;
                l.hex = m_testContent.Get(i, ID_HEX);
                l.newline = m_testContent.Get(i, ID_NEWLINE);
                l.cnt = m_testContent.Get(i, ID_CNT);
                if (l.cnt == INT_NULL) {
                    l.cnt = 1;
                }
                l.time = m_testContent.Get(i, ID_TIME);
                if (l.time == DOUBLE_NULL) {
                    //zero time mean wait forever
                    l.time = 1;
                }
                l.send = AsString(m_testContent.Get(i, ID_SEND));
                l.check = TrimBoth(AsString(m_testContent.Get(i, ID_CHECK)));
                m_lines.Add(l);
            }
        }
        if (m_lines.GetCount()) {
            auto a = Async([=] {
                SendWorker(m_lines);
            });
            m_worker = pick(a);
        }
        btnSend.SetLabel("Stop");
    } else {
        //stop, close worker
        m_worker.Cancel();
        EnableListener(false);
        btnSend.SetLabel("Send");
    }
}

void SendCheckCtrl::RecvWorkerEv(const EventPointer &ev)
{
    static int cnt = 0;
    cnt++;
    const CmdEvent * event = static_cast<const CmdEvent*>(ev.get());
    if (event->Id() != ID_CMD) {
        return;
    }
    GuiLock __;
    int cmd = event->Cmd();
    int row = cmd & 0xFF;
    bool recv_ok = (cmd >> 8) & 0xFF;
    if (cmd == -1) {
        //m_worker exit
        btnSend.SetLabel("Send");
        return;
    }
    if (row >= m_testContent.GetCount()) {
        return;
    }
    if (recv_ok) {
        //recv ok
        EditString *p = static_cast<EditString*> (m_testContent.GetCtrl(row, m_testContent.GetPos(ID_CHECK)));
        p->SetColor(Green());
    } else {
        //send ok
        EditString *p = static_cast<EditString*> (m_testContent.GetCtrl(row, m_testContent.GetPos(ID_SEND)));
        p->SetColor(Green());
        p = static_cast<EditString*> (m_testContent.GetCtrl(row, m_testContent.GetPos(ID_CHECK)));
        p->SetColor(Red());
    }
}

void SendWorker(Array<struct SendCheckCtrl::SendLineItem> &lines)
{
    EQ &q = EVGetGlobalQueue();
    EVHandle check_handle[2];
    String check_recv;
    String highlight;
    bool check_hex = false;

    Vector<int> colors = {
        pretty::MapColor("cyan"),
        pretty::MapColor("underlined"),
        pretty::MapColor("bold"),
    };


    //check recv lambda
    auto check_fn = [&] (const EventPointer &ev) {
        String recv;
        RegExp reg(check_recv);
        if (check_hex && ev->getType() == EventType::evRawHexInput) {
            const RawHexInputEvent *event = static_cast<const RawHexInputEvent*>(ev.get());
            recv = HexEncode(event->Get(), event->Size());
        } 
        if (EventType::evTextLine == ev->getType()) {
            //check string
            const TextLineEvent *event = static_cast<const TextLineEvent *>(ev.get());
            recv = TrimBoth(event->Line());
        }
        DUMP(check_hex);
        DUMP(recv);
        DUMP(check_recv);
        if (!recv.IsEmpty()) {
            // if (check_recv == recv) {
            if (reg.Match(recv)) {
                //found
                g_semaphore.Release();
            } else {
                //maybe not regexp, use nomal match
                if (recv == check_recv) {
                    g_semaphore.Release();
                }
            }
        }
    };

    for (int j = 0; j < lines.GetCount(); j++) {
        struct SendCheckCtrl::SendLineItem &l = lines[j];
        if (l.check.GetLength()) {
            //TODO: start check
            check_recv = l.check;
            if (!(bool)check_handle[0]) {
                //enable listener
                check_handle[0] = q.appendListener(EventType::evTextLine, check_fn);
                check_handle[1] = q.appendListener(EventType::evRawHexInput, check_fn);

            }
        }
        check_hex = l.hex;
        if (!l.hex && l.newline) {
            l.send << "\r\n";
        }
        highlight.Clear();
        highlight = pretty::paint(l.send, colors); 
        //send data 
        for (int i = 0; i < l.cnt; i++) {
            if (Upp::CoWork::IsCanceled()) {
                //quit, close listener
                if ((bool)check_handle[0]) {
                    q.removeListener(EventType::evTextLine, check_handle[0]);
                    q.removeListener(EventType::evTextLine, check_handle[1]);
                }
                break;
            }
            //send data to serial port
            if (l.hex) {
                //send hex
                String hexsend = HexEncode(l.send);
                q.enqueue(EventType::evRawSend, std::make_shared<RawSendEvent>(hexsend.Begin(), hexsend.GetLength()));
                //together send data to display(evTextHighlight)
            } else {
                //send string
                q.enqueue(EventType::evRawSend, std::make_shared<RawSendEvent>(l.send.Begin(), l.send.GetLength()));
            }
            //send data to display
            q.enqueue(EventType::evTextHighlight, std::make_shared<TextHighlightEvent>(highlight));
            
            q.enqueue(EventType::evCmd, std::make_shared<CmdEvent>(ID_CMD, l.index));
            //2 case:
            //  1. we need check recv string, 
            //  2. do not need check
            //anyway, we all need to wait, if time not zero
            if (g_semaphore.Wait(l.time*1000)) {
                //recv and check ok, break loop
                q.enqueue(EventType::evCmd, std::make_shared<CmdEvent>(ID_CMD, l.index | 1<<8));
                break;
            }
        }
    }

    //quit, close listener
    if ((bool)check_handle[0]) {
        q.removeListener(EventType::evTextLine, check_handle[0]);
        q.removeListener(EventType::evTextLine, check_handle[1]);
    }

    //send evcmd to notify gui to change text from "stop" -> "send"
    q.enqueue(EventType::evCmd, std::make_shared<CmdEvent>(ID_CMD, -1));
}

};
