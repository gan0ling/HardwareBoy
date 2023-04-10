#ifndef _SerialSetting_SerialSetting_h
#define _SerialSetting_SerialSetting_h

#include <CtrlLib/CtrlLib.h>
#include "CSerialPort/SerialPort.h"
#include "CSerialPort/SerialPortInfo.h"

using namespace Upp;

#define LAYOUTFILE <SerialSetting/SerialSetting.lay>
#include <CtrlCore/lay.h>

namespace Seven {
  struct SerialPortSetting {
    String name;
    int baudrate;
    itas109::StopBits stopBits;
    itas109::Parity parity;
    itas109::DataBits dataBits;
  };

  class SerialSetting : public WithSerialSettingLayout<TopWindow> {
  public:
    typedef SerialSetting CLASSNAME;
    SerialSetting();

    struct SerialPortSetting GetData();

    void RefreshPorts(void);

  private:
    bool m_select;

    void ok();
    // void cancel();
  };
}

#endif
