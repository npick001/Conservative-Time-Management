#include "Directives.h"

#include <iostream>
#include <fstream>

#include "Airport.h"

using namespace std;

int Airport::_nextID = 1;

class Airport::AirportEventAction : public EventAction
{
public:
	AirportEventAction(Airport* airport, Airplane* airplane)
	{
		_airport = airport;
		_airplane = airplane;
	}

	virtual void Execute() = 0;
protected:
	Airport* _airport;
	Airplane* _airplane;
};

class Airport::AcquireRunwayLandEA : public AirportEventAction
{
public:
	AcquireRunwayLandEA(Airport* airport, Airplane* airplane) : AirportEventAction(airport, airplane) {};
	void Execute();
};

class Airport::LandEA : public AirportEventAction
{
public:
	LandEA(Airport* airport, Airplane* airplane) : AirportEventAction(airport, airplane) {};
	void Execute();
};

class Airport::Taxi2GateEA : public AirportEventAction
{
public:
	Taxi2GateEA(Airport* airport, Airplane* airplane) : AirportEventAction(airport, airplane) {};
	void Execute();
};

class Airport::AcquireGateEA : public AirportEventAction
{
public:
	AcquireGateEA(Airport* airport, Airplane* airplane) : AirportEventAction(airport, airplane) {};
	void Execute();
};

class Airport::ArriveGateEA : public AirportEventAction
{
public:
	ArriveGateEA(Airport* airport, Airplane* airplane) : AirportEventAction(airport, airplane) {};
	void Execute();
};

class Airport::DockAtGateEA : public AirportEventAction
{
public:
	DockAtGateEA(Airport* airport, Airplane* airplane) : AirportEventAction(airport, airplane) {};
	void Execute();
};

class Airport::StartCleanEA : public AirportEventAction
{
public:
	StartCleanEA(Airport* airport, Airplane* airplane) : AirportEventAction(airport, airplane) {};
	void Execute();
};

class Airport::StartEmbarkEA : public AirportEventAction
{
public:
	StartEmbarkEA(Airport* airport, Airplane* airplane) : AirportEventAction(airport, airplane) {};
	void Execute();
};

class Airport::UnloadLuggageEA : public AirportEventAction
{
public:
	UnloadLuggageEA(Airport* airport, Airplane* airplane) : AirportEventAction(airport, airplane) {};
	void Execute();
};

class Airport::LoadLuggageEA : public AirportEventAction
{
public:
	LoadLuggageEA(Airport* airport, Airplane* airplane) : AirportEventAction(airport, airplane) {};
	void Execute();
};

class Airport::ReleaseLuggageCartEA : public AirportEventAction
{
public:
	ReleaseLuggageCartEA(Airport* airport, Airplane* airplane) : AirportEventAction(airport, airplane) {};
	void Execute();
};

class Airport::FuelPlaneEA : public AirportEventAction
{
public:
	FuelPlaneEA(Airport* airport, Airplane* airplane) : AirportEventAction(airport, airplane) {};
	void Execute();
};

class Airport::ReleaseFuelTruckEA : public AirportEventAction
{
public:
	ReleaseFuelTruckEA(Airport* airport, Airplane* airplane) : AirportEventAction(airport, airplane) {};
	void Execute();
};

class Airport::LeaveGateEA : public AirportEventAction
{
public:
	LeaveGateEA(Airport* airport, Airplane* airplane) : AirportEventAction(airport, airplane) {};
	void Execute();
};

class Airport::Taxi2RunwayEA : public AirportEventAction
{
public:
	Taxi2RunwayEA(Airport* airport, Airplane* airplane) : AirportEventAction(airport, airplane) {};
	void Execute();
};

class Airport::AcquireRunwayTakeoffEA : public AirportEventAction
{
public:
	AcquireRunwayTakeoffEA(Airport* airport, Airplane* airplane) : AirportEventAction(airport, airplane) {};
	void Execute();
};

class Airport::TakeoffEA : public AirportEventAction
{
public:
	TakeoffEA(Airport* airport, Airplane* airplane) : AirportEventAction(airport, airplane) {};
	void Execute();
};

class Airport::LeaveEA : public AirportEventAction
{
public:
	LeaveEA(Airport* airport, Airplane* airplane) : AirportEventAction(airport, airplane) {};
	void Execute();
};

/************************************************************/

