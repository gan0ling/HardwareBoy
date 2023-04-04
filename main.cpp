#include "CSerialPort/SerialPort_global.h"
#include "HardwareBoy.h"
#include <iostream>

HardwareBoy::HardwareBoy()
{
  m_serialSetting.Show(true);

	CtrlLayout(*this, "HardwareBoy");
  menu.Set(THISBACK(SetupMenu));
  Sizeable().MaximizeBox();
  Maximize();

  btnOpen <<= THISBACK(OpenClose);

  // m_serDev.setReadIntervalTimeout(0);
  int ret = m_serDev.connectReadEvent(this);
  LOG("connectReadEvent:ret " << ret);

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
    DUMP(set.baudrate);
    DUMP(set.name);
    m_serDev.init(set.name, set.baudrate, set.parity, set.dataBits, set.stopBits, itas109::FlowNone, READ_BUFF_SIZE);
    bool ret = m_serDev.open();

    //TODO: use config file
  } else {
    btnOpen.SetLabel("Open");
    //close serial
    m_serDev.close();
    m_serDev.disconnectReadEvent();
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

void HardwareBoy::onReadEvent(const char *portName, unsigned int readBufferLen)
{
  //if serial is not open return
  if (!m_serDev.isOpen()) {
    return;
  }

  //read data, write to term
  int len = 0;
  while (readBufferLen > 0) {
    if (readBufferLen > READ_BUFF_SIZE) {
      len = READ_BUFF_SIZE;
    } else {
      len = readBufferLen;
    }
    m_serDev.readData(m_readBuf, len);
    readBufferLen -= len;
    term.Write(m_readBuf, len);
  }
}

GUI_APP_MAIN
{
	HardwareBoy().Run();
}
