#pragma once

#include "OriginalBoard.h"
#include <memory>

class GameBoard : public BoardData
{
	std::shared_ptr<OriginalBoard> _originalBoard;
	std::map<Coordinate, char> _updatedBoard;


public:
	GameBoard(std::shared_ptr<OriginalBoard> originalBoard);
	~GameBoard() = default;
	// block copy and assignment
	GameBoard(const GameBoard&) = delete;
	GameBoard& operator=(const GameBoard&) = delete;


	/*returns only selected players' char at coordinate c*/
	char charAt(Coordinate c) const override;


	/*
	* set the symbol in the attckIndexes in the board to be newSymbol
	*/
	void setSymbol(Coordinate attackIndexes, char newSymbol);


	/*
	recieves an attack move indexes
	and check in the game total board if some ship had been attacked
	return the proper AttackResult (Hit, Miss, Sink)
	and the ID of the player that it's ship was Hitted\Sink
	(player ID 2 for Miss )
	*/
	std::pair<AttackResult, int> checkAttackResult(Coordinate attackMove) const;

	/* copies ships from battleShipsA to shipsA and from battleShipsB to shipsB*/
	void getBattleShips(std::vector<BattleShip>& ships, int playerID) const;
};
