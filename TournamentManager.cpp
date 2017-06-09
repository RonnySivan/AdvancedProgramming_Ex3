#include "TournamentManager.h"

TournamentManager::TournamentManager(const std::string path, int threads) :
	m_path(path), threads(threads)
{
	CLogger::GetLogger()->Log("The program began running!");
}

TournamentManager::~TournamentManager()
{

	CLogger::CloseLogger();

	// Iterator to iterate over dll's vector
	std::vector<std::tuple<std::string, HINSTANCE, GetPlayerFuncType>>::iterator dll_vec_itr;

	// Delete the dll vector
	for (dll_vec_itr = dll_vec.begin(); dll_vec_itr != dll_vec.end(); ++dll_vec_itr)
	{
		FreeLibrary(std::get<1>(*dll_vec_itr));
	}

}

bool TournamentManager::initDllsVector() {
	WIN32_FIND_DATAA fileData; //data struct for file

	HANDLE dir = FindFirstFileA((m_path + "\\*.dll").c_str(), &fileData);
	if (dir != INVALID_HANDLE_VALUE) //check if the dir opened successfully
	{
		do
		{
			HINSTANCE hDll;
			std::string fullFileName;
			if (!findDllFile(fileData, hDll, fullFileName)) {
				continue;
			}

		} while (FindNextFileA(dir, &fileData));
	}
	else {
		std::cout << "Can't open dir to get Algorithm (dll) files from " << std::endl;
		return false;
	}

	if (dll_vec.size() < 2)
	{
		std::cout << "Missing VALID algorithm (dll) files - needs at least two " << std::endl;
		return false;
	}

	return true;

}


bool TournamentManager::initBoardsVector(std::vector<std::string>& allFilesInDir)
{

	std::vector<std::string> foundFiles;
	Util::findAllFilesWithSuffix(allFilesInDir, foundFiles, ".sboard");
	auto isLegal = true;
	OriginalBoard board;
	int size = static_cast<int>(foundFiles.size());
	for (auto i = 0; i < size; ++i)
	{
		std::string errors = "";
		isLegal = board.createBoards(foundFiles[i], errors); // TODO - integrate with Tiana

		if (isLegal)
			boardsVector.push_back(board);

		if (errors.size() > 0)
		{
			auto errorsVector = Util::split(errors, '\n');
			for (auto error : errorsVector)
			{
				LOGGER->Log("%s : %s ", (isLegal) ? "Warning" : "ERROR", error.c_str());
			}
		}
	}

	if (boardsVector.size() == 0)
	{
		std::cout << "Missing LEGAL board (.sboard) file - needs at least one " << std::endl;
		return false;
	}

	return true;
}

void TournamentManager::startTournament() const
{
	std::cout << "Number of legal players: " << dll_vec.size() << std::endl;
	std::cout << "Number of legal boards: " << boardsVector.size() << std::endl;

	/*	// Create the players unique-ptr vector:
	// unique_ptr<IBattleshipGameAlgo> playerA = unique_ptr<IBattleShipGameAlgo> playerA(algofunc());


	createNaiveTournamentSchedule();
	// Start the scores thread


	//Start the Games Threads
	//	while(tournamentSchedule.size() > 0)
	//	{

	//	}

	//	// Initiating the gameManager
	GameManager gameManager(dll_vec[0], dll_vec[1], boardsVector[0]);
	GameResult gameResult = gameManager.runGame();
	*/
}


/////// private methods region
bool TournamentManager::findDllFile(WIN32_FIND_DATAA& fileData, HINSTANCE& hDll, std::string& fullFileName) {
	std::string fileName = fileData.cFileName;
	fullFileName = m_path + "\\" + fileName;
	auto playerName = fileName.substr(0, fileName.find("."));

	// Load dynamic library
	hDll = LoadLibraryA(fullFileName.c_str());
	if (!hDll)
		return false;

	// Get function pointer
	getPlayerFunc = (GetPlayerFuncType)GetProcAddress(hDll, "GetAlgorithm");
	if (!getPlayerFunc)
		return false;

	dll_vec.push_back(std::make_tuple(playerName, hDll, getPlayerFunc));

	return true;
}

void TournamentManager::createNaiveTournamentSchedule()
{
	int size = static_cast<int>(dll_vec.size());
	for (auto i = 0; i < size; ++i) // first players
	{
		for (auto j = 0; j < size; ++j)
		{
			for (auto k = 0; k < static_cast<int>(boardsVector.size()); ++k)
			{
				if (i == j)
					continue;

				tournamentSchedule.push_back(std::make_tuple(i, j, k));
			}
		}
	}

	std::cout << "Number of games: " << tournamentSchedule.size() << std::endl;

}