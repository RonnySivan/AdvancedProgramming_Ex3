#include "OriginalBoard.h"
#include <cctype>
#include <fstream>
#include <algorithm>

/*static functions*/

int OriginalBoard::convertTypeToInt(BattleShip& b)
{
	switch (std::tolower(b.getType()))
	{
	case 'b':
		return 0;
	case 'p':
		return 1;
	case 'm':
		return 2;
	default:
		return 3;
	}
}


/*member private functions*/

int OriginalBoard::typeToErr(char type) {
	if (type == 'B')
		return boardErrorWrongSizeForShipB;
	if (type == 'P')
		return boardErrorWrongSizeForShipP;
	if (type == 'M')
		return boardErrorWrongSizeForShipM;
	if (type == 'D')
		return boardErrorWrongSizeForShipD;
	if (type == 'b')
		return boardErrorWrongSizeForShipb;
	if (type == 'p')
		return boardErrorWrongSizeForShipp;
	if (type == 'm')
		return boardErrorWrongSizeForShipm;
	//if (type == 'd')
	return boardErrorWrongSizeForShipd;
}


bool OriginalBoard::printErr(bool errors[numOfBoardErrors], std::string& errorStr) {
	if (errors[boardErrorWrongSizeForShipB]) errorStr.append("Wrong size or shape for ship B for player A\n");
	if (errors[boardErrorWrongSizeForShipP]) errorStr.append("Wrong size or shape for ship P for player A\n");
	if (errors[boardErrorWrongSizeForShipM]) errorStr.append("Wrong size or shape for ship M for player A\n");
	if (errors[boardErrorWrongSizeForShipD]) errorStr.append("Wrong size or shape for ship D for player A\n");
	if (errors[boardErrorWrongSizeForShipb]) errorStr.append("Wrong size or shape for ship b for player B\n");
	if (errors[boardErrorWrongSizeForShipp]) errorStr.append("Wrong size or shape for ship p for player B\n");
	if (errors[boardErrorWrongSizeForShipm]) errorStr.append("Wrong size or shape for ship m for player B\n");
	if (errors[boardErrorWrongSizeForShipd]) errorStr.append("Wrong size or shape for ship d for player B\n");
	if (errors[boardErrorTooManyA]) errorStr.append("Too many ships for player A\n");
	if (errors[boardErrorTooFewA]) errorStr.append("Too few ships for player A\n");
	if (errors[boardErrorTooManyB]) errorStr.append("Too many ships for player B\n");
	if (errors[boardErrorTooFewB]) errorStr.append("Too few ships for player B\n");
	if (errors[boardErrorAdjShips]) errorStr.append("Adjacent Ships on Board\n");

	for (auto i = 0; i < numOfBoardErrors; i++) {
		if (errors[i]) return false;
	}
	return true;
}


bool OriginalBoard::getBoardSize(std::string& line)
{
	std::transform(line.begin(), line.end(), line.begin(), ::tolower);
	std::vector<std::string> splitCoord = Util::split(line, 'x');
	if (splitCoord.size() != 3)
			return false;
	
	_rows = std::stoi(splitCoord.at(0));
	_cols = std::stoi(splitCoord.at(1));
	_depth = std::stoi(splitCoord.at(2));

	return (_rows > 0 && _cols > 0 && _depth > 0);
}


void OriginalBoard::parseLine(std::string& line, int depth, int row)
{
	int col = 0;
	int l = static_cast<int>(line.length());
	while (col < _cols && col < l)
	{
		if (BattleShip::isLegalSymbol(line[col]))
		{
			_boardMap[Coordinate(row, col + 1, depth)] = line[col];
		}
		col++;
	}
}


bool OriginalBoard::parseBoards(const std::string& boardPath, std::string& errMsg) {
	std::string line;
	std::ifstream fin(boardPath);

	if (!fin.is_open()) {
		//std::cout << "Error: Cannot open *.sboard file in " << boardPath << std::endl;
		return false;
	}
	if (! getline(fin, line) || ! getBoardSize(line))
	{
		errMsg.append("Board size given is illegal\n");
		return false;
	}

	auto d = 1, r = 1;
	auto seenEmptyRow = false;
	while (d <= _depth && getline(fin, line))
	{
		if (seenEmptyRow)
		{
			parseLine(line, d, r);
			r++;
			if (r > _rows)
			{
				d++;
				r = 1;
				seenEmptyRow = false;
			}
		}
		else
		{
			seenEmptyRow = line.compare("\n") || line.compare("\r");
		}
	}
	return true;
}


