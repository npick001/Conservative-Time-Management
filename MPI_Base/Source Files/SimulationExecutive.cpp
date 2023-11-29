#include <iostream>
#include <vector>
#include "SimulationExecutive.h"
#include "Communication.h"
#include "FIFO.h"

using namespace std;
#define TOLERANCE 1E-26

class RecvNullMsgEA : public EventAction
{
public:
	RecvNullMsgEA(int source, double time)
	{
		_source = source;
		_time = time;
	}

	void Execute()
	{
#if NULL_MSG_TRACE
		std::cout << CommunicationRank() << ": Inside RecvNullMsgEA::Execute()" << std::endl;
		std::cout << CommunicationRank() << ": Null message received from " << _source << std::endl;
#endif // NULL_MSG_TRACE
	}

private:
	int _source;
	double _time;
};

class SendNullMsgEA : public EventAction
{
public:
	SendNullMsgEA(int destination, double time)
	{
		_destination = destination;
		_time = time;
	}

	void Execute()
	{
#if NULL_MSG_TRACE
		std::cout << CommunicationRank() << ": Inside SendNullMsgEA::Execute()" << std::endl;
		std::cout << CommunicationRank() << ": Sending null message to " << _destination << std::endl;
#endif // NULL_MSG_TRACE

		SendNullMsg(_destination, _time);
	}

private:
	int _destination;
	double _time;
};

class SimulationExecutive
{
public:
	static void InitializeSimulation()
	{
		_simTime = 0.0;

		_queueProcessors.clear();
		_incomingQueues.clear();
		for (int i = 0; i < CommunicationSize(); i++)
		{
			if (i == CommunicationRank()) {
				continue;
			}
			else {
				auto queue = FIFO<Event>();
				_incomingQueues.push_back(queue);

				auto queue2 = FIFO<Event>();
				_outgoingQueues.push_back(queue2);

				_queueProcessors.push_back(i);
			}
#if NULL_MSG_TRACE
			std::cout << "Incoming queue for " << i << " created on " << CommunicationRank() << " processor." << std::endl;
#endif // NULL_MSG_TRACE
		}
	}

	static Time GetSimulationTime() { return _simTime; }
	static void RunSimulation()
	{
		while (_eventList.HasEvent()) {
			Event* e = _eventList.GetEvent();

			if (e->_time < _simTime) {
				_events_executed_OoO++;
			}

			_simTime = e->_time;
			e->_ea->Execute();
			delete e;

			CheckAndProcessMessages();
		}

#if EXECUTION_TRACE
		std::cout << CommunicationRank() << ": Events executed out of order: " << _events_executed_OoO << std::endl;
#endif // EXECUTION_TRACE
	}

