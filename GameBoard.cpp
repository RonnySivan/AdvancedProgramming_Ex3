#include "GameBoard.h"

GameBoard::GameBoard(OriginalBoard& originalBoard) : _originalBoard(&originalBoard) {}

char GameBoard::charAt(Coordinate c) const {
	if (c.row > 0 && c.row < _originalBoard->rows() &&
		c.col> 0 && c.col < _originalBoard->cols() &&
		c.depth> 0 && c.depth < _originalBoard->depth())
	{
		std::map<Coordinate, char>::const_iterator cur = _updatedBoard.find(c);
		if (cur != _updatedBoard.end())
		{
			return cur->second;
		}
	}
	return _originalBoard->charAt(c);
}

void GameBoard::setSymbol(Coordinate& attackIndexes, char newSymbol)
{
	_updatedBoard[attackIndexes] = newSymbol;
}

std::pair<AttackResult, int> GameBoard::checkAttackResult(Coordinate& attackMove) const
{
	char symbol = charAt(attackMove);

	if (symbol == ' ' || symbol == 'X' || symbol == 'x')
		return std::make_pair(AttackResult::Miss, 2); // attacked the sea OR a sinked battleship

	if (symbol == 'h' || symbol == 'H')
		return std::make_pair(AttackResult::Hit, 2); // attacked ship who hasn't sank yet, in a place attacked already

	if (symbol == 'B' || symbol == 'P' || symbol == 'M' || symbol == 'D')
		return std::make_pair(AttackResult::Hit, 1); // B got hit

	if (symbol == 'b' || symbol == 'p' || symbol == 'm' || symbol == 'd')
		return std::make_pair(AttackResult::Hit, 1); //  A got hit

	//shouldn't reack this line
	return std::make_pair(AttackResult::Miss, 2);
}