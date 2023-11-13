#pragma once

#include <string>

#include "SimulationExecutive.h"
#include "FIFO.h"

class ResourcePool
{
public:
	ResourcePool(std::string name, int size);
	void RequestResource(EventAction* ea);
	void ReleaseResource();
	int GetQueueSize();
	int GetNumAvail();
private:
	int _available;
	FIFO<EventAction> _eaQueue;
};
