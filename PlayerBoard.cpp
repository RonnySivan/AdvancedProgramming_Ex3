#include "PlayerBoard.h"

PlayerBoard::PlayerBoard(int playerID, std::shared_ptr<OriginalBoard> originalBoard) :
	_playerID(playerID),
	_originalBoard(originalBoard)
{
	_rows = (_originalBoard.get())->rows();
	_cols = (_originalBoard.get())->cols();
	_depth = (_originalBoard.get())->depth();
}


char PlayerBoard::charAt(Coordinate c) const {
	char value = _originalBoard->charAt(c);
	if (_playerID == 0)
	{
		return (value < 'a') ? value : ' ';
	}
	return (value < 'a') ? ' ' : value;
}