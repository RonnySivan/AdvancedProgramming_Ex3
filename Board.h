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

	/*returns errno of ship of type <type>*/
	static int typeToErr(char type);

	/* prints all errors whose errno matches the "true" elements in <errors>*/
	static bool printErr(bool errors[numOfBoardErrors]);

	/*get board size from file*/
	bool getBoardSize(std::string& line);

	/*update all map elements corresponding to row in depth*/
	void parseLine(std::string& line, int depth, int row);
	
	/* fill all cells starting from depth d row r with empty space*/
	void fillEmptyCells(int r, int d);

	/*
	*recieves path for file containing board
	*updates the general board and the boards for both players
	*/
	bool parseBoards(const std::string& boardPath);

	void isEmptyNeighbors(int r, int c, int d, bool checkVert, bool checkHorz, bool checkDepth, std::pair<bool, bool> isIllegal);

	/*
	* receives a board and location of the beggining of the potential ship
	* checks if the vertical sequence is a legal ship
	* returns <a, b> where
	* a == true iff wrong shape of the ship
	* b == true iff there are adjancent ship with the given one
	*/
	std::pair<bool, bool> isLegalSeqVert(int r, int c, int d, std::vector<Coordinate>& locations);

	/*
	* receives a board and location of the beggining of the potential ship
	* checks if the horizontal sequence is a legal ship
	* returns <a, b> where
	* a == true iff wrong shape of the ship
	* b == true iff there are adjancent ship with the given one
	*/
	std::pair<bool, bool> isLegalSeqHorz(int r, int c, int d, std::vector<Coordinate>& locations);

	/*
	* receives a board and location of the beggining of the potential ship
	* checks if the depth sequence is a legal ship
	* returns <a, b> where
	* a == true iff wrong shape of the ship
	* b == true iff there are adjancent ship with the given one
	*/
	std::pair<bool, bool> isLegalSeqDeep(int r, int c, int d, std::vector<Coordinate>& locations);

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

	/*functions for mmapping Coordinate*/

	std::string to_string(Coordinate c);

	// this is one way to define hash function for a type
	// see: http://en.cppreference.com/w/cpp/utility/hash
	struct MyHash {
		std::size_t operator()(const Coordinate& c) const {
			return c.row * 7 + c.col * 5 + c.depth * 11;
		}
	};

	std::ostream& operator<<(std::ostream& out, const Coordinate& c);

	// required for unordered_map
	bool operator==(const Coordinate& c1, const Coordinate& c2);

	// required for map
	bool operator<(const Coordinate& c1, const Coordinate& c2);

public:
	/* empty constructor */
	Board();

	/*destructor for Board*/
	~Board() override;

	// block copy and assignment
	Board(const Board&) = delete;
	Board& operator=(const Board&) = delete;

	virtual char charAt(Coordinate c) const; //returns only selected players' chars

	/*
	* receives path for board
	* fills empty ships vectors with legal ships in player's boards
	* returns true iff board is legal
	*/
	bool Board::createBoards(const std::string& path);

	/*
	recieves an attack move indexes
	and check in the game total board if some ship had been attacked
	return the proper AttackResult (Hit, Miss, Sink)
	and the ID of the player that it's ship eas Hitted\Sink
	(player ID 2 for Miss )
	*/
	std::pair<AttackResult, int> checkAttackResult(Coordinate& attackMove);

	/* copies ships from battleShipsA to shipsA and from battleShipsB to shipsB*/
	void getBattleShips(std::vector<BattleShip>& ships, int playerID);

	void copyPlayerBoard(Board& board, int playerID);

	/*
	* set the symbol in the attckIndexes in the board to be newSymbol
	*/
	void setSymbol(Coordinate& attackIndexes, char newSymbol);
};