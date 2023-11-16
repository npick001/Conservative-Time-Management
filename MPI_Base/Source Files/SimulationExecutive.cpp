#include <iostream>
#include "SimulationExecutive.h"
#include "Communication.h"

using namespace std;

class SimulationExecutive
{
public:
	static void InitializeSimulation()
	{
		_simTime = 0.0;
	}

	static Time GetSimulationTime() { return _simTime; }
	static void RunSimulation()
	{
		_terminationMessagesReceived = 0;
		while (_eventList.HasEvent()) {
			Event* e = _eventList.GetEvent();

			if (e->_time < _simTime) {
				_events_executed_OoO++;
			}

			if (e->_time >= m_barrierPoint)
			{
				Barrier();
				m_barrierPoint += m_lookahead;
				#if TRACE == 1
				cout << "RunSimBarrier: " << CommunicationRank() << endl;
				#endif
			}

			_simTime = e->_time;
			e->_ea->Execute();
			delete e;

			CheckAndProcessMessages();
		}

		std::cout << CommunicationRank() << ": Events executed out of order: " << _events_executed_OoO << std::endl;

		// "Hey im done over here!"
		//BroadcastTerminationMessage();

		// Wait for everyone else to finish
		//TerminationLoop();
		Barrier();
	}

	static void RunSimulation(Time endTime)
	{
		_terminationMessagesReceived = 0;
		while (_eventList.HasEvent() && _simTime <= endTime) {
			Event* e = _eventList.GetEvent();

			if (e->_time < _simTime) {
				_events_executed_OoO++;
			}
			if ((e->_time >= m_barrierPoint) || e->_time > endTime)
			{
				Barrier();
				m_barrierPoint += m_lookahead;
				#if TRACE == 1
				cout << "RunSimBarrier: " << CommunicationRank() << endl;
				#endif
			}
			_simTime = e->_time;
			if (_simTime <= endTime) {
				e->_ea->Execute();
			}
			delete e;

			CheckAndProcessMessages();
		}

		std::cout << CommunicationRank() << ": Events executed out of order: " << _events_executed_OoO << std::endl;

		// "Hey im done over here!"
		//BroadcastTerminationMessage();

		// Wait for everyone else to finish
		//TerminationLoop();
		Barrier();
	}

	static void ScheduleEventIn(Time delta, EventAction *ea)
	{
		_eventList.AddEvent(_simTime + delta, ea);
	}

	static void ScheduleEventAt(Time time, EventAction* ea)
	{
		_eventList.AddEvent(time, ea);
	}

	// NEW FUNCTIONALITY FOR THE SIMULATION EXECUTIVE FOR THIS
	// -----------------------------------------------
	static void RegisterMsgHandler(std::function<void(int)> msgHandler)
	{
		_msgHandler = msgHandler;
	}

	static void setLookAhead(float l)
	{
		m_lookahead;
		m_barrierPoint = m_lookahead;
	}

	static void CaughtMsg(int source)
	{
		_msgHandler(source);
	}

	static void CheckAndProcessMessages()
	{
		int tag;
		int source;
		while (CheckForComm(tag, source)) {
			CaughtMsg(source);
		}
	}

	static void TerminationLoop()
	{
		std::cout << CommunicationRank() << ": Waiting for termination messages..." << std::endl;

		bool done = false;
		while (!done) {

			int tag;
			int source;
			while (!CheckForComm(tag, source))
			{
				if (tag == 0) {
					_terminationMessagesReceived++;
					std::cout << CommunicationRank() << ": Termination message received from " << source << std::endl;
					std::cout << "Number of termination messages: " << _terminationMessagesReceived << std::endl;
				}
			}

			if (_terminationMessagesReceived == CommunicationSize() - 1) {
				done = true;
			}
		}
	}
	// -----------------------------------------------

private:
	struct Event
	{
		Event(Time time, EventAction* ea)
		{
			_time = time;
			_ea = ea;
			_nextEvent = 0;
		}
		Time _time;
		EventAction* _ea;
		Event *_nextEvent;
	};

	class EventList
	{
	public:
		EventList()
		{
			_eventList = 0;
		}

		void AddEvent(Time time, EventAction*ea)
		{
			Event *e = new Event(time, ea);
			if (_eventList == 0) {
				//event list empty
				_eventList = e;
			}
			else if (time < _eventList->_time) {
				//goes at the head of the list
				e->_nextEvent = _eventList;
				_eventList = e;
			}
			else {
				//search for where to put the event
				Event *curr = _eventList;
				while ((curr->_nextEvent != 0) ? (e->_time >= curr->_nextEvent->_time) : false) {
					curr = curr->_nextEvent;
				}
				if (curr->_nextEvent == 0) {
					//goes at the end of the list
					curr->_nextEvent = e;
				}
				else {
					e->_nextEvent = curr->_nextEvent;
					curr->_nextEvent = e;
				}
			}
		}

		Event* GetEvent()
		{
			Event *next = _eventList;
			_eventList = _eventList->_nextEvent;
			return next;
		}

		bool HasEvent()
		{
			return _eventList != 0;
		}

	private:
		Event *_eventList;
	};

	static EventList _eventList;
	static Time _simTime;
	static int _events_executed_OoO;
	static int _terminationMessagesReceived; // Comm_World_Size - 1 stopping condition
	static float m_lookahead;
	static float m_barrierPoint;
	static std::function<void(int)> _msgHandler;
};

SimulationExecutive::EventList SimulationExecutive::_eventList;
Time SimulationExecutive::_simTime = 0.0;
int SimulationExecutive::_events_executed_OoO = 0;
int SimulationExecutive::_terminationMessagesReceived = 0;

std::function<void(int)> SimulationExecutive::_msgHandler = 0;
float SimulationExecutive::m_barrierPoint = 0.0;
float SimulationExecutive::m_lookahead = 0.0;

void InitializeSimulation()
{
	SimulationExecutive::InitializeSimulation();
}

Time GetSimulationTime()
{
	return SimulationExecutive::GetSimulationTime();
}

void RunSimulation()
{
	SimulationExecutive::RunSimulation();
}

void RunSimulation(Time endTime)
{
	SimulationExecutive::RunSimulation(endTime);
}

void ScheduleEventIn(Time delta, EventAction*ea)
{
	SimulationExecutive::ScheduleEventIn(delta, ea);
}

void ScheduleEventAt(Time time, EventAction*ea)
{
	SimulationExecutive::ScheduleEventAt(time, ea);
}

void RegisterMsgHandler(std::function<void(int)> eventHandler)
{
	SimulationExecutive::RegisterMsgHandler(eventHandler);
}

void setLookAhead(float l)
{
	SimulationExecutive::setLookAhead(l);
}