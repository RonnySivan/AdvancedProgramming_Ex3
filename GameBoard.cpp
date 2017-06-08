#include "GameBoard.h"

GameBoard::GameBoard(std::shared_ptr<OriginalBoard> originalBoard) : _originalBoard(originalBoard)
{
	_rows = (_originalBoard.get())->rows();
	_cols = (_originalBoard.get())->cols();
	_depth = (_originalBoard.get())->depth();
}

char GameBoard::charAt(Coordinate c) const {
	std::map<Coordinate, char>::const_iterator cur = _updatedBoard.find(c);
	if (cur != _updatedBoard.end())
	{
		return cur->second;
	}
	return _originalBoard->charAt(c);
}

void GameBoard::setSymbol(Coordinate& attackIndexes, char newSymbol)
{
	if (attackIndexes.row > 0 && attackIndexes.row <= _originalBoard->rows() &&
		attackIndexes.col> 0 && attackIndexes.col <= _originalBoard->cols() &&
		attackIndexes.depth> 0 && attackIndexes.depth <= _originalBoard->depth())
	{
		_updatedBoard[attackIndexes] = newSymbol;

	}
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