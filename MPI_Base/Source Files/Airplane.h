#pragma once
#include <iostream>
#include "Communication.h"

class Airplane : public CommunicationPattern
{
public:
	Airplane(double capacity, int planeID);
	Airplane(int source);

	void SendFlight(int rank, double arrivalTime);
	void AddFlight();
	void AddFlightOrigin(int origin);
	void AddCargo(double size);
	void RemoveCargo(double size);

	int GetCargoQuantity();
	double GetCargoSize();
	int GetNumberOfFlights();
	int* GetID();
	double GetArrivalTime();

	bool Fits(double size);
	void PrintAirplane();

protected:
	const int GetBufferSize() override;
	void Serialize(int* dataBuffer) override;
	void Deserialize(int* dataBuffer) override;

private:
	int m_id[2];

	int m_flightOrigin;
	int m_numberOfFlights;

	int m_cargoQuantity;
	double m_cargoCapacity;
	double m_cargoSize;

	double m_arrivalTime;
};