void Airport::AcquireRunwayLandEA::Execute()
{
#if TRACE
	_airport->PrintTrace("AcquireRunwayLand", _airplane);
#endif

#if WRITE_TO_FILE
	_airport->WriteTrace("AcquireRunwayLand", _airplane);
#endif // WRITE_TO_FILE

	_airport->_runways->RequestResource(new LandEA(_airport, _airplane));
}

void Airport::LandEA::Execute()
{
#if TRACE
	_airport->PrintTrace("Land", _airplane);
#endif

#if WRITE_TO_FILE
	_airport->WriteTrace("Land", _airplane);
#endif // WRITE_TO_FILE

	ScheduleEventIn(_airport->t_landing->GetRV(), new Taxi2GateEA(_airport, _airplane));
}

void Airport::Taxi2GateEA::Execute()
{
#if TRACE
	_airport->PrintTrace("Taxi2Gate", _airplane);
#endif

#if WRITE_TO_FILE
	_airport->WriteTrace("Taxi2Gate", _airplane);
#endif // WRITE_TO_FILE

	_airport->_runways->ReleaseResource();
	ScheduleEventIn(_airport->t_taxiGate->GetRV(), new AcquireGateEA(_airport, _airplane));
}

void Airport::AcquireGateEA::Execute()
{
#if TRACE
	_airport->PrintTrace("AcquireGate", _airplane);
#endif

#if WRITE_TO_FILE
	_airport->WriteTrace("AcquireGate", _airplane);
#endif // WRITE_TO_FILE

	_airport->_gates->RequestResource(new DockAtGateEA(_airport, _airplane));
}

void Airport::DockAtGateEA::Execute()
{
#if TRACE
	_airport->PrintTrace("DockAtGate", _airplane);
#endif

#if WRITE_TO_FILE
	_airport->WriteTrace("DockAtGate", _airplane);
#endif // WRITE_TO_FILE

	ScheduleEventIn(_airport->t_dockGate->GetRV(), new ArriveGateEA(_airport, _airplane));
}

void Airport::ArriveGateEA::Execute()
{
#if TRACE
	_airport->PrintTrace("ArriveGate", _airplane);
#endif

#if WRITE_TO_FILE
	_airport->WriteTrace("ArriveGate", _airplane);
#endif // WRITE_TO_FILE

	_airport->_baggageCarts->RequestResource(new UnloadLuggageEA(_airport, _airplane));
	_airport->_fuelTrucks->RequestResource(new FuelPlaneEA(_airport, _airplane));
	ScheduleEventIn(_airport->t_disembark->GetRV(), new StartCleanEA(_airport, _airplane));
}

void Airport::StartCleanEA::Execute()
{
#if TRACE
	_airport->PrintTrace("StartClean", _airplane);
#endif

#if WRITE_TO_FILE
	_airport->WriteTrace("StartClean", _airplane);
#endif // WRITE_TO_FILE

	ScheduleEventIn(_airport->t_cleanPlane->GetRV(), new StartEmbarkEA(_airport, _airplane));
}

void Airport::StartEmbarkEA::Execute()
{
#if TRACE
	_airport->PrintTrace("StartEmbark", _airplane);
#endif

#if WRITE_TO_FILE
	_airport->WriteTrace("StartEmbark", _airplane);
#endif // WRITE_TO_FILE

	ScheduleEventIn(_airport->t_embark->GetRV(), new LeaveGateEA(_airport, _airplane));
}

void Airport::UnloadLuggageEA::Execute()
{
#if TRACE
	_airport->PrintTrace("UnloadLuggage", _airplane);
#endif

#if WRITE_TO_FILE
	_airport->WriteTrace("UnloadLuggage", _airplane);
#endif // WRITE_TO_FILE

	ScheduleEventIn(_airport->t_unloadLuggage->GetRV(), new LoadLuggageEA(_airport, _airplane));
}

void Airport::LoadLuggageEA::Execute()
{
#if TRACE
	_airport->PrintTrace("LoadLuggage", _airplane);
#endif

#if WRITE_TO_FILE
	_airport->WriteTrace("LoadLuggage", _airplane);
#endif // WRITE_TO_FILE

	ScheduleEventIn(_airport->t_loadLuggage->GetRV(), new ReleaseLuggageCartEA(_airport, _airplane));
}

