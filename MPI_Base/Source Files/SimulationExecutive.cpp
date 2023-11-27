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
		std::cout << CommunicationRank() << ": Inside RecvNullMsgEA::Execute()" << std::endl;
		std::cout << CommunicationRank() << ": Null message received from " << _source << std::endl;
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
		std::cout << CommunicationRank() << ": Inside SendNullMsgEA::Execute()" << std::endl;
		std::cout << CommunicationRank() << ": Sending null message to " << _destination << std::endl;

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
				queue2.AddEntity(new Event(_lookahead, new SendNullMsgEA(i, _lookahead)));
				_outgoingQueues.push_back(queue2);
			}
			std::cout << "Incoming queue for " << i << " created on " << CommunicationRank() << " processor." << std::endl;
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

		std::cout << CommunicationRank() << ": Events executed out of order: " << _events_executed_OoO << std::endl;

		// "Hey im done over here!"
		//BroadcastTerminationMessage();

		// Wait for everyone else to finish
		//TerminationLoop();
	}

	static void RunSimulation(Time endTime)
	{
		SendInitialNullMsgs();
		while (_eventList.HasEvent() && _simTime <= endTime) {

			// wait for each incoming queue to have a message
			while (!EachIncomingQueueHasMsg())
			{
				//std::cout << CommunicationRank() << ": Waiting for all incoming queues to have a message..." << std::endl;

				// print out queue sizes
				//for (int i = 0; i < _incomingQueues.size(); i++)
				//{
				//	std::cout << CommunicationRank() << ": Queue size: " << _incomingQueues[i].GetSize() << std::endl;
				//}
				//std::cout << std::endl;

				CheckAndProcessMessages();
			}

			// get the minimum time stamp of all the messages
			auto min_msg = GetMinMsgTime();

			//std::cout << CommunicationRank() << ": smallest event time from EventList: " << _eventList.ViewEvent()._time << std::endl;

			// check if the minimum msg time stamp is less than the smallest time in the event list
			if (min_msg.second < _eventList.ViewEvent()._time)
			{
				// if it is, then advance the simulation time to the minimum msg time stamp
				_simTime = min_msg.second;

				//ReduceQueues(_simTime);

				std::cout << "Min msg time is " << min_msg.second << ", smallest event time from EventList: " << _eventList.ViewEvent()._time << std::endl;
			}
			else
			{
				std::cout << CommunicationRank() << ": min_msg_time >= smallest event time from EventList" << std::endl;

				// execute the next event
				Event* e = _eventList.GetEvent();

				if (e->_time < _simTime) {
					_events_executed_OoO++;
				}

				_simTime = e->_time;
				if (_simTime <= endTime) {
					e->_ea->Execute();
				}
				delete e;
			}

			ReduceQueues(_simTime);

			// finally send null messages to other LP's
			// so they can advance their simulation time
			for (int i = 0; i < _outgoingQueues.size(); i++)
			{
				if (i == CommunicationRank())
				{
					continue;
				}
				else
				{
					auto msg_time = _simTime + _lookahead;
					_outgoingQueues[i].AddEntity(new Event(msg_time, new SendNullMsgEA(i, msg_time)));

					// send null msg to all other lps
					SendNullMsg(i, msg_time);
				}
			}
		}

		std::cout << CommunicationRank() << ": Events executed out of order: " << _events_executed_OoO << std::endl;

		// "Hey im done over here!"
		//BroadcastTerminationMessage();

		// Wait for everyone else to finish
		//TerminationLoop();
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
			std::cout << CommunicationRank() << ": Null message received from " << source << " with time " << msg_time << std::endl;

			// add msg_time to incoming queue list
			_incomingQueues[source].AddEntity(new Event(msg_time, new RecvNullMsgEA(source, msg_time)));
		}
		else {
			std::cout << CommunicationRank() << ": Message received from " << source << std::endl;
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
				std::cout << CommunicationRank() << ": Queue " << i << " is empty." << std::endl;
				continue;
			}
			else if (queue.ViewEntity()->_time < TOLERANCE)
			{
				// if this value is below the tolerance
				// remove this from the queue

				std::cout << CommunicationRank() << ": Removing " << queue.ViewEntity()->_time << " from queue " << i << " due to being under tolerance." << std::endl;

				queue.GetEntity();
				continue;
			}
			else if (queue.ViewEntity()->_time < min_time)
			{
				min_time = queue.ViewEntity()->_time;
				min_index = i;

				//std::cout << CommunicationRank() << ": Min time is " << min_time << " from " << min_index << std::endl;
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

			std::cout << CommunicationRank() << ": Removed " << eventsRemoved << " events from queue " << std::endl;
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
					queue.ViewEntity()->_ea->Execute();
					msgsSent++;
				}
				else
				{
					break;
				}
			}

			std::cout << CommunicationRank() << ": Sent " << msgsSent << " messages from queue " << std::endl;
		}
	}

	static void InitialNullMsgs()
	{
		// send null msgs to other LPs
		for (int i = 0; i < _outgoingQueues.size(); i++)
		{
			if (i == CommunicationRank())
			{
				continue;
			}
			else
			{
				int destination = i;
				int msg_time = _simTime + _lookahead;

				std::cout << CommunicationRank() << ": Sending initial null msg to " << destination << " with time " << msg_time << std::endl;

				SendNullMsg(destination, msg_time);
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

	static std::function<void(int)> _msgHandler;
};

SimulationExecutive::EventList SimulationExecutive::_eventList;
Time SimulationExecutive::_simTime = 0.0;
int SimulationExecutive::_events_executed_OoO = 0;
double SimulationExecutive::_lookahead = 10.0;
std::vector<FIFO<SimulationExecutive::Event>> SimulationExecutive::_incomingQueues;
std::vector<FIFO<SimulationExecutive::Event>> SimulationExecutive::_outgoingQueues;

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
	SimulationExecutive::InitialNullMsgs();
}
