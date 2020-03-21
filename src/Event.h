// See the file "COPYING" in the main distribution directory for copyright.

#pragma once

#include "BroList.h"
#include "analyzer/Analyzer.h"
#include "iosource/IOSource.h"
#include "Flare.h"

class EventMgr;
template <class T> class IntrusivePtr;

// TODO: no need to derive from BroObj ?
class Event : public BroObj {
public:
	Event(EventHandlerPtr handler, std::vector<IntrusivePtr<Val>> args,
		SourceID src = SOURCE_LOCAL, analyzer::ID aid = 0,
		TimerMgr* mgr = 0, BroObj* obj = 0);

	void SetNext(Event* n)		{ next_event = n; }
	Event* NextEvent() const	{ return next_event; }

	SourceID Source() const		{ return src; }
	analyzer::ID Analyzer() const	{ return aid; }
	TimerMgr* Mgr() const		{ return mgr; }
	EventHandlerPtr Handler() const	{ return handler; }
	const std::vector<IntrusivePtr<Val>>& Args() const	{ return args; }

	void Describe(ODesc* d) const override;

protected:
	friend class EventMgr;

	// This method is protected to make sure that everybody goes through
	// EventMgr::Dispatch().
	void Dispatch(bool no_remote = false);

	EventHandlerPtr handler;
	std::vector<IntrusivePtr<Val>> args;
	SourceID src;
	analyzer::ID aid;
	TimerMgr* mgr;
	BroObj* obj;
	Event* next_event;
};

extern uint64_t num_events_queued;
extern uint64_t num_events_dispatched;

class EventMgr : public BroObj, public iosource::IOSource {
public:
	EventMgr();
	~EventMgr() override;

	// Queues an event without first checking if there's any available event
	// handlers (or remote consumers).  If it turns out there's actually
	// nothing that will consume the event, then this may leak memory due to
	// failing to decrement the reference count of each element in 'vl'.  i.e.
	// use this function instead of QueueEvent() if you've already guarded
	// against the case where there's no handlers (one usually also does that
	// because it would be a waste of effort to construct all the event
	// arguments when there's no handlers to consume them).
	// TODO: deprecate
	/* [[deprecated("Remove in v4.1.  Use IntrusivePtr overload instead.")]] */
	void QueueEventFast(const EventHandlerPtr &h, val_list vl,
			SourceID src = SOURCE_LOCAL, analyzer::ID aid = 0,
			TimerMgr* mgr = 0, BroObj* obj = 0);

	// Queues an event if there's an event handler (or remote consumer).  This
	// function always takes ownership of decrementing the reference count of
	// each element of 'vl', even if there's no event handler.  If you've
	// checked for event handler existence, you may wish to call
	// QueueEventFast() instead of this function to prevent the redundant
	// existence check.
	// TODO: deprecate
	/* [[deprecated("Remove in v4.1.  Use IntrusivePtr overload instead.")]] */
	void QueueEvent(const EventHandlerPtr &h, val_list vl,
			SourceID src = SOURCE_LOCAL, analyzer::ID aid = 0,
			TimerMgr* mgr = 0, BroObj* obj = 0);

	// Same as QueueEvent, except taking the event's argument list via a
	// pointer instead of by value.  This function takes ownership of the
	// memory pointed to by 'vl' as well as decrementing the reference count of
	// each of its elements.
	// TODO: deprecate
	/* [[deprecated("Remove in v4.1.  Use IntrusivePtr overload instead.")]] */
	void QueueEvent(const EventHandlerPtr &h, val_list* vl,
			SourceID src = SOURCE_LOCAL, analyzer::ID aid = 0,
			TimerMgr* mgr = 0, BroObj* obj = 0);

	// TODO: comments
	void QueueUncheckedEvent(const EventHandlerPtr &h,
	                         std::vector<IntrusivePtr<Val>> vl,
	                         SourceID src = SOURCE_LOCAL, analyzer::ID aid = 0,
	                         TimerMgr* mgr = nullptr, BroObj* obj = nullptr);

	// TODO: comments
	void QueueCheckedEvent(const EventHandlerPtr &h,
	                       std::vector<IntrusivePtr<Val>> vl,
	                       SourceID src = SOURCE_LOCAL, analyzer::ID aid = 0,
	                       TimerMgr* mgr = nullptr, BroObj* obj = nullptr);

	void Dispatch(Event* event, bool no_remote = false);

	void Drain();
	bool IsDraining() const	{ return draining; }

	int HasEvents() const	{ return head != 0; }

	// Returns the source ID of last raised event.
	SourceID CurrentSource() const	{ return current_src; }

	// Returns the ID of the analyzer which raised the last event, or 0 if
	// non-analyzer event.
	analyzer::ID CurrentAnalyzer() const	{ return current_aid; }

	// Returns the timer mgr associated with the last raised event.
	TimerMgr* CurrentTimerMgr() const	{ return current_mgr; }

	int Size() const
		{ return num_events_queued - num_events_dispatched; }

	void Describe(ODesc* d) const override;

	double GetNextTimeout() override { return -1; }
	void Process() override;
	const char* Tag() override { return "EventManager"; }
	void InitPostScript();

protected:
	void QueueEvent(Event* event);

	Event* head;
	Event* tail;
	SourceID current_src;
	analyzer::ID current_aid;
	TimerMgr* current_mgr;
	RecordVal* src_val;
	bool draining;
	bro::Flare queue_flare;
};

extern EventMgr mgr;
