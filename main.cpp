#include "CSerialPort/SerialPort_global.h"
#include "HardwareBoy.h"

using namespace Seven;

HardwareBoy::HardwareBoy():
  m_curRecvMode(TEXT_MODE)
{
  m_serialSetting.Show(true);

  CtrlLayout(*this, "HardwareBoy");
  menu.Set(THISBACK(SetupMenu));
  Sizeable().MaximizeBox().MinimizeBox();
  Maximize();

  btnOpen <<= THISBACK(OpenClose);

  // m_serDev.setReadIntervalTimeout(0);
  int ret = m_serDev.connectReadEvent(this);
  LOG("connectReadEvent:ret " << ret);

  //terminal setting
  term.ShowScrollBar();

  //TODO: remove
  //register evRawInput/evRawHexInput
  EQ &queue = EVGetGlobalQueue();
  // queue.appendListener(EventType::evRawInput, THISBACK(DisplayText));
  // queue.appendListener(EventType::evRawHexInput, THISBACK(DisplayText));
  queue.appendListener(EventType::evTextHighlight, THISBACK(DisplayText));

  //TODO: use config file to enable plugin
  m_rawtoline.Enable(true);
  m_highlighter.Enable(true);
}

HardwareBoy::~HardwareBoy()
{
  ShutdownThreads();
}

void HardwareBoy::SwitchHexTextMode(enum RecvMode newMode)
{
  m_curRecvMode = newMode;
}

void HardwareBoy::OpenClose()
{
  if (btnOpen.GetLabel() == "Open") {
    btnOpen.SetLabel("Close");
    //open serial
    struct SerialPortSetting set = m_serialSetting.GetData();
    if ((set.name.GetLength() == 0) || (set.baudrate == 0)) {
      LOG("invalid port setting");
      PromptOK("invalid port setting");
      return;
    }
    if (m_serDev.isOpen()) {
      m_serDev.close();
    }
    //TODO: 1. buffer size use config file
    //      2. Flow control use ui config
    m_serDev.init(set.name, set.baudrate, set.parity, set.dataBits, set.stopBits, itas109::FlowNone, READ_BUFF_SIZE);
    bool ret = m_serDev.open();
    m_rawtolog.Enable(false);
    m_rawtolog.Enable(true);

    //TODO: use config file
  } else {
    btnOpen.SetLabel("Open");
    //close serial
    m_serDev.close();
  }
}

void HardwareBoy::FileMenu(Bar &menu)
{
  menu.Add("Exit", Breaker(IDOK));
  menu.Separator();

}

void HardwareBoy::SettingMenu(Bar &menu)
{
  menu.Add("Serial", THISBACK(RunSerialConfig));
}

void HardwareBoy::SetupMenu(Bar &menu)
{
  menu.Add("File", THISBACK(FileMenu));
  menu.Add("Setting", THISBACK(SettingMenu));
}

void HardwareBoy::RunSerialConfig()
{
  //FIXME: why need click twice button to close window
  m_serialSetting.Run();
}

static inline int FormatHexDigit(int c) {
	return c < 10 ? c + '0' : c - 10 + 'a';
}

void HardwareBoy::onReadEvent(const char *portName, unsigned int readBufferLen)
{
  //if serial is not open return
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
      //read failed, exit
      return;
    }
    readBufferLen -= len;
    if (m_curRecvMode == HEX_MODE) {
      //change text to hex char
      Buffer<byte> hex(len * 2);
      for (int i = 0; i < readBufferLen; i++) {
        hex[i] = FormatHexDigit(m_readBuf[i] & 0xF);
        hex[i+1] = FormatHexDigit((m_readBuf[i] >> 4) & 0xF);
      }
      queue.enqueue(std::make_shared<RawHexInputEvent>(~hex, len*2));
    } else {
      //Text mode, just enqueue event
      queue.enqueue(std::make_shared<RawInputEvent>((byte*)&m_readBuf[0], len));
    }
  }

    // term.Write(m_readBuf, len, false);
    //use event queue to dispatch event
}

void HardwareBoy::DisplayText(const EventPointer &ev)
{
  // receive evRawInput && evRawHexInput
#if 0
  switch (ev->getType()) {
    case EventType::evRawInput: {
      const RawInputEvent * textEv = static_cast<const RawInputEvent*>(ev.get());
      term.Write(textEv->Get(), textEv->Size(), true);
      break;
    }
    case EventType::evRawHexInput: {
      const RawHexInputEvent * hexEv = static_cast<const RawHexInputEvent*>(ev.get());
      term.Write(hexEv->Get(), hexEv->Size(), true);
      break;
    }
    default: {
      break;
    }
  }
#endif
  const TextHighlightEvent *event = static_cast<const TextHighlightEvent*>(ev.get());
  term.Write(event->Line(), true);
}

GUI_APP_MAIN
{
  //create event queue process thread
  Thread::Start(EVProcess);
  // EQ &queue = EVGetGlobalQueue();
  // queue.appendListener(EventType::evRawInput, [](const EventPointer &ev) {
    // LOG("recv raw input");
  // });
  HardwareBoy().Run();
}