void Airport::ReleaseLuggageCartEA::Execute()
{
#if TRACE
	_airport->PrintTrace("ReleaseLuggageCart", _airplane);
#endif

#if WRITE_TO_FILE
	_airport->WriteTrace("ReleaseLuggageCart", _airplane);
#endif // WRITE_TO_FILE

	_airport->_baggageCarts->ReleaseResource();
	ScheduleEventIn(0.0, new LeaveGateEA(_airport, _airplane));
}

void Airport::FuelPlaneEA::Execute()
{
#if TRACE
	_airport->PrintTrace("FuelPlane", _airplane);
#endif

#if WRITE_TO_FILE
	_airport->WriteTrace("FuelPlane", _airplane);
#endif // WRITE_TO_FILE

	ScheduleEventIn(_airport->t_fuelPlane->GetRV(), new ReleaseFuelTruckEA(_airport, _airplane));
}

void Airport::ReleaseFuelTruckEA::Execute()
{
#if TRACE
	_airport->PrintTrace("ReleaseFuelTruck", _airplane);
#endif

#if WRITE_TO_FILE
	_airport->WriteTrace("ReleaseFuelTruck", _airplane);
#endif // WRITE_TO_FILE

	_airport->_fuelTrucks->ReleaseResource();
	ScheduleEventIn(0.0, new LeaveGateEA(_airport, _airplane));
}

void Airport::LeaveGateEA::Execute()
{
#if TRACE
	_airport->PrintTrace("LeaveGate", _airplane);
#endif

#if WRITE_TO_FILE
	_airport->WriteTrace("LeaveGate", _airplane);
#endif // WRITE_TO_FILE

	std::map<Airplane*, int>::iterator it;
	it = _airport->_gateJoin.find(_airplane);
	if (it == _airport->_gateJoin.end()) {
		_airport->_gateJoin.insert(std::pair<Airplane*, int>(_airplane, 1));
	}
	else {
		it->second++;
		if (it->second == 3) {
			_airport->_gateJoin.erase(it);
			_airport->_gates->ReleaseResource();
			ScheduleEventIn(_airport->t_leaveGate->GetRV(), new Taxi2RunwayEA(_airport, _airplane));
		}
	}
}

void Airport::Taxi2RunwayEA::Execute()
{
#if TRACE
	_airport->PrintTrace("Taxi2RunwayEA", _airplane);
#endif

#if WRITE_TO_FILE
	_airport->WriteTrace("Taxi2RunwayEA", _airplane);
#endif // WRITE_TO_FILE

	ScheduleEventIn(_airport->t_taxiRunway->GetRV(), new AcquireRunwayTakeoffEA(_airport, _airplane));
}

void Airport::AcquireRunwayTakeoffEA::Execute()
{
#if TRACE
	_airport->PrintTrace("AcquireRunwayTakeoff", _airplane);
#endif

#if WRITE_TO_FILE
	_airport->WriteTrace("AcquireRunwayTakeoff", _airplane);
#endif // WRITE_TO_FILE

	_airport->_runways->RequestResource(new TakeoffEA(_airport, _airplane));
}

void Airport::TakeoffEA::Execute()
{
#if TRACE
	_airport->PrintTrace("Takeoff", _airplane);
#endif

#if WRITE_TO_FILE
	_airport->WriteTrace("Takeoff", _airplane);
#endif // WRITE_TO_FILE

	ScheduleEventIn(_airport->t_takeoff->GetRV(), new LeaveEA(_airport, _airplane));
}

void Airport::LeaveEA::Execute()
{
#if TRACE
	_airport->PrintTrace("Leave", _airplane);
#endif

#if WRITE_TO_FILE
	_airport->WriteTrace("Leave", _airplane);
#endif // WRITE_TO_FILE

	_airport->_runways->ReleaseResource();
	_airport->Leave(_airplane);
}

/***************************************************************/

