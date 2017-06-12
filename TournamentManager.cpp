﻿#include "TournamentManager.h"

TournamentManager::TournamentManager() :
	m_path(""), m_threads(DEFAULT_THREADS_NUM), m_numOfPlayers(0), m_currentRound(0)
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
				try
				{
					auto threadsNum = std::stoi(argv[i + 1]);
					if (threadsNum < 2)
					{
						CLogger::GetLogger()->Log("Error: threads parameter given is illegal");
					}
					else 
					{
						m_threads = threadsNum;
						givenThreads = true;
					}

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
		CLogger::GetLogger()->Log("Error: Wrong path: %s", m_path.c_str());
		std::cout << "Error: Wrong path: " << m_path << std::endl;
		return false;
	}
	if (! givenThreads) {
		CLogger::GetLogger()->Log("Info: Using default parameter for -threads");
		setDefaultArgs();
	}
	
	CLogger::GetLogger()->Log("Info: Using path = %s", m_path.c_str());
	CLogger::GetLogger()->Log("Info: Using -threads = %d", m_threads);
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
		if (tokens.size() < 2)
		{
			continue;
		}
		try
		{
			auto value = std::stoi(tokens.at(1));
			if (tokens.at(0).compare("-threads") == 0 && value >= 2)
			{
				m_threads = value;
				return;
			}
		}
		catch (std::invalid_argument) { }
		catch (std::out_of_range) { }
	}

	CLogger::GetLogger()->Log("Warning: The *config file <%s> doesn't contain a valid argument for -thread parameter", 
								configFile.c_str());
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
	auto isLegal = true;
	auto size = static_cast<int>(foundFiles.size());

	for (auto i = 0; i < size; ++i)
	{
		std::string errors = "";
		auto board = std::make_shared<OriginalBoard>();
		isLegal = (board.get())->createBoards(foundFiles[i], errors);

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

	if (boardsVector.size() == 0)
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
	std::cout << "Number of legal boards: " << boardsVector.size() << std::endl;

	/* Create the scoreBalance chart */
	for (auto player: dll_vec)
	{
		auto fileName = std::get<0>(player);
		scoreBalance.push_back(std::make_tuple(fileName , 0, 0, 0.0, 0, 0));
	}

	/* Start One Game */
	GameManager gameManager(std::move(playersVector[0]), std::move(playersVector[1]), boardsVector[0]);
	auto gameResult = gameManager.runGame();

	/* Update the score chart and Print the result */
	updateScoreBalance( 0, 1, gameResult);
	print_scores(scoreBalance);
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
	getPlayerFunc = reinterpret_cast<GetPlayerFuncType>(GetProcAddress(hDll, "GetAlgorithm"));
	if (!getPlayerFunc)
		return false;

	dll_vec.push_back(std::make_tuple(playerName, hDll, getPlayerFunc));
	std::unique_ptr<IBattleshipGameAlgo> player(std::get<2>(dll_vec.back())());
	playersVector.push_back(std::move(player));

	return true;
}


void TournamentManager::print_scores(std::vector<std::tuple<std::string, int, int, double, int, int>> scores) const
{
	std::string name;
	int win, losses, pts_for, pts_against;
	double percent;

	// find max name length for column width
	auto it = std::max_element(scores.begin(), scores.end(), [](auto const &t1, auto const &t2) {
		return std::get<0>(t1) < std::get<0>(t2);
	});
	size_t max_name_size = std::get<0>(*it).length();

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

void TournamentManager::updateScoreBalance(int playerIdFirst, int PlayerIdSecond, GameResult gameResult)
{
	m_scoreBalanceMutex.lock();

	allGameResults[playerIdFirst].push_back(std::tuple<int, int, int>(gameResult.scorePlayerA, gameResult.scorePlayerB, playerIdFirst == gameResult.winnerId));
	int roundAPlayed = allGameResults[playerIdFirst].size();
	playedRound[roundAPlayed]++;
	allGameResults[PlayerIdSecond].push_back(std::tuple<int, int, int>(gameResult.scorePlayerB, gameResult.scorePlayerA, PlayerIdSecond == gameResult.winnerId));
	int roundBPlayed = allGameResults[PlayerIdSecond].size();
	playedRound[roundBPlayed]++;

	while (playedRound[m_currentRound] == m_numOfPlayers) {
		updateScoreBalanceTable();
	}
	
	m_scoreBalanceMutex.unlock();
}

void TournamentManager::updateScoreBalanceTable()
{
	for (int i = 0; i < m_numOfPlayers; i++) {
		// update the Wins & Losses cols
		std::get<1>(scoreBalance[i]) += std::get<2>(allGameResults[i][m_currentRound]); //update number of wins
		std::get<2>(scoreBalance[i]) += ! std::get<2>(allGameResults[i][m_currentRound]); //update number of loses
		
		// Update the % col
		std::get<3>(scoreBalance[i]) = (std::get<1>(scoreBalance[i]) / (m_currentRound + 1)) * 100;

		// update the Pts For & Pts Against cols
		std::get<4>(scoreBalance[i]) += std::get<0>(allGameResults[i][m_currentRound]);
		std::get<5>(scoreBalance[i]) += std::get<1>(allGameResults[i][m_currentRound]);
	}
	m_currentRound++;
	//TODO wake up the thread that prints to screen the score balance 
}