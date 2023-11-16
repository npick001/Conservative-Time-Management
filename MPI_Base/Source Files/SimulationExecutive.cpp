#include <iostream>
#include <vector>
#include "SimulationExecutive.h"
#include "Communication.h"
#include "FIFO.h"

using namespace std;

class SimulationExecutive
{
public:
	static void InitializeSimulation()
	{
		_simTime = 0.0;

		_incomingQueue.clear();
		for (int i = 0; i < CommunicationSize(); i++)
		{
			if (i == CommunicationRank()) {
				continue;
			}
			else {
				_incomingQueue.push_back(std::make_pair(i, new FIFO<double>()));
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
		while (_eventList.HasEvent() && _simTime <= endTime) {

			// wait for each incoming queue to have a message
			while (!EachIncomingQueueHasMsg())
			{
				std::cout << CommunicationRank() << ": Waiting for all incoming queues to have a message..." << std::endl;

				// print out queue sizes
				for (int i = 0; i < _incomingQueue.size(); i++)
				{
					std::cout << CommunicationRank() << ": Queue " << i << " size: " << _incomingQueue[i].second->GetSize() << std::endl;
				}
				std::cout << std::endl;

				CheckAndProcessMessages();
			}

			// get the minimum time stamp of all the messages
			auto min_msg = GetMinMsgTime();

			// check if the minimum msg time stamp is less than the smallest time in the event list
			if (min_msg.second < _eventList.ViewEvent()._time)
			{
				// if it is, then advance the simulation time to the minimum msg time stamp
				_simTime = min_msg.second;
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
					e->_ea->Execute();
				}
				delete e;
			}

			// finally send null messages to other LP's
			// so they can advance their simulation time
			for (int i = 0; i < _incomingQueue.size(); i++)
			{
				if (_incomingQueue[i].first == CommunicationRank())
				{
					continue;
				}
				else
				{
					SendNullMsg(_incomingQueue[i].first, _simTime + _lookahead);
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
			std::cout << CommunicationRank() << ": Null message received from " << source << std::endl;

			double msg_time = ReceiveNullMsg(source);

			// add msg_time to incoming queue list
			AddNullMsgToIncomingQueue(source, msg_time);
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
		for (int i = 0; i < _incomingQueue.size(); i++)
		{
			if (_incomingQueue[i].second->IsEmpty())
			{
				return false;
			}
		}
		return true;
	}

	static std::pair<int, double> GetMinMsgTime()
	{
		double min_time = *_incomingQueue[0].second->ViewEntity();
		int min_index = 0;
		for (int i = 1; i < _incomingQueue.size(); i++)
		{
			if (*_incomingQueue[i].second->ViewEntity() < min_time)
			{
				min_time = *_incomingQueue[i].second->ViewEntity();
				min_index = i;

				std::cout << CommunicationRank() << ": Min time is " << min_time << " from " << min_index << std::endl;
			}
		}
		return std::make_pair(min_index, min_time);
	}

	static void AddNullMsgToIncomingQueue(int source, double msg_time)
	{
		for (int i = 0; i < _incomingQueue.size(); i++)
		{
			if (_incomingQueue[i].first == source)
			{
				std::cout << CommunicationRank() << ": Adding null message from " << source << " to incoming queue with time " << msg_time << std::endl;

				_incomingQueue[i].second->AddEntity(&msg_time);
				break;
			}
		}
	}

	static void InitialNullMsgs()
	{
		// send null msgs to other LPs
		for (int i = 0; i < _incomingQueue.size(); i++)
		{
			if (_incomingQueue[i].first == CommunicationRank())
			{
				continue;
			}
			else
			{
				SendNullMsg(_incomingQueue[i].first, _simTime + _lookahead);
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
	static std::vector<std::pair<int, FIFO<double>*>> _incomingQueue;

	static std::function<void(int)> _msgHandler;
};

SimulationExecutive::EventList SimulationExecutive::_eventList;
Time SimulationExecutive::_simTime = 0.0;
int SimulationExecutive::_events_executed_OoO = 0;
double SimulationExecutive::_lookahead = 1.0;
std::vector<std::pair<int, FIFO<double>*>> SimulationExecutive::_incomingQueue;

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
