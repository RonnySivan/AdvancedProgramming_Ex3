#pragma once

#include "IBattleshipGameAlgo.h"

#include "BattleShip.h"
#include "Util.h"
#include <vector>
#include <map>


#define boardErrorWrongSizeForShipB 0
#define boardErrorWrongSizeForShipP 1
#define boardErrorWrongSizeForShipM 2
#define boardErrorWrongSizeForShipD 3
#define boardErrorWrongSizeForShipb 4
#define boardErrorWrongSizeForShipp 5
#define boardErrorWrongSizeForShipm 6
#define boardErrorWrongSizeForShipd 7
#define boardErrorTooManyA 8
#define boardErrorTooFewA 9
#define boardErrorTooManyB 10
#define boardErrorTooFewB 11
#define boardErrorAdjShips 12
#define numOfBoardErrors 13
#define numOfPlayerBoardErrors 5

#define numOfPlayerShips 5

class OriginalBoard : public BoardData
{
private:
	std::map<Coordinate, char> _boardMap;
	std::vector<BattleShip> _battleShipsA;
	std::vector<BattleShip> _battleShipsB;

	/*returns errno of ship of type <type>*/
	static int typeToErr(char type);

	/* prints all errors whose errno matches the "true" elements in <errors>*/
	static bool printErr(bool errors[numOfBoardErrors], std::string& errorStr);

	/*get board size from file*/
	bool getBoardSize(std::string& line);

	/*update all map elements corresponding to row in depth*/
	void parseLine(std::string& line, int depth, int row);

	/*
	*recieves path for file containing board
	*updates the general board and the boards for both players
	*/
	bool parseBoards(const std::string& boardPath);

	void isEmptyNeighbors(int r, int c, int d, bool checkVert, bool checkHorz, bool checkDepth, 
		std::pair<bool, bool> isIllegal) const;

	/*
	* receives a board and location of the beggining of the potential ship
	* checks if the vertical sequence is a legal ship
	* returns <a, b> where
	* a == true iff wrong shape of the ship
	* b == true iff there are adjancent ship with the given one
	*/
	std::pair<bool, bool> isLegalSeqVert(int r, int c, int d, std::vector<Coordinate>& locations) const;

	/*
	* receives a board and location of the beggining of the potential ship
	* checks if the horizontal sequence is a legal ship
	* returns <a, b> where
	* a == true iff wrong shape of the ship
	* b == true iff there are adjancent ship with the given one
	*/
	std::pair<bool, bool> isLegalSeqHorz(int r, int c, int d, std::vector<Coordinate>& locations) const;

	/*
	* receives a board and location of the beggining of the potential ship
	* checks if the depth sequence is a legal ship
	* returns <a, b> where
	* a == true iff wrong shape of the ship
	* b == true iff there are adjancent ship with the given one
	*/
	std::pair<bool, bool> isLegalSeqDeep(int r, int c, int d, std::vector<Coordinate>& locations) const;

	/*
	* finds all valid ships on board
	* saves to <errorsInBoard? all errors found on board
	*/
	void findShips(bool errorsInBoard[numOfBoardErrors], std::vector<BattleShip>& ships);

	/*
	* divides all ships found by <findShips> to two vectors, each for a different player
	* returns the result from <printErr> with all the errors found till now
	* meaning, returns true iff board is legal
	*/
	bool isLegalBoard(std::string& errorStr);

public:
	/* empty constructor */
	OriginalBoard();

	/*destructor for OriginalBoard*/
	~OriginalBoard() override;

	// block copy and assignment
	OriginalBoard(const OriginalBoard&) = delete;
	OriginalBoard& operator=(const OriginalBoard&) = delete;

	char charAt(Coordinate c) const override; 

	/*
	* receives path for board
	* fills empty ships vectors with legal ships in player's boards
	* writes all errors found to <errors>
	* returns true iff board is legal
	*/
	bool OriginalBoard::createBoards(const std::string& path, std::string& errorsStr);

	/*
	recieves an attack move indexes
	and check in the game total board if some ship had been attacked
	return the proper AttackResult (Hit, Miss, Sink)
	and the ID of the player that it's ship eas Hitted\Sink
	(player ID 2 for Miss )
	*/
	std::pair<AttackResult, int> checkAttackResult(Coordinate& attackMove) const;

	/* copies ships from battleShipsA to shipsA and from battleShipsB to shipsB*/
	void getBattleShips(std::vector<BattleShip>& ships, int playerID);

	/*
	* set the symbol in the attckIndexes in the board to be newSymbol
	*/
	void setSymbol(Coordinate& attackIndexes, char newSymbol);
};