Airport::Airport(string name, SystemOfAirports *soa)
{
	_name = name;
	_id = _nextID++;
	_planeID = 1;
	_soa = soa;

	ifstream airportFile(name + ".txt");
	string s;
	getline(airportFile, s);
	getline(airportFile, s);
	getline(airportFile, s);

	int numResource;
	getline(airportFile, s);
	airportFile >> numResource;
	_runways = new ResourcePool(_name + "Runways", numResource);
	getline(airportFile, s);

	getline(airportFile, s);
	airportFile >> numResource;
	_gates = new ResourcePool(_name + "Gates", numResource);
	getline(airportFile, s);

	getline(airportFile, s);
	airportFile >> numResource;
	_baggageCarts = new ResourcePool(_name + "BaggageCarts", numResource);
	getline(airportFile, s);

	getline(airportFile, s);
	airportFile >> numResource;
	_fuelTrucks = new ResourcePool(_name + "FuelTrucks", numResource);
	getline(airportFile, s);

	getline(airportFile, s);
	getline(airportFile, s);

	float tmin, tmode, tmax;

	getline(airportFile, s);
	airportFile >> tmin >> tmode >> tmax;
	t_landing = new Triangular(tmin, tmode, tmax);
	getline(airportFile, s);

	getline(airportFile, s);
	airportFile >> tmin >> tmode >> tmax;
	t_taxiGate = new Triangular(tmin, tmode, tmax);
	getline(airportFile, s);

	getline(airportFile, s);
	airportFile >> tmin >> tmode >> tmax;
	t_dockGate = new Triangular(tmin, tmode, tmax);
	getline(airportFile, s);

	getline(airportFile, s);
	airportFile >> tmin >> tmode >> tmax;
	t_disembark = new Triangular(tmin, tmode, tmax);
	getline(airportFile, s);

	getline(airportFile, s);
	airportFile >> tmin >> tmode >> tmax;
	t_cleanPlane = new Triangular(tmin, tmode, tmax);
	getline(airportFile, s);

	getline(airportFile, s);
	airportFile >> tmin >> tmode >> tmax;
	t_embark = new Triangular(tmin, tmode, tmax);
	getline(airportFile, s);

	getline(airportFile, s);
	airportFile >> tmin >> tmode >> tmax;
	t_loadLuggage = new Triangular(tmin, tmode, tmax);
	getline(airportFile, s);

	getline(airportFile, s);
	airportFile >> tmin >> tmode >> tmax;
	t_unloadLuggage = new Triangular(tmin, tmode, tmax);
	getline(airportFile, s);

	getline(airportFile, s);
	airportFile >> tmin >> tmode >> tmax;
	t_fuelPlane = new Triangular(tmin, tmode, tmax);
	getline(airportFile, s);

	getline(airportFile, s);
	airportFile >> tmin >> tmode >> tmax;
	t_leaveGate = new Triangular(tmin, tmode, tmax);
	getline(airportFile, s);

	getline(airportFile, s);
	airportFile >> tmin >> tmode >> tmax;
	t_taxiRunway = new Triangular(tmin, tmode, tmax);
	getline(airportFile, s);

	getline(airportFile, s);
	airportFile >> tmin >> tmode >> tmax;
	t_takeoff = new Triangular(tmin, tmode, tmax);
	getline(airportFile, s);

	getline(airportFile, s);
	airportFile >> tmin >> tmode >> tmax;
	t_flightTime = new Triangular(tmin, tmode, tmax);
	getline(airportFile, s);

	airportFile.close();

#if WRITE_TO_FILE
	m_arrivals_file = "Output Files\\Process " + std::to_string(CommunicationRank()) + " Arrivals.txt";
	m_simulation_file = "Output Files\\Process " + std::to_string(CommunicationRank()) + " Simulation.txt";
	m_departures_file = "Output Files\\Process " + std::to_string(CommunicationRank()) + " Departures.txt";

	std::string ad_header = "Airport, Airplane ID, Arrival Time, Local Simulation Time, Number of Flights, Origin, Destination";
	std::string sim_data_header = "Current sim time, airport, event name, airplane id, runway queue size, gate queue size, baggage cart queue size, fuel truck queue size";

	WriteToFile(m_arrivals_file, ad_header, 1);
	WriteToFile(m_simulation_file, sim_data_header, 1);
	WriteToFile(m_departures_file, ad_header, 1);
#endif // WRITE_TO_FILE
}

void Airport::Arrive(Time delay, Airplane* a)
{
	ScheduleEventIn(delay, new AcquireRunwayLandEA(this, a));
}

