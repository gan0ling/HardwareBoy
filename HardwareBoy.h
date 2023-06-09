#ifndef _HardwareBoy_HardwareBoy_h
#define _HardwareBoy_HardwareBoy_h

#include <CtrlLib/CtrlLib.h>

#include "CSerialPort/SerialPort.h"
#include "plugins/SerialSetting/SerialSetting.h"
#include "plugins/TerminalCtl/Terminal/Terminal.h"
#include "plugins/EventQueue/EventQueue.h"
#include "plugins/Highlighter/Highlighter.hpp"
#include "utils.h"
#include "plugins/SendCheck/SendCheck.h"

#define READ_BUFF_SIZE  (1024)

using namespace Upp;
using namespace Seven;

#define LAYOUTFILE <HardwareBoy/HardwareBoy.lay>
#include <CtrlCore/lay.h>

enum RecvMode {
  TEXT_MODE = 0,
  HEX_MODE = 1,
};

namespace Seven {
  class SearchBox: public WithSearchLayout<TopWindow> {
  public:
      typedef SearchBox CLASSNAME;
      SearchBox();
      ~SearchBox();
      virtual bool Key(dword key, int);
      void Close();
  private:
      void OnTimeout(void);
  };
class HardwareBoy : public WithHardwareBoyLayout<TopWindow>, itas109::CSerialPortListener {
public:
  typedef HardwareBoy CLASSNAME;
  HardwareBoy();
  ~HardwareBoy();
  virtual void onReadEvent(const char *portName, unsigned int readBufferLen);

private:
  enum RecvMode m_curRecvMode;
  byte m_readBuf[READ_BUFF_SIZE]; 
  SerialSetting m_serialSetting;
  itas109::CSerialPort m_serDev;
  RawDataToLine m_rawtoline;
  CommonHighlighter m_highlighter;
  RawDataToLog m_rawtolog;
  SearchBox m_searchBox;
  SendCheckCtrl m_sendCheckCtrl;


  //menu 
  void SetupMenu(Bar &bar);
  void FileMenu(Bar &bar);
  void PluginMenu(Bar &bar);
  void SettingMenu(Bar &bar);
  
  //serial
  void RunSerialConfig(void);
  void OpenClose(void);
  void SwitchHexTextMode();
  void SerialWrite(const EventPointer &ev);
  void SendToSerial(void);

  //terminal receive 
  void DisplayText(const EventPointer &ev);
  void Search(const EventPointer &ev);
  void SearchTerm(void);

  void RunSendCheck(void);
};

};

#endif
