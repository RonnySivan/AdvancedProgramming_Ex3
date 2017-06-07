﻿#include "PlayerBoard.h"

PlayerBoard::PlayerBoard(int playerID, OriginalBoard& originalBoard) :
	_playerID(playerID),
	_originalBoard(&originalBoard) {}


PlayerBoard::~PlayerBoard() {
	_originalBoard = nullptr;
}

char PlayerBoard::charAt(Coordinate c) const {
	char value = _originalBoard->charAt(c);
	if (_playerID == 0)
	{
		return (value < 'a') ? value : ' ';
	}
	return (value < 'a') ? ' ' : value;
}