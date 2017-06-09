#include "TournamentManager.h"

TournamentManager::TournamentManager() :
	m_threads(DEFAULT_THREADS_NUM), m_path("")
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

bool TournamentManager::initTournament(int argc, char* argv[])
{
	bool givenThreads = false;

	for (auto i = 1; i < argc; ++i)
	{
		if (!strcmp(argv[i], "-threads"))
		{
			if ((i + 1) < argc)
			{
				try
				{
					m_threads = std::stoi(argv[i + 1]);
					givenThreads = true;

				}
				catch (std::invalid_argument) { }
				catch (std::out_of_range) { }
			}
			break;
		}
		if (i == 1)
		{
			m_path = argv[i];
		}
	}

	m_path.erase(std::remove(m_path.begin(), m_path.end(), ' '), m_path.end());
	m_path = Util::findAbsPath(m_path.c_str());
	if (! Util::findFiles(m_path, m_allFilesInDir))
	{
		CLogger::GetLogger()->Log("Error: Wrong path: <%s>", m_path);
		//write to logger error
		return false;
	}
	if (! givenThreads) {
		CLogger::GetLogger()->Log("Info: Using default parameter for -threads");
		setDefaultArgs();
	}
	return true;
}


void TournamentManager::setDefaultArgs()
{
	std::string configFile = Util::findSuffix(m_allFilesInDir, ".config", 1);
	if (configFile.compare("") == 0) {
		CLogger::GetLogger()->Log("Warning: *.config file is missing from path: <%s>", m_path);
		return;
	}
	std::string line;
	std::ifstream fin(configFile);

	if (!fin.is_open()) {
		CLogger::GetLogger()->Log("Warning: Couldn't open file: <%s>, in path: <%s>", configFile, m_path);
		return;
	}
	while (getline(fin, line))
	{
		std::vector<std::string> tokens = Util::split(line, ' ');
		if (tokens.size() < 2)
		{
			continue;
		}
		try
		{
			auto value = std::stoi(tokens.at(1));
			if (tokens.at(0).compare("-threads") == 0)
			{
				m_threads = value;
				return;
			}
		}
		catch (std::invalid_argument) { }
		catch (std::out_of_range) { }
	}

	CLogger::GetLogger()->Log("Warning: The file <%s> doesn't contain a valid argument for -thread parameter:", configFile);
}

void TournamentManager::print_scores(std::vector<std::tuple<std::string, int, int, double, int, int>> scores)
{
	std::string name;
	int win, losses, pts_for, pts_against;
	double percent;

	// find max name length for column width
	auto it = std::max_element(scores.begin(), scores.end(), [](auto const &t1, auto const &t2) {
		return std::get<0>(t1) > std::get<0>(t2);
	});
	int max_name_size = std::get<0>(*it).length();

	// sort according to highest percent of wins
	std::sort(begin(scores), end(scores), [](auto const &t1, auto const &t2) {
		return std::get<3>(t1) > std::get<3>(t2);
	});

	// print headlines
	std::cout << "#\t" << std::left << std::setw(max_name_size + 2) << "Team Name" << "\tWins\tLosses\t%\tPts For\tPts Against" << std::endl;

	// print results
	for (size_t i = 1; i <= scores.size(); i++)
	{
		std::tie(name, win, losses, percent, pts_for, pts_against) = scores[i - 1];
		std::cout << i << ".\t"
			<< std::left << std::setw(max_name_size + 2) << name
			<< "\t" << win
			<< "\t" << losses
			<< "\t" << percent
			<< "\t" << pts_for
			<< "\t" << pts_against
			<< std::endl;
	}
}


bool TournamentManager::findBoardAndDlls()
{
	std::string fileBoard = Util::findSuffix(m_allFilesInDir, ".sboard", 1);
	std::string fileDll1 = Util::findSuffix(m_allFilesInDir, ".dll", 1);
	std::string fileDll2 = Util::findSuffix(m_allFilesInDir, ".dll", 2);

	if (fileBoard.compare("") == 0) {
		CLogger::GetLogger()->Log("Error: No board files (*.sboard) looking in path: <%s>", m_path);
		std::cout << "No board files (*.sboard) looking in path: " << m_path << std::endl;
		return false;
	}

	if (fileDll1.compare("") == 0 || fileDll2.compare("") == 0)
	{
		CLogger::GetLogger()->Log("Error: Missing algorithm (dll) files looking in path: <%s>", m_path);
		std::cout << "Missing algorithm (dll) files looking in path: " << m_path << " (needs at least two)" << std::endl;
		return false;
	}

	return true;
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
				CLogger::GetLogger()->Log("Warning: Couldn't find dll file: <%s>", fullFileName);
			}

		} while (FindNextFileA(dir, &fileData));
	}
	else {
		CLogger::GetLogger()->Log("Error: Can't open dir to get Algorithm (dll) files from");
		std::cout << "Can't open dir to get Algorithm (dll) files from " << std::endl;
		return false;
	}

	if (dll_vec.size() < 2)
	{
		CLogger::GetLogger()->Log("Error: Missing VALID algorithm (dll) files - needs at least two: <%s>");
		std::cout << "Missing VALID algorithm (dll) files - needs at least two " << std::endl;
		return false;
	}

	return true;
}


bool TournamentManager::initBoardsVector()
{
	std::vector<std::string> foundFiles;
	Util::findAllFilesWithSuffix(m_allFilesInDir, foundFiles, ".sboard");
	auto isLegal = true;
	int size = static_cast<int>(foundFiles.size());
	for (auto i = 0; i < size; ++i)
	{
		std::string errors = "";
		std::shared_ptr<OriginalBoard> board = std::make_shared<OriginalBoard>();
		isLegal = (board.get())->createBoards(foundFiles[i], errors); // TODO - integrate with Tiana

		if (isLegal)
			boardsVector.push_back(board);
			
		if (errors.size() > 0)
		{
			auto errorsVector = Util::split(errors, '\n');
			for (auto error : errorsVector)
			{
				LOGGER->Log("%s : %s in sboard file: %s", (isLegal) ? "Warning" : "ERROR", error.c_str(), foundFiles[i]);
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