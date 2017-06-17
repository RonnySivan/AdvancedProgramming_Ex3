#include "TournamentManager.h"
#include "Logger.h"
#include <condition_variable> //for threads management
#include <iomanip> // for print_scores
#include <algorithm> // for print_scores + std::random_shuffle
#include <iostream>
#include <fstream>
#include <map> // for print_scores


TournamentManager::TournamentManager() :
	m_path(""), 
	m_threads(DEFAULT_THREADS_NUM), m_numOfPlayers(0), m_numOfBoards(0), m_numOfGames(0), m_numOfCycles(0), 
	m_wakeMain(false), m_finishedGames(false)
{
	CLogger::GetLogger()->Log("The program began running!");
}

TournamentManager::~TournamentManager()
{

	// Delete the dll vector
	for (auto dll_vec_itr = dll_vec.begin(); dll_vec_itr != dll_vec.end(); ++dll_vec_itr)
	{
		FreeLibrary(std::get<1>(*dll_vec_itr));
		std::get<1>(*dll_vec_itr) = nullptr;
	}

	CLogger::CloseLogger();
}

bool TournamentManager::initTournament(int argc, char* argv[])
{
	auto givenThreads = false;

	for (auto i = 1; i < argc; ++i)
	{
		if (!strcmp(argv[i], "-threads"))
		{
			if ((i + 1) < argc)
			{
				if (parseIntArgument(argv[i + 1]))
					givenThreads = true;
				else
					CLogger::GetLogger()->Log("Warning: threads parameter given is illegal");
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
		CLogger::GetLogger()->Log("Error: Wrong path: %s", m_path.c_str());
		std::cout << "Wrong path: " << m_path << std::endl;
		return false;
	}
	if (! givenThreads) {
		CLogger::GetLogger()->Log("Info: Using default parameter for -threads");
		setDefaultArgs();
	}
	
	CLogger::GetLogger()->Log("Info: Using path = %s", m_path.c_str());
	CLogger::GetLogger()->Log("Info: Using threads = %d", m_threads);
	return true;
}

void TournamentManager::setDefaultArgs()
{
	auto configFile = Util::findSuffix(m_allFilesInDir, ".config", 1);
	if (configFile.compare("") == 0) {
		CLogger::GetLogger()->Log("Warning: *.config file is missing from path: %s", m_path.c_str());
		return;
	}
	std::string line;
	std::ifstream fin(configFile);

	if (!fin.is_open()) {
		CLogger::GetLogger()->Log("Warning: Couldn't open file: <%s>, in path: %s", 
									configFile.c_str(), m_path.c_str());
		return;
	}
	while (getline(fin, line))
	{
		auto tokens = Util::split(line, ' ');
		if (tokens.size() < 2 || tokens.at(0).compare("-threads"))
			continue;
		if (parseIntArgument(tokens.at(1)))
			return;
		CLogger::GetLogger()->Log("Warning: The default parameter for -threads given in the *config file <%s> is illegal",
			configFile.c_str());
	}

	CLogger::GetLogger()->Log("Warning: The *config file <%s> doesn't contain a valid argument for -thread parameter", 
			configFile.c_str());
}

bool TournamentManager::parseIntArgument(std::string arg)
{
	try
	{
		auto value = std::stoi(arg);
		if (value > 0)
		{
			m_threads = value;
			return true;
		}
		return false;
	}
	catch (std::invalid_argument)
	{
		return false;
	}
	catch (std::out_of_range)
	{
		return false;
	}
}

bool TournamentManager::findBoardAndDlls()
{
	auto fileBoard = Util::findSuffix(m_allFilesInDir, ".sboard", 1);
	auto fileDll1 = Util::findSuffix(m_allFilesInDir, ".dll", 1);
	auto fileDll2 = Util::findSuffix(m_allFilesInDir, ".dll", 2);

	if (fileBoard.compare("") == 0) {
		CLogger::GetLogger()->Log("Error: No board files (*.sboard) looking in path: <%s>", m_path.c_str());
		std::cout << "No board files (*.sboard) looking in path: " << m_path << std::endl;
		return false;
	}

	if (fileDll1.compare("") == 0 || fileDll2.compare("") == 0)
	{
		CLogger::GetLogger()->Log("Error: Missing algorithm (dll) files looking in path: <%s>", m_path.c_str());
		std::cout << "Missing algorithm (dll) files looking in path: " << m_path.c_str() << " (needs at least two)" << std::endl;
		return false;
	}

	return true;
}

bool TournamentManager::initDllsVector() {
	WIN32_FIND_DATAA fileData; //data struct for file

	auto dir = FindFirstFileA((m_path + "\\*.dll").c_str(), &fileData);
	if (dir != INVALID_HANDLE_VALUE) //check if the dir opened successfully
	{
		do
		{
			HINSTANCE hDll;
			std::string fullFileName;
			if (!findDllFile(fileData, hDll, fullFileName)) {
				CLogger::GetLogger()->Log("Warning: Couldn't find dll file: <%s>", fullFileName.c_str());
			}

		} while (FindNextFileA(dir, &fileData));
	}
	else {
		CLogger::GetLogger()->Log("Error: Can't open dir to get Algorithm (dll) files from");
		return false;
	}

	m_numOfPlayers = static_cast<int>(dll_vec.size());
	if (m_numOfPlayers < 2)
	{
		CLogger::GetLogger()->Log("Error: Missing VALID algorithm (dll) files - needs at least two");
		std::cout << "Missing VALID algorithm (dll) files - needs at least two" << std::endl;
		return false;
	}

	return true;
}

bool TournamentManager::initBoardsVector()
{
	std::vector<std::string> foundFiles;
	Util::findAllFilesWithSuffix(m_allFilesInDir, foundFiles, ".sboard");
	auto size = static_cast<int>(foundFiles.size());

	for (auto i = 0; i < size; ++i)
	{
		std::string errors = "";
		auto board = std::make_shared<OriginalBoard>();
		auto isLegal = (board.get())->createBoards(foundFiles[i], errors);

		if (isLegal)
			boardsVector.push_back(board);
			
		if (errors.size() > 0)
		{
			auto errorsVector = Util::split(errors, '\n');
			for (auto error : errorsVector)
			{
				LOGGER->Log("%s : %s in sboard file: %s", (isLegal) ? "Warning" : "ERROR", error.c_str(), foundFiles[i].c_str());
			}
		}
	}

	m_numOfBoards = static_cast<int>(boardsVector.size());
	if (m_numOfBoards == 0)
	{
		LOGGER->Log("ERROR: Missing LEGAL board (.sboard) file - needs at least one");
		std::cout << "Missing LEGAL board (.sboard) file - needs at least one" << std::endl;
		return false;
	}

	return true;
}

void TournamentManager::startTournament()
{
	std::cout << "Number of legal players: " << m_numOfPlayers << std::endl;
	std::cout << "Number of legal boards: " << m_numOfBoards << std::endl;
	prepareTournamentVariables();
	int currentRound = 0;
	// Start the tournament using m_threads threads
	std::vector<std::thread> vec_threads(m_threads);
	for (auto & t : vec_threads) {
		t = std::thread(&TournamentManager::singleThreadMethod, this);
	}

	/* fire the "start" signal for all threads */
	startThreads = true;
	startThreadsCV.notify_all();

	/* After each cycle finished - update and print the score table */
	while (currentRound < m_numOfCycles) {
		std::unique_lock<std::mutex> lock(m_finishOneCycleMutex);
		finishOneCyclesCV.wait(lock, [this] {return m_wakeMain.load() || m_finishedGames.load();  });
		while (playedRound[currentRound] == m_numOfPlayers) {
			updateScoreBalanceTable(currentRound);
			currentRound++;
		}
		m_wakeMain = false;
	}
		
	/* Wait for all threads */
	for (auto & t : vec_threads) {
		t.join();
	}
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
	GetPlayerFuncType getPlayerFunc = reinterpret_cast<GetPlayerFuncType>(GetProcAddress(hDll, "GetAlgorithm"));
	if (!getPlayerFunc)
		return false;

	dll_vec.push_back(std::make_tuple(playerName, hDll, getPlayerFunc));

	return true;
}


void TournamentManager::print_scores(const std::vector<std::tuple<std::string, int, int, double, int, int>>& scores) const
{
	std::string name;
	int win, losses, pts_for, pts_against;
	double percent;

	// find max name length for column width
	auto it = std::max_element(scores.begin(), scores.end(), [](auto const &t1, auto const &t2) {
		return std::get<0>(t1) < std::get<0>(t2);
	});
	auto max_name_size = std::get<0>(*it).length();

	// sort according to highest percent of wins - map them to vector indexes
	std::multimap<double, int, std::greater<double>> percent_of_wins_map;
	for (auto i = 0; i < scores.size(); i++) {
		percent_of_wins_map.insert({ std::get<3>(scores[i]), i });
	}

	// print headlines
	std::cout << "#\t" << std::left << std::setw(max_name_size + 2) << "Team Name" << "\tWins\tLosses\t%\tPts For\tPts Against" << std::endl << std::endl;

	// print results
	auto num = 1;
	for (const auto& item : percent_of_wins_map) {
		std::tie(name, win, losses, percent, pts_for, pts_against) = scores[item.second];
		std::cout << num << ".\t"
			<< std::left << std::setw(max_name_size + 2) << name
			<< "\t" << win
			<< "\t" << losses
			<< "\t" << std::setprecision(4) << percent
			<< "\t" << pts_for
			<< "\t" << pts_against
			<< std::endl;
		num++;
	}
	std::cout << std::endl << std::endl;
}

void TournamentManager::updateScoreBalance(int playerIdFirst, int PlayerIdSecond, GameResult gameResult)
{
	m_scoreBalanceMutex.lock();

	allGameResults[playerIdFirst].push_back(std::make_tuple(gameResult.scorePlayerA, gameResult.scorePlayerB, gameResult.winnerId == 0 ? 1 : 0));
	auto currentRound_playerA = static_cast<int>(allGameResults[playerIdFirst].size());
	playedRound[currentRound_playerA - 1]++;

	allGameResults[PlayerIdSecond].push_back(std::make_tuple(gameResult.scorePlayerB, gameResult.scorePlayerA, gameResult.winnerId == 1 ? 1 : 0));
	auto currentRound_playerB = static_cast<int>(allGameResults[PlayerIdSecond].size());
	playedRound[currentRound_playerB - 1]++;
	
	m_wakeMain = true;
	finishOneCyclesCV.notify_all();

	m_scoreBalanceMutex.unlock();
}


void TournamentManager::updateScoreBalanceTable(int currentRound)
{
	for (auto i = 0; i < m_numOfPlayers; i++) {
		// update the Wins & Losses cols
		std::get<2>(allGameResults[i][currentRound]) == 1 ? std::get<1>(scoreBalance[i])++ : std::get<2>(scoreBalance[i])++;

		// Update the % col
		std::get<3>(scoreBalance[i]) = (static_cast<double>(std::get<1>(scoreBalance[i])) / (std::get<1>(scoreBalance[i]) + std::get<2>(scoreBalance[i]))) * 100;

		// update the Pts For & Pts Against cols
		std::get<4>(scoreBalance[i]) += std::get<0>(allGameResults[i][currentRound]);
		std::get<5>(scoreBalance[i]) += std::get<1>(allGameResults[i][currentRound]);
	}

	print_scores(scoreBalance);
}


void TournamentManager::createTournamentSchedule()
{
	for (auto i = 0; i < m_numOfPlayers; ++i)
	{
		for (auto j = 0; j < m_numOfPlayers; ++j) //choose the second Player
		{
			for (auto k = 0; k < m_numOfBoards; ++k) //choose the relevant board
			{ 
				if (i != j)
				{
					tournamentSchedule.push_back(std::make_tuple(i, j, k));
				}
			}
		}
	}


	std::random_shuffle(tournamentSchedule.begin(), tournamentSchedule.end());

//	std::cout << "Number of legal gamess: " << tournamentSchedule.size() << std::endl;

}

void TournamentManager::singleThreadMethod()
{
	// wait for "start" sign
	{
		std::unique_lock<std::mutex> lock(m_startThreadsMutex);
		startThreadsCV.wait(lock, [this] {return startThreads; });
	}
	std::tuple<int, int, int> gameRepresentation;

	while (!m_finishedGames.load())
	{
		getGame(gameRepresentation);

		auto firstPlayerId = std::get<0>(gameRepresentation);
		auto secondPlayerId = std::get<1>(gameRepresentation);
		auto boardId = std::get<2>(gameRepresentation);

		// end of games to play (in the all schedule), exit the thread. 
		if (firstPlayerId == -1 && secondPlayerId == -1 && boardId == -1)
		{
			break;
		}

		/* Create the two players, and a gameManager - and run it with both players and board.*/
		std::unique_ptr<IBattleshipGameAlgo> playerA(std::get<2>(dll_vec[firstPlayerId])());
		std::unique_ptr<IBattleshipGameAlgo> playerB(std::get<2>(dll_vec[secondPlayerId])());

		GameManager gameManager(std::move(playerA), std::move(playerB), boardsVector[boardId]);
		auto gameResult = gameManager.runGame();

		/* update the gameResult to the current cycle score chart */
		updateScoreBalance(firstPlayerId, secondPlayerId, gameResult);
	}
}

void TournamentManager::getGame(std::tuple<int, int, int>& game)
{
	m_getGameMutex.lock();
	if (!tournamentSchedule.empty())
	{
		game = tournamentSchedule.front();
		tournamentSchedule.pop_front();
	} 
	else
	{
		CLogger::GetLogger()->Log("tournamentSchedule but someone asks for a game");
	}
	if (tournamentSchedule.empty())
	{
		m_finishedGames = true;
	}
	m_getGameMutex.unlock();
}

void TournamentManager::prepareTournamentVariables()
{
	/* Create the scoreBalance chart */
	for (auto player : dll_vec)
	{
		auto playerName = std::get<0>(player);
		scoreBalance.push_back(std::make_tuple(playerName, 0, 0, 0.0, 0, 0));
		allGameResults.push_back(std::vector<std::tuple<int, int, int>>());
	}

	/* Create the Games-Schedule */
	createTournamentSchedule();
	m_numOfGames = static_cast<int>(tournamentSchedule.size());
	m_numOfCycles = (m_numOfGames / m_numOfPlayers) * 2;

	/* Initialize the "playedRound" variable - how many players played each round */
	for (auto i = 0; i < m_numOfCycles; ++i)
	{
		playedRound.push_back(0);
	}

	/* If there are too much threads (more than optionl games) - we don't need to use all of the threads */
	if (m_threads > m_numOfGames)
		m_threads = m_numOfGames;

	/* Insert custom game-representation objects, to mark the end of the gameSchedule */
	for (auto i = 0; i < m_threads; ++i)
	{
		tournamentSchedule.push_back(std::make_tuple(-1, -1, -1));
	}

}
