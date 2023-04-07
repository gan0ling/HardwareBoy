#include "utils.h"

#define TIME_ID 

namespace Seven {

RawDataToLine::RawDataToLine()
{
  m_cache.Clear();
}

RawDataToLine::~RawDataToLine()
{
  Disable();
}

void RawDataToLine::Enable()
{
  EQ &q = EVGetGlobalQueue();
  m_handle = q.appendListener(EventType::evRawInput, THISBACK(RawToLine));
}

void RawDataToLine::Disable()
{
  EQ &q = EVGetGlobalQueue();
  q.removeListener(EventType::evRawInput, m_handle);
}

void RawDataToLine::RawToLine(const Seven::EventPointer &ev)
{
  EQ &q = EVGetGlobalQueue();

  if (ExistsTimeCallback(this)) {
    KillTimeCallback(this);
    // LOG("RawToLine:Kill Timer");
  }
  //only work with evRawInput
  if (ev->getType() == EventType::evRawInput) {
    const RawInputEvent *event = static_cast<const RawInputEvent*>(ev.get());
    const byte * pdata = event->Get();
    int len = event->Size();
    for (int i = 0; i < len; i++) {
      m_cache.Cat(pdata[i]);
      //3 种情况:
      //1. \r, 2. \r\n 3. \n
      if (pdata[i] == '\n') {
        q.enqueue(EventType::evTextLine, std::make_shared<TextLineEvent>(m_cache));      
        // DUMP(m_cache);
        m_cache.Clear();
        // LOG("RawToLine:\\n");
      }
#if 0
       else if (pdata[i] == '\r') {
        //分两种情况：最后一个字节; 或者下一个字节不是\n，都认为是一行进行上报
        if ((i == (len - 1)) || (pdata[i+1] != '\n')) {
          q.enqueue(EventType::evTextLine, std::make_shared<TextLineEvent>(m_cache));      
          // DUMP(m_cache);
          m_cache.Clear();
          LOG("RawToLine:\\r");
        }
      }
#endif
    }

    //如果tmp 不为空，开启timer，100ms若还没收到换行符则上报
    if (!m_cache.IsEmpty()) {
      SetTimeCallback(100, THISBACK(RecvTimeout), this);
      // DUMP(m_cache);
      // LOG("RawToLine:setup timer");
    }
  }
}

void RawDataToLine::RecvTimeout()
{
  EQ &q = EVGetGlobalQueue();
  q.enqueue(EventType::evTextLine, std::make_shared<TextLineEvent>(m_cache));      
  m_cache.Clear();
  LOG("RawToLine:Timeout");
}


void MyTextSettings::Load(const char *filename)
{
	FileIn in(filename);
	int themei = 0;
	settings.Add("");
	while(!in.IsEof()) {
		String ln = in.GetLine();
		const char *s = ln;
		if(*s == '[') {
			s++;
			String theme;
			while(*s && *s != ']')
				theme.Cat(*s++);
			themei = settings.FindAdd(theme);
		}
		else {
			if(themei >= 0) {
				String key;
				while(*s && *s != '=') {
					key.Cat(*s++);
				}
				if(*s == '=') s++;
				String value;
				while(*s) {
					value.Cat(*s++);
				}
				if(!IsEmpty(key))
					settings[themei].GetAdd(TrimBoth(key)) = TrimBoth(value);
			}
		}
	}
}

String MyTextSettings::Get(const char *group, const char *key) const
{
	int itemi = settings.Find(group);
	return itemi < 0 ? Null : settings.Get(group).Get(key, Null);
}

String MyTextSettings::Get(int groupIndex, const char *key) const
{
	return groupIndex >= 0 && groupIndex < settings.GetCount() ?
	              settings[groupIndex].Get(key, Null) : Null;
}

String MyTextSettings::Get(int groupIndex, int keyIndex) const
{
	if (groupIndex >= 0 && groupIndex < settings.GetCount())
		return keyIndex >= 0 && keyIndex < settings[groupIndex].GetCount() ?
		          settings[groupIndex][keyIndex] : Null;
	else
		return Null;
}

const VectorMap<String, String>& MyTextSettings::GetAllInGroup(const char * group) const
{ 
  int itemi = settings.Find(group); 
  return itemi < 0 ? nullMap: settings.Get(group);
}
MyTextSettings& Seven::GetGlobalSetting()
{
  static bool init = false;
  static MyTextSettings settings;

  if (init == false) {
    settings.Load(ConfigFile("setting.ini"));
  }

  return settings;
}

};
