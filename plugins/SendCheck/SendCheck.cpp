#include "SendCheck.h"
#include "Highlighter/pretty.hpp"

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

SendCheckCtrl::SendCheckCtrl() 
{
    m_checkHex = false;
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
    m_txtHandle = q.appendListener(EventType::evTextLine, THISBACK(Check));
    m_hexHandle = q.appendListener(EventType::evRawHexInput, THISBACK(Check));
  } else {
    q.removeListener(EventType::evTextLine, m_txtHandle);
    q.removeListener(EventType::evRawHexInput, m_hexHandle);
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
    for (int i = 0; i < m_testContent.GetCount(); i++) {
        bool enable = m_testContent.Get(i, ID_ENABLE);
        if (enable) {
            m_testContent.Select(i);
            SendAndCheck(i);
            //set color
        }
    } 
}

void SendCheckCtrl::Check(const EventPointer &ev)
{
  String recv;
  if (m_checkHex && ev->getType() == EventType::evRawHexInput) {
    const RawHexInputEvent *event = static_cast<const RawHexInputEvent*>(ev.get());
    recv = HexEncode(event->Get(), event->Size());
  } 
  if (EventType::evTextLine == ev->getType()) {
    //check string
    const TextLineEvent *event = static_cast<const TextLineEvent *>(ev.get());
    recv = event->Line();
  }
  DUMP(m_check);
  DUMP(recv);
  if (!recv.IsEmpty()) {
    if (m_check == recv) {
      //found
      m_semaphore.Release();
    }
  }
}

bool SendCheckCtrl::SendAndCheck(int row)
{
    EQ &q = EVGetGlobalQueue();
    String send = AsString(m_testContent.Get(row, ID_SEND));
    String check = TrimBoth(AsString(m_testContent.Get(row, ID_CHECK)));
    bool newline = m_testContent.Get(row, ID_NEWLINE);
    bool hex = m_testContent.Get(row, ID_HEX);

    if (!check.IsEmpty()) {
      //need check receive data
      EnableListener(true);
      m_checkHex = hex;
      m_check.Clear();
      m_check = check;
    }
    
    if (!hex && newline) {
      send << "\r\n";
    }
    Vector<int> colors = {
    pretty::MapColor("cyan"),
      pretty::MapColor("underlined"),
      pretty::MapColor("bold"),
    };
    String high = pretty::paint(send, colors); 
    if (hex) {
        //send hex
        String hexsend = HexEncode(send);
        q.enqueue(EventType::evRawSend, std::make_shared<RawSendEvent>(hexsend.Begin(), hexsend.GetLength()));
        //together send data to display(evTextHighlight)
    } else {
        //send string
        q.enqueue(EventType::evRawSend, std::make_shared<RawSendEvent>(send.Begin(), send.GetLength()));
    }
    q.enqueue(EventType::evTextHighlight, std::make_shared<TextHighlightEvent>(high));
    EditString *p = static_cast<EditString*> (m_testContent.GetCtrl(row, m_testContent.GetPos(ID_SEND)));
    p->SetColor(Green());
    if (!check.IsEmpty()) {
      EditString *p = static_cast<EditString*> (m_testContent.GetCtrl(row, m_testContent.GetPos(ID_CHECK)));
      p->SetColor(Red());
    } 
  return true;
}

};
