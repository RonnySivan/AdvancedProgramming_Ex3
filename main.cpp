#include "TournamentManager.h"

int main(int argc, char* argv[])
{

	/* Initiating the Tournament Manager */
	TournamentManager tournamentManager;
	if (!tournamentManager.initTournament(argc, argv))
		return -1;

	/* Get all files in the directory - Exit if Wrong path OR No board files OR Missing algorithm (dll) files */
	if (!tournamentManager.findBoardAndDlls())
		return -1;

	/* Initiating the dll vector - exit if there isn't (at least) two valid dll's */
	if (!tournamentManager.initDllsVector())
		return -1;

	/* Initiating boards - exit if there isn't a legal board*/
	if (!tournamentManager.initBoardsVector())
		return -1;

	tournamentManager.startTournament();
	
	return 0;
}