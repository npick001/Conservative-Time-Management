#pragma once
#include <string>
#include <map>

#include "Directives.h"
#include "Utility.h"
#include "SimulationExecutive.h"
#include "Distribution.h"
#include "ResourcePool.h"

#include "Airplane.h"

class SystemOfAirports;

class Airport
{
public:
	Airport(std::string name, SystemOfAirports *soa);
	void Arrive(Time t, Airplane* a);

	void PlaneArrival(int source);

	// called to create a new plane, not specific to this airport
	// but the unique id for the plane is specific to this airport
	Airplane* CreatePlane();

protected:
	int GetNextDestination();
	void Leave(Airplane* a);

private:
	std::string _name;
	int _id;
	int _planeID;
	static int _nextID;

	std::string m_arrivals_file;
	std::string m_simulation_file;
	std::string m_departures_file;

	SystemOfAirports* _soa;

	ResourcePool* _runways;
	ResourcePool* _gates;
	ResourcePool* _baggageCarts;
	ResourcePool* _fuelTrucks;

	Distribution* t_landing;
	Distribution* t_taxiGate;
	Distribution* t_dockGate;
	Distribution* t_disembark;
	Distribution* t_cleanPlane;
	Distribution* t_embark;
	Distribution* t_loadLuggage;
	Distribution* t_unloadLuggage;
	Distribution* t_fuelPlane;
	Distribution* t_leaveGate;
	Distribution* t_taxiRunway;
	Distribution* t_takeoff;
	Distribution* t_flightTime;

	Time _lookAhead;

	std::map<Airplane*, int> _gateJoin;

	class AirportEventAction;

	class AcquireRunwayLandEA;
	class LandEA;
	class Taxi2GateEA;
	class AcquireGateEA;
	class DockAtGateEA;
	class ArriveGateEA;
	class StartCleanEA;
	class StartEmbarkEA;
	class UnloadLuggageEA;
	class LoadLuggageEA;
	class ReleaseLuggageCartEA;
	class FuelPlaneEA;
	class ReleaseFuelTruckEA;
	class LeaveGateEA;
	class Taxi2RunwayEA;
	class AcquireRunwayTakeoffEA;
	class TakeoffEA;
	class LeaveEA;
	class BarrierEA;

	// print trace to terminal
	void PrintTrace(std::string eventName, Airplane* airplane);

	// write trace to file
	void WriteTrace(std::string eventName, Airplane* airplane);
};

void PopulateAirportWithPlanes(Airport* airport, int numPlanes);