#include "Airport.h"
#include "SimulationExecutive.h"

int main()
{
	// simulation data viewing can be done with the Data_Viewer.xlsx file
	// need to refresh the connections to the data source

	// takes a while to run when writing all data to files,
	// I recommend checking the directives before running.
	// command arg: -localonly -n 2 "$(TargetPath)"

	CommunicationInitialize();

	int rank = CommunicationRank();

	Airport airport("Airport" + std::to_string(rank+1), NULL);

	std::function<void(int)> msgHandler = std::bind(&Airport::PlaneArrival, &airport, std::placeholders::_1);

	RegisterMsgHandler(msgHandler);
	PopulateAirportWithPlanes(&airport, 10);

	InitializeSimulation();

	//std::cout << "Process " << rank << " done with initialization " << std::endl;
	Barrier();
	//std::cout << "Process " << rank << " allowed through barrier" << std::endl;

	RunSimulation(1000);

	// I want to implement the termination messages, but the barrier works so well
	std::cout << "Process " << rank << " done with simulation" << std::endl;
	Barrier();
	std::cout << "Process " << rank << " allowed through barrier" << std::endl;

	CommunicationFinalize();
	return 0;
}
