#include "Communication.h"
#include "mpi.h"
#include <iostream>
using namespace std;

int processID = -1;
int numProcess = -1;

void CommunicationInitialize()
{

	int *argc = 0;
	char ***argv = 0;
	MPI_Init( NULL, NULL);
	MPI_Comm_rank( MPI_COMM_WORLD, &processID);
	MPI_Comm_size( MPI_COMM_WORLD, &numProcess);

	cout << processID << " in initialize" << endl;
	cout << processID << " done initialize" << endl;
}

void CommunicationFinalize()
{
	cout << processID << " in finalize" << endl;
	MPI_Finalize();
	cout << processID << " done finalize" << endl;
}

int CommunicationRank()
{
	return processID;
}

int CommunicationSize()
{
	return numProcess;
}

bool CheckForComm( int &tag, int &source)
{
	MPI_Status status;
	int flag;
	MPI_Iprobe( MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &status); 
	if (flag) {
		tag = status.MPI_TAG;
		source = status.MPI_SOURCE; 
	}
	return( flag == 1);
}

void BroadcastTerminationMessage()
{
	for (int proc = 0; proc < numProcess; proc++) {
		if (proc == CommunicationRank()) {
			continue;
		}
		else {
			int* emptyBuffer = new int[1];
			MPI_Request request;
			std::cout << "Proc " << CommunicationRank() << " Sending tag 0 to " << proc << std::endl;

			MPI_Isend(emptyBuffer, 1, MPI_INTEGER, proc, 0, MPI_COMM_WORLD, &request);
		}
	}
}

void Barrier()
{
	MPI_Barrier(MPI_COMM_WORLD);
}

CommunicationPattern::CommunicationPattern()
{
}

void CommunicationPattern::Send( int dest)
{
	int bufferSize;
	int *dataBuffer;

	MPI_Request request;
	bufferSize = GetBufferSize();
	dataBuffer = new int[bufferSize];
	Serialize( dataBuffer);
	MPI_Isend( dataBuffer, bufferSize, MPI_INTEGER, dest, 1, MPI_COMM_WORLD, &request);
}

void CommunicationPattern::Receive( int source)
{
	int bufferSize;
	int *dataBuffer;

	MPI_Request request;
	bufferSize = GetBufferSize();
	dataBuffer = new int[bufferSize];
	MPI_Irecv( dataBuffer, bufferSize, MPI_INTEGER, source, 1, MPI_COMM_WORLD, &request);
	Deserialize( dataBuffer);
}
