#include "Airplane.h"

Airplane::Airplane(double capacity, int planeID)
{
	// unique ID is the process rank and the plane ID
	m_id[0] = CommunicationRank() + 1;
	m_id[1] = planeID;
	m_cargoCapacity = capacity;
	m_cargoSize = 10.0;
	m_cargoQuantity = 1;
	m_numberOfFlights = 0;
	m_flightOrigin = -1;
	m_arrivalTime = -1.0;
}

Airplane::Airplane(int source)
{
	/* 
		– this constructor populates the new airplane by receiving its contents
		through an MPI communication. It should receive the message buffer from MPI and populate the
		object using the CommunicationPattern Recv function.
	*/
	m_id[0] = -1;
	m_id[1] = -1;
	m_cargoCapacity = 150.0;
	m_cargoSize = 10.0;
	m_cargoQuantity = 1;
	m_numberOfFlights = 0;
	m_flightOrigin = -1;
	m_arrivalTime = -1.0;

	Receive(source);
}

void Airplane::SendFlight(int rank, double arrivalTime)
{
	m_arrivalTime = arrivalTime;
	// This should send the airplane to the process indicated by rank using MPI
	// through the CommunicationPattern

	//std::cout << CommunicationRank() << ": Sending flight " << std::to_string(m_id[0]) << "." << std::to_string(m_id[1]) << " to " << rank << " with time " << m_arrivalTime << std::endl;

	Send(rank);
}

void Airplane::AddFlight()
{
	m_numberOfFlights++;
}

void Airplane::AddFlightOrigin(int origin)
{
	m_flightOrigin = origin;
}

void Airplane::AddCargo(double size)
{
	m_cargoQuantity++;
	m_cargoSize += size;
}

void Airplane::RemoveCargo(double size)
{
	m_cargoQuantity--;
	m_cargoSize -= size;
}

int Airplane::GetCargoQuantity()
{
	return m_cargoQuantity;
}

double Airplane::GetCargoSize()
{
	return m_cargoSize;
}

int Airplane::GetNumberOfFlights()
{
	return m_numberOfFlights;
}

int* Airplane::GetID()
{
	return m_id;
}

double Airplane::GetArrivalTime()
{
	return m_arrivalTime;
}

bool Airplane::Fits(double size)
{
	if(m_cargoSize + size <= m_cargoCapacity)
		return true;
	else
		return false;
}

void Airplane::PrintAirplane()
{
	//std::cout 
	//	<< "Current Process Rank" << ", "
	//	<< "Flight Origin Rank" << ", "
	//	<< "ID Rank" << ", "
	//	<< "Unique ID" << ", "
	//	<< "Number of Flights" << ", "
	//	<< "Cargo Quantity" << ", "
	//	<< "Cargo Capacity" << ", "
	//	<< "Cargo Utilized"
	//<< std::endl;

	std::cout
		<< CommunicationRank() << ", "
		<< m_flightOrigin << ", "
		<< m_id[0] << "."
		<< m_id[1] << ", "
		<< m_numberOfFlights << ", "
		<< m_cargoQuantity << ", "
		<< m_cargoCapacity << ", "
		<< m_cargoSize << ", "
		<< m_arrivalTime
	<< std::endl;
}

const int Airplane::GetBufferSize()
{
	return ((sizeof(m_id[0])
		+ sizeof(m_id[1])
		+ sizeof(m_flightOrigin)
		+ sizeof(m_numberOfFlights)
		+ sizeof(m_cargoQuantity)
		+ sizeof(m_cargoCapacity)
		+ sizeof(m_cargoSize)
		+ sizeof(m_arrivalTime)) / sizeof(int));
}

void Airplane::Serialize(int* dataBuffer)
{
	int index = 0;
	int* dataRef;
	dataRef = (int*)&m_id[0];
	for (int i = 0; i < sizeof(m_id[0]) / sizeof(int); i++) {
		dataBuffer[index++] = dataRef[i];
	}
	dataRef = (int*)&m_id[1];
	for (int i = 0; i < sizeof(m_id[1]) / sizeof(int); i++) {
		dataBuffer[index++] = dataRef[i];
	}
	dataRef = (int*)&m_flightOrigin;
	for (int i = 0; i < sizeof(m_flightOrigin) / sizeof(int); i++) {
		dataBuffer[index++] = dataRef[i];
	}
	dataRef = (int*)&m_numberOfFlights;
	for (int i = 0; i < sizeof(m_numberOfFlights) / sizeof(int); i++) {
		dataBuffer[index++] = dataRef[i];
	}
	dataRef = (int*)&m_cargoQuantity;
	for (int i = 0; i < sizeof(m_cargoQuantity) / sizeof(int); i++) {
		dataBuffer[index++] = dataRef[i];
	}
	dataRef = (int*)&m_cargoCapacity;
	for (int i = 0; i < sizeof(m_cargoCapacity) / sizeof(int); i++) {
		dataBuffer[index++] = dataRef[i];
	}
	dataRef = (int*)&m_cargoSize;
	for (int i = 0; i < sizeof(m_cargoSize) / sizeof(int); i++) {
		dataBuffer[index++] = dataRef[i];
	}	
	dataRef = (int*)&m_arrivalTime;
	for (int i = 0; i < sizeof(m_arrivalTime) / sizeof(int); i++) {
		dataBuffer[index++] = dataRef[i];
	}
}

void Airplane::Deserialize(int* dataBuffer)
{
	int index = 0;
	int* dataRef;
	dataRef = (int*)&m_id[0];
	for (int i = 0; i < sizeof(m_id[0]) / sizeof(int); i++) {
		dataRef[i] = dataBuffer[index++];
	}
	dataRef = (int*)&m_id[1];
	for (int i = 0; i < sizeof(m_id[1]) / sizeof(int); i++) {
		dataRef[i] = dataBuffer[index++];
	}
	dataRef = (int*)&m_flightOrigin;
	for (int i = 0; i < sizeof(m_flightOrigin) / sizeof(int); i++) {
		dataRef[i] = dataBuffer[index++];
	}
	dataRef = (int*)&m_numberOfFlights;
	for (int i = 0; i < sizeof(m_numberOfFlights) / sizeof(int); i++) {
		dataRef[i] = dataBuffer[index++];
	}
	dataRef = (int*)&m_cargoQuantity;	
	for (int i = 0; i < sizeof(m_cargoQuantity) / sizeof(int); i++) {
		dataRef[i] = dataBuffer[index++];
	}
	dataRef = (int*)&m_cargoCapacity;
	for (int i = 0; i < sizeof(m_cargoCapacity) / sizeof(int); i++) {
		dataRef[i] = dataBuffer[index++];
	}
	dataRef = (int*)&m_cargoSize;
	for (int i = 0; i < sizeof(m_cargoSize) / sizeof(int); i++) {
		dataRef[i] = dataBuffer[index++];
	}
	dataRef = (int*)&m_arrivalTime;
	for (int i = 0; i < sizeof(m_arrivalTime) / sizeof(int); i++) {
		dataRef[i] = dataBuffer[index++];
	}

	// clean up data buffer 
	delete[] dataBuffer;
}