void Airport::PlaneArrival(int source)
{
	Airplane* airplane = new Airplane(source);
	double arrival_time = airplane->GetArrivalTime();

#if TRACE
	std::cout << std::endl;
	std::cout << "Current time: " << GetSimulationTime() << std::endl;
	std::cout << "Airplane arrived at " << _name << " from " << source << " at " << arrival_time << std::endl;
	std::cout << std::endl;
#endif // TRACE

#if WRITE_TO_FILE
	std::string arrivals_data = _name + ", " + std::to_string(airplane->GetID()[0]) + "." + std::to_string(airplane->GetID()[1]) + ", "
		+ std::to_string(arrival_time) + ", "
		+ std::to_string(GetSimulationTime()) + ", "
		+ std::to_string(airplane->GetNumberOfFlights()) + ", "
		+ std::to_string(source) + ", " + std::to_string(CommunicationRank());

	WriteToFile(m_arrivals_file, arrivals_data, 0);
#endif // WRITE_TO_FILE

	ScheduleEventAt(arrival_time, new AcquireRunwayLandEA(this, airplane));
}

Airplane* Airport::CreatePlane()
{
#if TRACE
	std::cout << "plane id: " << _planeID << std::endl;
#endif // TRACE

	Airplane* plane = new Airplane(150.0, _planeID++);

#if TRACE
	std::cout << "plane id: " << _planeID << std::endl;
#endif // TRACE

	return plane;
}

int Airport::GetNextDestination()
{
	int rank = CommunicationRank();
	int size = CommunicationSize();
	int offset = rand() % (size - 1);
	return((rank + 1 + offset) % size);
}

void Airport::Leave(Airplane* airplane)
{
	int next_destination = GetNextDestination();
	double arrival_time = GetSimulationTime() + t_flightTime->GetRV();

#if TRACE
	std::cout << std::endl;
	std::cout << "Plane " << airplane->GetID()[0] << "." << airplane->GetID()[1] << " leaving airport " << _name << std::endl;
	std::cout << "Number of Flights: " << airplane->GetNumberOfFlights() << std::endl;
	std::cout << "Origin: " << CommunicationRank() << " Destination: " << next_destination << std::endl;
	std::cout << "Departure Time: " << GetSimulationTime() << std::endl;
	std::cout << "Arrival Time: " << arrival_time << std::endl;
	std::cout << std::endl;
#endif // TRACE

#if WRITE_TO_FILE
	std::string data = _name + ", " + std::to_string(airplane->GetID()[0]) + "." + std::to_string(airplane->GetID()[1]) + ", "
		+ std::to_string(arrival_time) + ", "
		+ std::to_string(GetSimulationTime()) + ", "
		+ std::to_string(airplane->GetNumberOfFlights()) + ", "
		+ std::to_string(CommunicationRank()) + ", " + std::to_string(next_destination);

	WriteToFile(m_departures_file, data, 0);
#endif // WRITE_TO_FILE

	airplane->AddFlightOrigin(CommunicationRank());
	airplane->AddFlight();
	airplane->SendFlight(next_destination, arrival_time);
}

void Airport::PrintTrace(std::string eventName, Airplane *airplane)
{
	// Current sim time, airport, event name, airplane id, runway queue size, gate queue size, baggage cart queue size, fuel truck queue size

	cout << GetSimulationTime() << ", " << _name << ", " << eventName << ", " 
		 << airplane->GetID()[0] << "." << airplane->GetID()[1] << ", ";
	cout << _runways->GetQueueSize() << ", " << _gates->GetQueueSize() << ", ";
	cout << _baggageCarts->GetQueueSize() << ", " << _fuelTrucks->GetQueueSize() << ", " << endl;
}

void Airport::WriteTrace(std::string eventName, Airplane* airplane)
{
// Current sim time, airport, event name, airplane id, runway queue size, gate queue size, baggage cart queue size, fuel truck queue size

	std::string data = std::to_string(GetSimulationTime()) + ", " + _name + ", " + eventName + ", "
		+ std::to_string(airplane->GetID()[0]) + "." + std::to_string(airplane->GetID()[1]) + ", "
		+ std::to_string(_runways->GetQueueSize()) + ", " + std::to_string(_gates->GetQueueSize()) + ", "
		+ std::to_string(_baggageCarts->GetQueueSize()) + ", " + std::to_string(_fuelTrucks->GetQueueSize()) + ", ";

	WriteToFile(m_simulation_file, data, 0);
}

void PopulateAirportWithPlanes(Airport* airport, int numPlanes)
{
	for (int i = 0; i < numPlanes; i++)
	{
		airport->Arrive(i, airport->CreatePlane());
	}
}
