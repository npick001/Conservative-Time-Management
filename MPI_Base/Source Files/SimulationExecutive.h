#pragma once
#include <functional>
#include "Utility.h"

#define NULL_MSG_TRACE 0
#define EXECUTION_TRACE 0

typedef double Time;

class EventAction
{
public:
	EventAction() {};
	virtual void Execute() = 0;
};

void InitializeSimulation();
Time GetSimulationTime();
void RunSimulation();
void RunSimulation(Time endTime);
void ScheduleEventIn(Time delta, EventAction *ea);
void ScheduleEventAt(Time time, EventAction *ea);
void RegisterMsgHandler(std::function<void(int)> msgHandler);

void SendInitialNullMsgs();
double GetLookahead();
