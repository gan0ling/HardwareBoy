#include "EventQueue.h"

using namespace Seven;


//global queue
static EQ queue;

#if 0
//helper function for register event listener
EVHandle EVAddListener(const EventType &ev, std::function<void(const EventPointer &)> callback)
{
    return queue.appendListener(ev, callback);
}

bool EVDelListener(const EventType &ev, const EVHandle hnd)
{
    return queue.removeListener(ev, hnd);
}
#endif

EQ& Seven::EVGetGlobalQueue()
{
    return queue;
}

void Seven::EVProcess(EQ &q)
{
    while (1) {
        q.waitFor(std::chrono::milliseconds(10));
        if (Thread::IsShutdownThreads()) {
          break;
        }
        q.process();
    }
}

MyTime Seven::MyGetSysTime() 
{
#ifdef PLATFORM_WIN32

	SYSTEMTIME st;
	GetLocalTime(&st);
	return MyTime(st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
#elif define(PLATFORM_POSIX)
    //TODO: correct??
	return Time(time(NULL));
#endif
}

