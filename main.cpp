#include "CSerialPort/SerialPort_global.h"
#include "HardwareBoy.h"

namespace Seven {

HardwareBoy::HardwareBoy() : m_curRecvMode(TEXT_MODE) {
  m_serialSetting.Show(true);

  CtrlLayout(*this, "HardwareBoy");
  menu.Set(THISBACK(SetupMenu));
  Sizeable().MaximizeBox().MinimizeBox();
  Maximize();

  btnOpen <<= THISBACK(OpenClose);
  btnSend <<= THISBACK(SendToSerial);

  // m_serDev.setReadIntervalTimeout(0);
  int ret = m_serDev.connectReadEvent(this);

  // terminal setting

  // TODO: remove
  // register evRawInput/evRawHexInput
  EQ &queue = EVGetGlobalQueue();
  // queue.appendListener(EventType::evRawInput, THISBACK(DisplayText));
  // queue.appendListener(EventType::evRawHexInput, THISBACK(DisplayText));
  queue.appendListener(EventType::evTextHighlight, THISBACK(DisplayText));
  queue.appendListener(EventType::evSearchText, THISBACK(Search));
  queue.appendListener(EventType::evRawSend, THISBACK(SerialWrite));

  // TODO: use config file to enable plugin
  m_rawtoline.Enable(true);
  m_highlighter.Enable(true);
}

HardwareBoy::~HardwareBoy() { ShutdownThreads(); }

void HardwareBoy::SwitchHexTextMode() {
  if (m_curRecvMode == RecvMode::TEXT_MODE) {
    m_curRecvMode = RecvMode::HEX_MODE;
  } else {
    m_curRecvMode = RecvMode::TEXT_MODE;
  }
}

void HardwareBoy::OpenClose() {
  if (btnOpen.GetLabel() == "Open") {
    btnOpen.SetLabel("Close");
    // open serial
    struct SerialPortSetting set = m_serialSetting.GetData();
    if ((set.name.GetLength() == 0) || (set.baudrate == 0)) {
      LOG("invalid port setting");
      PromptOK("invalid port setting");
      return;
    }
    if (m_serDev.isOpen()) {
      m_serDev.close();
    }
    // TODO: 1. buffer size use config file
    //      2. Flow control use ui config
    m_serDev.init(set.name, set.baudrate, set.parity, set.dataBits,
                  set.stopBits, itas109::FlowNone, READ_BUFF_SIZE);
    bool ret = m_serDev.open();
    m_rawtolog.Enable(false);
    m_rawtolog.Enable(true);

    // TODO: use config file
  } else {
    btnOpen.SetLabel("Open");
    // close serial
    m_serDev.close();
  }
}

void HardwareBoy::FileMenu(Bar &menu) {
  menu.Add("Search", THISBACK(SearchTerm));
  menu.Separator();
  menu.Add("Exit", Breaker(IDOK));
}

void HardwareBoy::SettingMenu(Bar &menu) {
  menu.Add("Serial", THISBACK(RunSerialConfig));
  menu.Add("HexMode", THISBACK(SwitchHexTextMode));
}

void HardwareBoy::PluginMenu(Bar &menu) {
  menu.Add("SendCheck", THISBACK(RunSendCheck));
}

void HardwareBoy::SetupMenu(Bar &menu) {
  menu.Add("File", THISBACK(FileMenu));
  menu.Add("Setting", THISBACK(SettingMenu));
  menu.Add("Plugins", THISBACK(PluginMenu));
}

void HardwareBoy::RunSendCheck() { m_sendCheckCtrl.OpenMain(); }
void HardwareBoy::RunSerialConfig() {
  // FIXME: why need click twice button to close window
  m_serialSetting.RefreshPorts();
  // m_serialSetting.Run();
  m_serialSetting.OpenMain();
}

static inline int FormatHexDigit(int c) {
  return c < 10 ? c + '0' : c - 10 + 'A';
}

void HardwareBoy::onReadEvent(const char *portName,
                              unsigned int readBufferLen) {
  // if serial is not open return
  if (!m_serDev.isOpen()) {
    return;
  }

  EQ &queue = EVGetGlobalQueue();

  int len = 0;
  while (readBufferLen > 0) {
    if (readBufferLen > READ_BUFF_SIZE) {
      len = READ_BUFF_SIZE;
    } else {
      len = readBufferLen;
    }
    int readLen = m_serDev.readData(m_readBuf, readBufferLen);
    if (readLen <= 0) {
      // read failed, exit
      return;
    }
    if (m_curRecvMode == RecvMode::HEX_MODE) {
      // change text to hex char
      Buffer<byte> hex(len * 3); //增加空格
      for (int i = 0; i < len; i++) {
        hex[i * 3] = FormatHexDigit((m_readBuf[i] >> 4) & 0xF);
        hex[i * 3 + 1] = FormatHexDigit(m_readBuf[i] & 0xF);
        hex[i * 3 + 2] = ' ';
      }
      queue.enqueue(std::make_shared<RawHexInputEvent>(~hex, len * 3));
    } else {
      // Text mode, just enqueue event
      queue.enqueue(
          std::make_shared<RawInputEvent>((byte *)&m_readBuf[0], len));
    }
    readBufferLen -= len;
  }

  // term.Write(m_readBuf, len, false);
  // use event queue to dispatch event
}

void HardwareBoy::Search(const EventPointer &ev) {
  const SearchTextEvent *event = static_cast<const SearchTextEvent *>(ev.get());
  const String &search = event->text();
  term.SetSearchKeyword(search);
}

void HardwareBoy::SerialWrite(const EventPointer &ev) {
  const RawSendEvent *event = static_cast<const RawSendEvent *>(ev.get());
  m_serDev.writeData(event->Get(), event->Size());
}

void HardwareBoy::DisplayText(const EventPointer &ev) {
  // receive evRawInput && evRawHexInput
#if 0
    switch (ev->getType()){
      case EventType::evRawInput:
        {
          const RawInputEvent* textEv = static_cast<const RawInputEvent*>(ev.get());
          term.Write(textEv->Get(), textEv->Size(), true);
          break;
        }
      case EventType::evRawHexInput:
        {
          const RawHexInputEvent* hexEv = static_cast<const RawHexInputEvent*>(ev.get());
          term.Write(hexEv->Get(), hexEv->Size(), true);
          break;
        }
      default:
        {
          break;
        }
    }
#endif
  const TextHighlightEvent *event =
      static_cast<const TextHighlightEvent *>(ev.get());
  term.Write(event->Line(), true);
}

void HardwareBoy::SearchTerm() {
  m_searchBox.OpenMain();
  m_searchBox.TopMost(true);
}

void HardwareBoy::SendToSerial() {
  String send = docSend.Get();
  EQ &q = EVGetGlobalQueue();
  q.enqueue(EventType::evRawSend, std::make_shared<RawSendEvent>(send.Begin(), send.GetLength()));
}

SearchBox::SearchBox() { CtrlLayout(*this, "Search"); }

SearchBox::~SearchBox() {}
void SearchBox::Close() {
  String nuller;
  EQ &q = EVGetGlobalQueue();
  q.enqueue(EventType::evSearchText, std::make_shared<SearchTextEvent>(nuller));
  TopWindow::Close();
}
bool SearchBox::Key(dword key, int count) {
  bool ret = false;

  if (Upp::ExistsTimeCallback(this)) {
    Upp::KillTimeCallback(this);
  }

  if (key == K_ENTER) {
    //
    EQ &q = EVGetGlobalQueue();
    q.enqueue(EventType::evSearchText,
              std::make_shared<SearchTextEvent>((String)~searchTxt));
    ret = true;
  } else if (key == K_ESCAPE) {
    Close();
  } else {
    // create timer
    Upp::SetTimeCallback(1000, THISBACK(OnTimeout), this);
  }

  return ret;
}

void SearchBox::OnTimeout() {
  EQ &q = EVGetGlobalQueue();
  q.enqueue(EventType::evSearchText,
            std::make_shared<SearchTextEvent>((String)~searchTxt));
}

}; // namespace Seven

GUI_APP_MAIN {
  // create event queue process thread
  Thread::Start(
    [=] {
      EQ &q = EVGetGlobalQueue();
      EVProcess(q);
    }
  );
  // EQ &queue = EVGetGlobalQueue();
  // queue.appendListener(EventType::evRawInput, [](const EventPointer &ev) {
  // LOG("recv raw input");
  // });
  // HardwareBoy().Run();
  HardwareBoy main;
  LoadFromFile(main);
  main.OpenMain();
  Ctrl::EventLoop();
  StoreToFile(main);
}