void OriginalBoard::isEmptyNeighbors(int r, int c, int d, bool checkVert, bool checkHorz, bool checkDepth, std::pair<bool, bool> isIllegal) const
{
	char cur = charAt(Coordinate(r, c, d));
	for (auto i = 0; i < 2; i++)
	{
		bool check = false;
		char neighbor = ' ';
		switch (i)
		{
		case 0:
			if ((r > 0 && checkVert) || (c > 0 && checkHorz) || (d > 0 && checkDepth))
			{
				neighbor = charAt(Coordinate(r - 1 * checkVert, c - 1 * checkHorz, d - 1 * checkDepth));
				check = true;
			}
		default:
			if ((r < _rows && checkVert) || (c < _cols && checkHorz) || (d < _depth && checkDepth))
			{
				neighbor = charAt(Coordinate(r + 1 * checkVert, c + 1 * checkHorz, d + 1 * checkDepth));
				check = true;
			}
		}
		if (check)
		{
			isIllegal.second = isIllegal.second || neighbor == cur;
			isIllegal.first = isIllegal.first || (neighbor != cur && neighbor != ' ');
		}
	}
}


std::pair<bool, bool> OriginalBoard::isLegalSeqHorz(int r, int c, int d, std::vector<Coordinate>& locations) const
{
	auto len = 0;
	char type = charAt(Coordinate(r, c, d));
	std::pair<bool, bool> illegal(false, false); //(adjucent ships, illegal length of type)
	if (c > 1)
	{
		illegal.first = charAt(Coordinate(r, c - 1, d)) != ' ';
	}
	for (auto k = c; k <= _cols; k++)
	{
		char cur = charAt(Coordinate(r , k, d));
		if (cur != type)
		{
			illegal.first = illegal.first || (cur != ' ');
			break;
		}
		isEmptyNeighbors(r, k, d, true, false, false, illegal);
		isEmptyNeighbors(r, k, d, false, false, true, illegal);
		locations.push_back(Coordinate(r, k, d));
		len++;
	}
	if (!BattleShip::isLegalShip(type, len))
		illegal.second = true;

	return illegal;
}


std::pair<bool, bool> OriginalBoard::isLegalSeqVert(int r, int c, int d, std::vector<Coordinate>& locations) const
{
	auto len = 0;
	char type = charAt(Coordinate(r, c, d));
	std::pair<bool, bool> illegal(false, false); //(adjucent ships, illegal length of type)
	if (r > 1)
	{
		illegal.first = charAt(Coordinate(r - 1, c, d)) != ' ';
	}
	for (auto k = r; k <= _rows; k++)
	{
		char cur = charAt(Coordinate(k, c, d));
		if (cur != type)
		{
			illegal.first = illegal.first || (cur != ' ');
			break;
		}
		isEmptyNeighbors(k, c, d, false, true, false, illegal);
		isEmptyNeighbors(k, c, d, false, false, true, illegal);
		locations.push_back(Coordinate(k, c, d));
		len++;
	}
	if (!BattleShip::isLegalShip(type, len))
		illegal.second = true;

	return illegal;
}


std::pair<bool, bool> OriginalBoard::isLegalSeqDeep(int r, int c, int d, std::vector<Coordinate>& locations) const
{
	auto len = 0;
	char type = charAt(Coordinate(r, c, d));
	std::pair<bool, bool> illegal(false, false); //(adjucent ships, illegal length of type)
	if (d > 1)
	{
		illegal.first = charAt(Coordinate(r, c, d - 1)) != ' ';
	}
	for (auto k = d; k <= _depth; k++)
	{
		char cur = charAt(Coordinate(r, c, k));
		if (cur != type)
		{
			illegal.first = illegal.first || (cur != ' ');
			break;
		}
		isEmptyNeighbors(r, c, k, true, false, false, illegal);
		isEmptyNeighbors(r, c, k, false, true, false, illegal);
		locations.push_back(Coordinate(r, c, k));
		len++;
	}
	if (!BattleShip::isLegalShip(type, len))
		illegal.second = true;

	return illegal;
}


