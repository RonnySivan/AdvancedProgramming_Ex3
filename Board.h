#pragma once

#include "IBattleshipGameAlgo.h"

#include "BattleShip.h"
#include "Util.h"
#include <vector>
#include <map>

#define boardSize 10

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

class Board : public BoardData
{
private:
	std::map<Coordinate, char> _boardMap;
	std::vector<BattleShip> _battleShipsA;
	std::vector<BattleShip> _battleShipsB;
	friend class GameManager;

	/*returns errno of ship of type <type>*/
	static int typeToErr(char type);

	/* prints all errors whose errno matches the "true" elements in <errors>*/
	static bool printErr(bool errors[numOfBoardErrors]);

	/*
	*recieves path for file containing board
	*updates the general board and the boards for both players
	*/
	bool parseBoards(const std::string& boardPath,
		char(*boardA)[boardSize][boardSize],
		char(*boardB)[boardSize][boardSize]);

	/*
	* receives a board and location of the beggining of the potential ship
	* checks if the vertical sequence is a legal ship
	* returns <a, b> where
	* a == true iff wrong shape of the ship
	* b == true iff there are adjancent ship with the given one
	*/
	std::pair<bool, bool> isLegalSeqVert(int r, int c, std::vector<std::pair<int, int>>& locations);

	/*
	* receives a board and location of the beggining of the potential ship
	* checks if the horizontal sequence is a legal ship
	* returns <a, b> where
	* a == true iff wrong shape of the ship
	* b == true iff there are adjancent ship with the given one
	*/
	std::pair<bool, bool> isLegalSeqHorz(int r, int c, std::vector<std::pair<int, int>>& locations);

	/*
	* finds all valid ships on board
	*
	* saves to <errorsInBoard? all errors found on board
	*
	*/
	void findShips(bool errorsInBoard[numOfBoardErrors], std::vector<BattleShip>& ships);

	/*
	* divides all ships found by <findShips> to two vectors, each for a different player
	* returns the result from <printErr> with all the errors found till now
	* meaning, returns true iff board is legal
	*/
	bool isLegalBoard();

public:
	virtual char charAt(Coordinate c); //returns only selected players' chars

protected:
	int _rows = 0; // make sure you set all protected members in the derived class.
	int _cols = 0; // make sure you set all protected members in the derived class.
	int _depth = 0; // make sure you set all protected members in the derived class.
};