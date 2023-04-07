#ifndef _hardwareboy_utils_h
#define _hardwareboy_utils_h

#include <CtrlLib/CtrlLib.h>
#include "plugins/EventQueue/EventQueue.h"

using namespace Upp;
using namespace Seven;

namespace Seven {
    enum TimerID {
        TIMERID_RAWTOLINE,
    };

    class RawDataToLine 
    {
    public:
        typedef RawDataToLine CLASSNAME;
        RawDataToLine();
        ~RawDataToLine();
        void Enable();
        void Disable();

    private:
        EVHandle m_handle;
        void RawToLine(const EventPointer &ev);
        void RecvTimeout(void);

        String m_cache;
    };

    class MyTextSettings {
        VectorMap< String, VectorMap< String, String > > settings;
        VectorMap<String, String> nullMap;

    public:
        const VectorMap<String, String>& GetAllInGroup(const char * group) const;
        String Get(const char *group, const char *key) const;
        String Get(const char *key) const                            { return Get("", key); }
        String Get(int groupIndex, const char *key) const;
        String Get(int groupIndex, int keyIndex) const;
	
        String operator()(const char *group, const char *key) const  { return Get(group, key); }
        String operator()(const char *key) const                     { return Get(key); }

        void Clear()                                                 { settings.Clear(); }
        void Load(const char *filename);
	
        int GetGroupCount()                                          { return settings.GetCount(); }
        int GetKeyCount(int group)                                   { return settings[group].GetCount(); }
	
        String GetGroupName(int groupIndex)                          { return settings.GetKey(groupIndex); }
        String GetKey(int groupIndex, int keyIndex)                  { return settings[groupIndex].GetKey(keyIndex); }
    };

    MyTextSettings& GetGlobalSetting();
};

#endif
