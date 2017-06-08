#include "Util.h"
#include "TournamentManager.h"

#define DEFAULT_THREADS 4

int main(int argc, char* argv[])
{
	/* Initialize default main variables */
	auto threads = DEFAULT_THREADS;
	std::string path = "";
	Util::initMain(argc, argv, path, threads);

	/* Get all files in the directory - Exit if Wrong path OR No board files OR Missing algorithm (dll) files */
	std::vector<std::string> allFilesInDir;
	if (!Util::findFiles(path, allFilesInDir))
		return -1;

	/* Initiating the Tournament Manager */
	TournamentManager tournamentManager(path, threads);

	/* Initiating the dll vector - exit if there isn't (at least) two valid dll's */
	if (!tournamentManager.initDllsVector())
		return -1;

	/* Initiating boards - exit if there isn't a legal board*/
	if (!tournamentManager.initBoardsVector(allFilesInDir))
		return -1;

	tournamentManager.startTournament();


	return 0;
}