void OriginalBoard::findShips(bool errorsInBoard[numOfBoardErrors], std::vector<BattleShip>& ships) // TODO: is function const?
{
	for (auto k = 1; k <= _depth; k++)
	{
		for (auto i = 1; i <= _rows; i++)
		{
			for (auto j = 1; j <= _cols; j++)
			{
				char type = charAt(Coordinate(i, j, k));
				if (type == ' ') continue;
				/*enter <if> only if cell is not empty and
				*is not part of a sequence that was already checked(from above or left or within)*/
				if (type != ' ' && 
					(i == 1 || charAt(Coordinate(i - 1, j, k)) != type) &&
					(j == 1 || charAt(Coordinate(i, j - 1, k)) != type) &&
					(k == 1 || charAt(Coordinate(i, j, k - 1)) != type))
				{
					std::vector<Coordinate> locV, locH, locD;
					std::pair<bool, bool> vert = isLegalSeqVert(i, j, k, locV);
					std::pair<bool, bool> horz = isLegalSeqHorz(i, j, k, locH);
					std::pair<bool, bool> deep = isLegalSeqDeep(i, j, k, locD);
					if (vert.first || horz.first || deep.first)
					{
						//adjancent ships
						errorsInBoard[boardErrorAdjShips] = true;
					}
					if (!vert.second)
					{
						//legal vert ship!
						ships.push_back(BattleShip(type, locV));
						continue;
					}
					if (!horz.second)
					{
						//legal horz ship!
						ships.push_back(BattleShip(type, locH));
						continue;
					}
					if (!deep.second)
					{
						//legal deep ship!
						ships.push_back(BattleShip(type, locD));
						continue;
					}
					errorsInBoard[OriginalBoard::typeToErr(type)] = true;
				}
			}
		}
	}
}


void OriginalBoard::isSameShips(std::string& errorsStr)
{
	int countAShipsTypes[4] = { 0 };
	int countBShipsTypes[4] = { 0 };
	for (BattleShip& b : _battleShipsA)
	{
		countAShipsTypes[convertTypeToInt(b)]++;
	}
	for (BattleShip& b : _battleShipsB)
	{
		countBShipsTypes[convertTypeToInt(b)]++;
	}
	for (auto i = 0; i < 4; i++)
	{
		if (countAShipsTypes[i] != countBShipsTypes[i])
		{
			errorsStr.append("Players do not have the same ships\n");
			return;
		}
	}
}



bool OriginalBoard::isLegalBoard(std::string& errorsMsg) {
	bool errors[numOfBoardErrors] = { false };
	std::vector<BattleShip> ships;
	findShips(errors, ships);
	for (std::vector<BattleShip>::iterator itr = ships.begin(); itr != ships.end(); ++itr)
	{
		if ((*itr).getType() < 'a')
			_battleShipsA.push_back(*itr);
		else
			_battleShipsB.push_back(*itr);
	}

	if (_battleShipsA.size() < numOfPlayerShips) errors[boardErrorTooFewA] = true;
	if (_battleShipsA.size() > numOfPlayerShips) errors[boardErrorTooManyA] = true;
	if (_battleShipsB.size() < numOfPlayerShips) errors[boardErrorTooFewB] = true;
	if (_battleShipsB.size() > numOfPlayerShips) errors[boardErrorTooManyB] = true;
	return printErr(errors, errorsMsg);
}


/*public member functions*/

bool OriginalBoard::createBoards(const std::string& path, std::string& errorsStr) {

	if (! parseBoards(path, errorsStr))
		return false;

	if (isLegalBoard(errorsStr))
	{
		isSameShips(errorsStr);
		return true;
	}
	return false;
}


void OriginalBoard::getBattleShips(std::vector<BattleShip>& ships, int playerID) const
{
	std::vector<BattleShip> fromShips = (playerID == 0) ? _battleShipsA : _battleShipsB;
	for (std::vector<BattleShip>::iterator itr = fromShips.begin(); itr != fromShips.end(); ++itr)
		ships.push_back(*itr);
}


char OriginalBoard::charAt(Coordinate c) const
{
	if (c.row > 0 && c.row <= _rows && c.col > 0 && c.col <= _cols && c.depth > 0 && c.depth <= _depth)
	{
		std::map<Coordinate, char>::const_iterator cur = _boardMap.find(c);
		if (cur != _boardMap.end())
		{
			return cur->second;
		}
	}
	return ' ';
}


