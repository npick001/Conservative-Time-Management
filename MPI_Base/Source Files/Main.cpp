#include <chrono>

#include "Airport.h"
#include "SimulationExecutive.h"

int main()
{
	// simulation data viewing can be done with the Data_Viewer.xlsx file
	// need to refresh the connections to the data source

	// takes a while to run when writing all data to files,
	// I recommend checking the directives before running.
	// command arg: -localonly -n 2 "$(TargetPath)"
	auto start_time = std::chrono::high_resolution_clock::now();

	CommunicationInitialize();

	int rank = CommunicationRank();

	Airport airport("Airport" + std::to_string(rank + 1), NULL);

	std::function<void(int)> msgHandler = std::bind(&Airport::PlaneArrival, &airport, std::placeholders::_1);

	RegisterMsgHandler(msgHandler);
	PopulateAirportWithPlanes(&airport, 10);

	InitializeSimulation();

	// make sure all are started
	//std::cout << "Process " << rank << " done with initialization." << std::endl;
	Barrier();
	//std::cout << "Process " << rank << " allowed through barrier." << std::endl;

	RunSimulation(100);

	// I want to implement the termination messages, but the barrier works so well
	//std::cout << "Process " << rank << " done with simulation." << std::endl;
	Barrier();
	//std::cout << "Process " << rank << " allowed through barrier." << std::endl;

	CommunicationFinalize();

	auto end_time = std::chrono::high_resolution_clock::now();
	double clock_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
	clock_time /= 1000.0;

	std::cout << "Process " << rank << " program execution time: " << clock_time << "s" << std::endl;
	return 0;
}
