#pragma once
#include "GameBoard.h"
#include "PlayerBoard.h"

#include <vector>
#include <iostream>
#include <memory>


#define PLAYER_A 0
#define PLAYER_B 1


struct GameResult
{
	int winnerId, scorePlayerA, scorePlayerB;
	GameResult(int winnerId_, int scoreA_, int scoreB_) : winnerId(winnerId_), scorePlayerA(scoreA_), scorePlayerB(scoreB_) {}
};

class GameManager
{
	GameBoard gameBoard;
	std::unique_ptr<IBattleshipGameAlgo> playerA;
	std::unique_ptr<IBattleshipGameAlgo> playerB;
	std::vector<BattleShip> battleShipsA;
	std::vector<BattleShip> battleShipsB;
	int numOfShipsA;
	int numOfShipsB;
	bool hasMoreMovesA;
	bool hasMoreMovesB;
	int m_scorePlayerA;
	int m_scorePlayerB;


	/*
	* run turn of playerId player
	* checks if attack indexes player returned are legal
	* notifies both players on the attack results
	* return turn of the next player in the game
	*/
	int runPlayer(int playerId);

	/*
	* Check if the indexes that the player tries to attack are legal (in the boards limits)
	* return a bbolean answer accordingly.
	*/
	bool attackCoordinateLegal(Coordinate attackCoordinate) const;

	/*
	* receives legal attack Coordinates
	* checks attack result
	* decides to whom belongs the next turn and returns it
	*/
	int analyzeLegalAttack(int playerId, int turn, std::pair<AttackResult, int>& attackResAndPlayer, Coordinate attackCoordinates);

	/*
	* When there is a Hit in playerId board, in attackMove Coordinate,
	* this function will check if it was a Hit or a Sink
	* if it was a sink - it will update the score accordingly, and call 'updateSinkShipInBoard' function to change 'h\H' to 'x\X'.
	* Return - Hit or Sink
	*/
	AttackResult checkHitResult(int playerId, Coordinate attackMove, std::vector<BattleShip> &battleShips);

	/* If a battleShip had sink -
	*  Update all of the BattleShip's occurrences in the board to be 'x' (Or 'X').
	*/
	void updateSinkShipInBoard(int playerID, BattleShip& battleShip);


public:
	/* Constructor
	* initializes all fields
	*/
	GameManager(std::unique_ptr<IBattleshipGameAlgo> playerA_, std::unique_ptr<IBattleshipGameAlgo> playerB_, std::shared_ptr<OriginalBoard> originalBoard);

	/* Empty destructor for GameManager
	*/
	~GameManager();

	// block copy and assignment c'tor
	GameManager(const GameManager&) = delete;
	GameManager& operator=(const GameManager&) = delete;

	//GameResult runGame();
	GameResult runGame();
};
