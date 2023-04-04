#ifndef _HardwareBoy_HardwareBoy_h
#define _HardwareBoy_HardwareBoy_h

#include <CtrlLib/CtrlLib.h>

#include "CSerialPort/SerialPort.h"
#include "plugins/SerialSetting/SerialSetting.h"
#include "TerminalCtrl/Terminal/Terminal.h"

#define READ_BUFF_SIZE  (1024)

using namespace Upp;

#define LAYOUTFILE <HardwareBoy/HardwareBoy.lay>
#include <CtrlCore/lay.h>

class HardwareBoy : public WithHardwareBoyLayout<TopWindow>, itas109::CSerialPortListener {
public:
  typedef HardwareBoy CLASSNAME;
	HardwareBoy();
  virtual void onReadEvent(const char *portName, unsigned int readBufferLen);

private:
  char m_readBuf[READ_BUFF_SIZE]; 
  SerialSetting m_serialSetting;
  itas109::CSerialPort m_serDev;


  //menu 
  void SetupMenu(Bar &bar);
  void FileMenu(Bar &bar);
  // void PluginMenu(Bar &bar);
  void SettingMenu(Bar &bar);
  
  //serial
  void RunSerialConfig(void);
  void OpenClose(void);
};

#endif