	static void RunSimulation(Time endTime)
	{
#if EXECUTION_TRACE
		std::cout << CommunicationRank() << ": Running simulation until " << endTime << std::endl;
#endif // EXECUTION_TRACE

		SendInitialNullMsgs();
		
		while (_eventList.HasEvent() && _simTime <= endTime) {

			// wait for each incoming queue to have a message
			while (!EachIncomingQueueHasMsg()) 
			{
				CheckAndProcessMessages();
			}

			// get the minimum time stamp of all the messages
			auto min_msg = GetMinMsgTime();

			// check if the minimum msg time stamp is less than the smallest time in the event list
			if (min_msg.second < _eventList.ViewEvent()._time)
			{
				// if it is, then advance the simulation time to the minimum msg time stamp
				_simTime = min_msg.second;

				// find the right queue to remove the time from
				for (int i = 0; i < CommunicationSize(); i++)
				{
					// remove that time from the queue
					if (_queueProcessors[i] == min_msg.first)
					{
#if NULL_MSG_TRACE
						std::cout << CommunicationRank() << ": Removing " << min_msg.second << " from queue " << i << std::endl;
#endif // NULL_MSG_TRACE

						_incomingQueues[i].GetEntity()->_ea->Execute();
						break;
					}
				}
			}
			else
			{
				// execute the next event
				Event* e = _eventList.GetEvent();

				if (e->_time < _simTime) {
					_events_executed_OoO++;
				}

				_simTime = e->_time;
				if (_simTime <= endTime) {

#if EXECUTION_TRACE
					std::cout << CommunicationRank() << ": Executing event with time " << _simTime << std::endl;
#endif // EXECUTION_TRACE

					e->_ea->Execute();
				}
				delete e;
			}

			ReduceQueues(_simTime);

			// finally send null messages to other LP's
			// so they can advance their simulation time
			BroadcastNullMsgs();
		}
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

	static void CaughtMsg(int source)
	{
		_msgHandler(source);
	}

	static void ProcessMsg(int tag, int source) {
		if (tag == 0) {
			double msg_time = ReceiveNullMsg(source);

#if NULL_MSG_TRACE
			std::cout << CommunicationRank() << ": Null message received from " << source << " with time " << msg_time << std::endl;
#endif // NULL_MSG_TRACE


			// find the source from proc vector
			for (int i = 0; i < _queueProcessors.size(); i++)
			{
				if (_queueProcessors[i] == source)
				{
					_incomingQueues[i].AddEntity(new Event(msg_time, new RecvNullMsgEA(source, msg_time)));
					break;
				}
			}
		}
		else {
#if NULL_MSG_TRACE
			std::cout << CommunicationRank() << ": Message received from " << source << std::endl;
#endif // NULL_MSG_TRACE

			CaughtMsg(source);
		}
	}

	static void CheckAndProcessMessages()
	{
		int tag;
		int source;
		while (CheckForComm(tag, source)) {
			ProcessMsg(tag, source);
		}
	}

	static bool EachIncomingQueueHasMsg()
	{
		for (int i = 0; i < _incomingQueues.size(); i++)
		{
			if (_incomingQueues[i].IsEmpty())
			{
				return false;
			}
		}
		return true;
	}

	static std::pair<int, double> GetMinMsgTime()
	{
		double min_time = DBL_MAX;
		int min_index = -1;
		for (int i = 0; i < _incomingQueues.size(); i++)
		{
			auto queue = _incomingQueues[i];
			bool queue_empty = queue.IsEmpty();
			if (queue_empty)
			{
				continue;
			}
			else if (queue.ViewEntity()->_time < TOLERANCE)
			{
				// if this value is below the tolerance
				// remove this from the queue
				queue.GetEntity();
				continue;
			}
			else if (queue.ViewEntity()->_time < min_time)
			{
				min_time = queue.GetEntity()->_time;
				min_index = _queueProcessors[i];
			}
		}
		return std::make_pair(min_index, min_time);
	}

	static void ReduceQueues(Time time)
	{
		// remove all the time values that are less than the current simulation time 
		// from the incoming queues.
		for (auto queue : _incomingQueues)
		{
			int eventsRemoved = 0;
			while (!queue.IsEmpty())
			{
				if (queue.ViewEntity()->_time < time)
				{
					queue.GetEntity();
					eventsRemoved++;
				}
				else
				{
					break;
				}
			}
		}

		// send all the time values that are less than the current simulation time
		// from the outgoing queues.
		for (auto queue : _outgoingQueues)
		{
			int msgsSent = 0;
			while (!queue.IsEmpty())
			{
				if (queue.ViewEntity()->_time < time)
				{
					queue.GetEntity();
					msgsSent++;
				}
				else
				{
					break;
				}
			}
		}
	}

	static void BroadcastNullMsgs()
	{
		// send null msgs to other LPs
		for (int i = 0; i < _outgoingQueues.size(); i++)
		{
			int destination = _queueProcessors[i];
			double msg_time = _simTime + _lookahead;

#if NULL_MSG_TRACE
			std::cout << CommunicationRank() << ": Sending outgoing null msg to " << destination << " with time " << msg_time << std::endl;
#endif // NULL_MSG_TRACE

			SendNullMsg(destination, msg_time);
		}
	}

	static double RevealLookahead()
	{
		return _lookahead;
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

		Event& ViewEvent() const
		{
			Event *next = _eventList;
			return *next;
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
	static double _lookahead;
	static std::vector<FIFO<Event>> _incomingQueues;
	static std::vector<FIFO<Event>> _outgoingQueues;
	static std::vector<int> _queueProcessors;

	static std::function<void(int)> _msgHandler;
};

SimulationExecutive::EventList SimulationExecutive::_eventList;
Time SimulationExecutive::_simTime = 0.0;
int SimulationExecutive::_events_executed_OoO = 0;
double SimulationExecutive::_lookahead = 10.0;
std::vector<FIFO<SimulationExecutive::Event>> SimulationExecutive::_incomingQueues;
std::vector<FIFO<SimulationExecutive::Event>> SimulationExecutive::_outgoingQueues;
std::vector<int> SimulationExecutive::_queueProcessors;

std::function<void(int)> SimulationExecutive::_msgHandler = 0;

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

void SendInitialNullMsgs()
{
	SimulationExecutive::BroadcastNullMsgs();
}

double GetLookahead()
{
	return SimulationExecutive::RevealLookahead();
}