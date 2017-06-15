#pragma once
#include "Util.h"
#include "GameManager.h"
#include "Logger.h"

#include <thread> //for threads management
#include <mutex> //for threads management
#include <condition_variable> //for threads management
#include <deque> //for threads management

#include <memory>
#include <iomanip> // for print_scores
#include <algorithm> // for print_scores + std::random_shuffle

#define DEFAULT_THREADS_NUM 4


class TournamentManager
{
	std::string m_path;
	std::vector<std::string> m_allFilesInDir;
	int m_threads;
	int m_numOfPlayers;
	int m_numOfGames;

	std::mutex m_scoreBalanceMutex; //update the score Balance - each thread at his time.
	std::mutex m_getGameMutex; // get the first game representation from the gameSchedule deque (so no two threads taking the same game).
	std::mutex m_startThreadsMutex; // Start the threads action at the same time.
	std::mutex m_finishCyclesMutex; 
	std::condition_variable finishAllCyclesCV;

	volatile bool startThreads = false; 
	std::condition_variable startThreadsCV;

	std::vector<std::vector<std::tuple<int, int, int>>> allGameResults; //<points gained, points lost, has won?>
	std::vector<std::tuple<std::string, int, int, double, int, int>> scoreBalance; // name, wins, loses, percent, pts_for, pts_against
	int m_currentRound;
	std::vector<int> playedRound;
	std::deque<std::tuple<int , int , int>> tournamentSchedule;

	// define function of the type we expect from IBattleshipGameAlgo
	typedef IBattleshipGameAlgo *(*GetPlayerFuncType)();
	GetPlayerFuncType getPlayerFunc;

	// vector of <Player Name, dll handle, GetPlayer function ptr>
	std::vector<std::tuple<std::string, HINSTANCE, GetPlayerFuncType>> dll_vec;
	std::vector<std::shared_ptr<OriginalBoard>> boardsVector; 

	/***************** private methods ************/

	/*
	* gets handle to dll file and tries to load it
	* if succeeded, add new algorithm to dll_vec
	* return true iff succeeded
	*/
	bool findDllFile(WIN32_FIND_DATAA& fileData, HINSTANCE& hDll, std::string& fullFileName);


	/*
	 * Create the tournament schedule - according num of players and boards.
	 * Each player will play against all other players, on each board - TWICE (switch positions)!
	 */
	void createTournamentSchedule();


	/*
	* gets the names of all the files in the path given and extracts the default parameters from config file
	* returns true iff succedded
	*/
	void setDefaultArgs();

	/**
	 * \brief prints tournament's scores according to the required format
	 * \param scores vector holding the scores in the following tuple format: Team Name, Wins, Losses, %, Pts For, Pts Against
	 */
	void print_scores(std::vector< std::tuple< std::string, int, int, double, int, int > > scores) const;


	/**
	* Update the tournament-all-games-scores according to the last game played.
	* params playerIdFirst and playerIdSecond represents the players Id's in the playersVector \ score chart.
	* param gameResult holding the scores and the winner of the last game played. 
	* Use mutex update each game seperatly.
	*/
	void updateScoreBalance(int playerIdFirst, int PlayerIdSecond, GameResult gameResult);


	/*
	 * Update the tournament-score-balance according to the last round completed.
	 */
	void updateScoreBalanceTable();


	/**
	 * The methos that each thread makes:
	 * calls the gameManager to run the game and pass the gameResult to the scoreBalance */
	void singleThreadMethod();

	/**
	 * Assign the input tuple with the next game to play, from the gameSchedule.
	 */
	void getGame(std::tuple<int, int, int>& game);

public:
	/* Constructor
	* initializes all fields
	*/
	TournamentManager();

	/*destructor for TournamentManager
	* delete the dll_vec
	*/
	~TournamentManager();

	// block copy and assignment c'tor
	TournamentManager(const TournamentManager&) = delete;
	TournamentManager& operator=(const TournamentManager&) = delete;


	/**
	* initializes program veriables
	*/
	bool initTournament(int argc, char* argv[]);


	/*
	* return true if one .sboard file and two dll files exist in m_path folder.
	*/
	bool findBoardAndDlls();


	/*
	* Load the dll files in the folder, and push them into a "dll_vec".
	* Return false if there is less than 2 valid dll's.
	*/
	bool initDllsVector();

	/*
	 * Load the .sboard files, and push them into "boardsVector".
	 * Return false if there aren't legal files. 
	 */
	bool initBoardsVector ();

	/**
	 * start the tournament - print num of players&boards, start the game threads, print the scores status. 
	 */
	void startTournament();
};
