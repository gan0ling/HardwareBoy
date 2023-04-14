#ifndef _EventQueue_EventQueue_h
#define _EventQueue_EventQueue_h

#include <CtrlLib/CtrlLib.h>
#include "eventpp/eventqueue.h"

using namespace Upp;

namespace Seven {


	enum class EventType{
		//cmd
		evCmd,
		//input raw text data
		evRawInput,
        //input raw hwx data
        evRawHexInput,
        //lines
        evTextLine,
		//highlight lines
		evTextHighlight,
		//search text
		evSearchText,
		//send raw data
		evRawSend,
	};
    class MyTime: public Time {
      public:
        MyTime()                   { hour = minute = second = 0; }
        MyTime(const Nuller&)      { hour = minute = second = 0; }
        MyTime(int y, int m, int d, int h = 0, int n = 0, int s = 0, int ms=0)
            { day = d; month = m; year = y; hour = h; minute = n; second = s; milliseconds=ms;}
      private:
        int milliseconds;
    };
    MyTime MyGetSysTime();

	// This is the base event. It has a getType function to return the actual event type.
	class Event {
		public:
			explicit Event(const EventType type): type(type), t(MyGetSysTime()) {
			}

			virtual ~Event() {
			}

			EventType getType() const {
				return type;
			}

		private:
			EventType type;
            MyTime t;
	};

	class CmdEvent : public Event {
		public:
		  explicit CmdEvent(Upp::Id id, int cmd)
				: Event(EventType::evCmd),
				  target(id),
				  cmd(cmd)
			{

			}
			const int Cmd() const {return cmd;}
			const Upp::Id& Id() const {return target;}
		private:
			Upp::Id target;
			int cmd;
				
	};

	class RawInputEvent : public Event {
		public:
			explicit RawInputEvent(byte *raw, size_t size) :
				Event(EventType::evRawInput),size(size) {
					//copy data form raw 
					data.Alloc(size);
					memcpy(~data, raw, size);
				}
			const byte *Get() const { return data.Get();}
            size_t Size() const { return size;}
		private:
            size_t size;
			Buffer<byte> data;
	};

	class RawSendEvent: public Event {
	public:
		explicit RawSendEvent(const char *raw, size_t size) : Event(EventType::evRawSend), size(size) {
			//copy data from raw
			data.Alloc(size);
			memcpy(~data, raw, size);
		}
		const byte *Get() const { return data.Get();}
		size_t Size() const { return size;}
	private:
		size_t size;
		Buffer<byte> data;
	};

	class RawHexInputEvent : public Event {
		public:
            explicit RawHexInputEvent(byte *raw, size_t size) :
				Event(EventType::evRawHexInput), size(size) {
					//copy data form raw 
					data.Alloc(size);
					memcpy(~data, raw, size);
				}
			const byte *Get() const { return data.Get();}
            size_t Size() const { return size;}
		private:
            size_t size;
			Buffer<byte> data;
	};
    
    class TextLineEvent: public Event {
      public:
        explicit TextLineEvent(String &line): Event(EventType::evTextLine), line(line) {}
        const String &Line() const {return line;}
      private:
        String line;
    };

	class TextHighlightEvent: public Event {
		public:
			explicit TextHighlightEvent(String &line): Event(EventType::evTextHighlight), line(line) {}
			explicit TextHighlightEvent(const String &line): Event(EventType::evTextHighlight), line(line) {}
			const String &Line() const { return line;}
		
		private:
			String line;
	};

	class SearchTextEvent: public Event {
		public:
			explicit SearchTextEvent(String &text) : Event(EventType::evSearchText), search(text) {}
			const String &text() const {return search;}
		private:
			String search;
	};
    
	// We will pass event as EventPointer, here it's std::shared_ptr<Event>.
	// It allows EventQueue to store the events in internal buffer without slicing the objects
	// in asynchronous API (EventQueue::enqueue and EventQueue::process, etc).
	// If we only use the synchronous API (EventDispatcher, or EventQueue::dispatch),
	// we can dispatch events as reference.
	using EventPointer = std::shared_ptr<Event>;
	// We are going to dispatch event objects directly without specify the event type explicitly,
	// so we need to define policy to let eventpp know how to get the event type from the event object.
	struct EventPolicy
	{
		static EventType getEvent(const EventPointer & event) {
			return event->getType();
		}
	};

	using EQ = eventpp::EventQueue<EventType, void(const EventPointer &), EventPolicy>;
	typedef EQ::Handle EVHandle;

	void EVProcess(EQ &q);
	EQ& EVGetGlobalQueue();
};

#endif
