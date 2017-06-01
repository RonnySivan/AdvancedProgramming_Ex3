#pragma once

#include "OriginalBoard.h"

class PlayerBoard : public BoardData
{
	int _playerID;
	OriginalBoard* _originalBoard;

public:
	PlayerBoard(int playerID, OriginalBoard* originalBoard);
	~PlayerBoard() = default;
	// block copy and assignment
	PlayerBoard(const PlayerBoard&) = delete;
	PlayerBoard& operator=(const PlayerBoard&) = delete;

	/*returns only selected players' char at coordinate c*/
	char charAt(Coordinate c) const override;
};