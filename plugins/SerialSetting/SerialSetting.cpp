#include "SerialSetting.h"
#include "CSerialPort/SerialPortInfo.h"

using namespace Seven;

namespace Seven {
SerialSetting::SerialSetting() {
  // TODO: How to enable edit for baudList
  baudList.Add({
      {0, 110},     {1, 300},     {2, 600},     {3, 1200},    {4, 2400},
      {5, 4800},    {6, 9600},    {7, 14400},   {8, 19200},   {9, 38400},
      {10, 56000},  {11, 57600},  {12, 115200}, {13, 128000}, {14, 230400},
      {15, 256000}, {16, 460800}, {17, 500000}, {18, 512000}, {19, 600000},

  });
  baudList.SetIndex(12);

  // Init Data Bits list
  dataList.Add({{5, 5}, {6, 6}, {7, 7}, {8, 8}});
  dataList.SetIndex(3);

  stopBitsList.Add({{0, 1}, {1, 1.5}, {2, 2}});
  stopBitsList.SetIndex(0);

  parityList.Add(
      {{0, "None"}, {1, "Odd"}, {2, "Even"}, {3, "Mark"}, {4, "Space"}});
  parityList.SetIndex(0);

  // init port list
  RefreshPorts();

  CtrlLayout(*this, "Serial");

  // ok button, close window
  btnOK <<= THISBACK(ok);
  btnCancel <<= THISBACK(Close);

  m_select = false;
}

void SerialSetting::RefreshPorts(void)
{
  portList.Clear();
  auto ports = itas109::CSerialPortInfo::availablePortInfos();
  for (auto p : ports) {
    portList.Add(p.portName);
  }
}

void SerialSetting::ok()
{
  m_select = true;
  Close();
}


struct SerialPortSetting SerialSetting::GetData() {
  struct SerialPortSetting s;
  if (m_select) {
    s.name = portList.GetValue();
    s.baudrate = baudList.GetValue();
    int parity = parityList.GetData();
    switch(parity) {
      case 0:
      default: {
        s.parity = itas109::Parity::ParityNone;
          break;
      }
      case 1: {
        s.parity = itas109::Parity::ParityOdd;
        break;
      }
      case 2: {
        s.parity = itas109::Parity::ParityEven;
        break;
      }
      case 3: {
        s.parity = itas109::Parity::ParityMark;
        break;
      }
      case 4: {
        s.parity = itas109::Parity::ParitySpace;
        break;
      }
    }
    switch (int(dataList.GetData())) {
      case 5: {
        s.dataBits = itas109::DataBits::DataBits5;
        break;
      }
      case 6: {
        s.dataBits = itas109::DataBits::DataBits6;
        break;
      }
      case 7: {
        s.dataBits = itas109::DataBits::DataBits7;
        break;
      }
      default:
      case 8: {
        s.dataBits = itas109::DataBits::DataBits8;
      }
    }
    switch (int(stopBitsList.GetData())) {
      default:
      case 0: {
        s.stopBits = itas109::StopBits::StopOne;
        break;
      }

      case 1: {
        s.stopBits = itas109::StopBits::StopOneAndHalf;
        break;
      }

      case 2: {
        s.stopBits = itas109::StopBits::StopTwo;
        break;
      }
    }
  }
  // else {
  // }

  return s;
}

};

// GUI_APP_MAIN
// {
// 	SerialSetting().Run();
// }
