
#include "ResourcePool.h"

ResourcePool::ResourcePool(string name, int size)
{
	_available = size;
	_eaQueue.AddName(name);
}

void ResourcePool::RequestResource(EventAction* ea)
{
	if (_available > 0) {
		_available--;
		ea->Execute();
	}
	else {
		_eaQueue.AddEntity(ea);
	}
}

void ResourcePool::ReleaseResource()
{
	if (_eaQueue.IsEmpty())
		_available++;
	else {
		EventAction *ea = _eaQueue.GetEntity();
		ea->Execute();
	}
}

int ResourcePool::GetQueueSize()
{
	return _eaQueue.GetSize();
}

int ResourcePool::GetNumAvail()
{
	return _available;
}
