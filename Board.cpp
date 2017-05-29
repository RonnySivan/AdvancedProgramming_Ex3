#include "Board.h"

Board::Board()
{
}

Board::~Board()
{
}

int Board::typeToErr(char type) {
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

bool Board::printErr(bool errors[numOfBoardErrors]) {
	if (errors[boardErrorWrongSizeForShipB]) std::cout << "Wrong size or shape for ship B for player A" << std::endl;
	if (errors[boardErrorWrongSizeForShipP]) std::cout << "Wrong size or shape for ship P for player A" << std::endl;
	if (errors[boardErrorWrongSizeForShipM]) std::cout << "Wrong size or shape for ship M for player A" << std::endl;
	if (errors[boardErrorWrongSizeForShipD]) std::cout << "Wrong size or shape for ship D for player A" << std::endl;
	if (errors[boardErrorWrongSizeForShipb]) std::cout << "Wrong size or shape for ship b for player B" << std::endl;
	if (errors[boardErrorWrongSizeForShipp]) std::cout << "Wrong size or shape for ship p for player B" << std::endl;
	if (errors[boardErrorWrongSizeForShipm]) std::cout << "Wrong size or shape for ship m for player B" << std::endl;
	if (errors[boardErrorWrongSizeForShipd]) std::cout << "Wrong size or shape for ship d for player B" << std::endl;
	if (errors[boardErrorTooManyA]) std::cout << "Too many ships for player A" << std::endl;
	if (errors[boardErrorTooFewA]) std::cout << "Too few ships for player A" << std::endl;
	if (errors[boardErrorTooManyB]) std::cout << "Too many ships for player B" << std::endl;
	if (errors[boardErrorTooFewB]) std::cout << "Too few ships for player B" << std::endl;
	if (errors[boardErrorAdjShips]) std::cout << "Adjacent Ships on Board" << std::endl;

	for (auto i = 0; i < numOfBoardErrors; i++) {
		if (errors[i]) return false;
	}
	return true;
}

bool Board::getBoardSize(std::string& line)
{
	std::vector<std::string> splitCoord = Util::split(line, 'x');
	if (splitCoord.size() != 3)
			return false;
	
	_rows = std::stoi(splitCoord.at(0));
	_cols = std::stoi(splitCoord.at(1));
	_depth = std::stoi(splitCoord.at(2));

	return (_rows > 0 && _cols > 0 && _depth > 0);
}

void Board::parseLine(std::string& line, int depth, int row)
{
	auto col = 0;
	while (col < _cols && col < line.length())
	{
		_boardMap[Coordinate(row, col, depth)] = (BattleShip::isLegalSymbol(line[col])) ? line[col] : ' ';
	}
	while (col < _cols)
	{
		_boardMap[Coordinate(row, col, depth)] = ' ';
	}
}

void Board::fillEmptyCells(int r, int d)
{
	while (d < _depth)
	{
		while (r < _rows)
		{
			int c = 0;
			while (c < _cols)
			{
				_boardMap[Coordinate(r, c, d)] = ' ';
				c++;
			}
			r++;
		}
		r = 0;
		d++;
	}
}

bool Board::parseBoards(const std::string& boardPath) {
	std::string line;
	std::ifstream fin(boardPath);

	if (!fin.is_open()) {
		//std::cout << "Error: Cannot open *.sboard file in " << boardPath << std::endl;
		return false;
	}
	if (getline(fin, line))
	{
		if (! getBoardSize(line))
		{
			//Util::printToLogger("Board size given is illegal");
			return false;
		}
	}
	else
	{
		//Util::printToLogger("Board size given is illegal");
		return false;
	}

	int d = 0, r = 0;
	bool seenEmptyRow = false;
	while (d < _depth && getline(fin, line))
	{
		if (seenEmptyRow)
		{
			parseLine(line, d, r);
			r++;
			if (r == _rows)
			{
				d++;
				r = 0;
				seenEmptyRow = false;
			}
		}
		else
		{
			seenEmptyRow = line.compare("\n") || line.compare("\r");
		}
	}
	fillEmptyCells(r, d);
	fin.close;
	return true;
}

void Board::isEmptyNeighbors(int r, int c, int d, bool checkVert, bool checkHorz, bool checkDepth, std::pair<bool, bool> isIllegal)
{
	char cur = _boardMap.at(Coordinate(r, c, d));
	for (int i = 0; i < 2; i++)
	{
		bool check = false;
		char neighbor = ' ';
		switch (i)
		{
		case 0:
			if ((r > 0 && checkVert) || (c > 0 && checkHorz) || (d > 0 && checkDepth))
			{
				neighbor = _boardMap.at(Coordinate(r - 1 * checkVert, c - checkHorz, d - checkDepth));
				check = true;
			}
		default:
			if ((r < _rows && checkVert) || (c < _cols && checkHorz) || (d < _depth && checkDepth))
			{
				neighbor = _boardMap.at(Coordinate(r + 1 * checkVert, c + checkHorz, d + checkDepth));
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

std::pair<bool, bool> Board::isLegalSeqHorz(int r, int c, int d, std::vector<Coordinate>& locations)
{
	auto len = 0;
	char type = _boardMap.at(Coordinate(r, c, d));
	std::pair<bool, bool> illegal(false, false); //(adjucent ships, legal length of type)
	for (auto k = c; k < _cols; k++)
	{
		char cur = _boardMap.at(Coordinate(r , k, d));
		if (cur == ' ') break;
		if (cur != type) {
			illegal.second = true;
			break;
		}
		isEmptyNeighbors(r, k, d, true, false, false, illegal);
		isEmptyNeighbors(r, k, d, false, false, true, illegal);
		locations.push_back(Coordinate(r, k, d));
		len++;
	}
	if (!BattleShip::isLegalShip(type, len))
		illegal.first = true;

	return illegal;
}

std::pair<bool, bool> Board::isLegalSeqVert(int r, int c, int d, std::vector<Coordinate>& locations)
{
	auto len = 0;
	char type = _boardMap.at(Coordinate(r, c, d));
	std::pair<bool, bool> illegal(false, false); //(adjucent ships, legal length of type)
	for (auto k = r; k < _rows; k++)
	{
		char cur = _boardMap.at(Coordinate(k, c, d));
		if (cur == ' ') break;
		if (cur != type) {
			illegal.second = true;
			break;
		}
		isEmptyNeighbors(k, c, d, false, true, false, illegal);
		isEmptyNeighbors(k, c, d, false, false, true, illegal);
		locations.push_back(Coordinate(k, c, d));
		len++;
	}
	if (!BattleShip::isLegalShip(type, len))
		illegal.first = true;

	return illegal;
}

std::pair<bool, bool> Board::isLegalSeqDeep(int r, int c, int d, std::vector<Coordinate>& locations)
{
	auto len = 0;
	char type = _boardMap.at(Coordinate(r, c, d));
	std::pair<bool, bool> illegal(false, false); //(adjucent ships, legal length of type)
	for (auto k = d; k < _depth; k++)
	{
		char cur = _boardMap.at(Coordinate(r, c, k));
		if (cur == ' ') break;
		if (cur != type) {
			illegal.second = true;
			break;
		}
		isEmptyNeighbors(r, c, k, true, false, false, illegal);
		isEmptyNeighbors(r, c, k, false, true, false, illegal);
		locations.push_back(Coordinate(r, c, k));
		len++;
	}
	if (!BattleShip::isLegalShip(type, len))
		illegal.first = true;

	return illegal;
}

void Board::findShips(bool errorsInBoard[numOfBoardErrors], std::vector<BattleShip>& ships)
{
	for (auto k = 0; k < _depth; k++)
	{
		for (auto i = 0; i < _rows; i++)
		{
			for (auto j = 0; j < _cols; j++)
			{
				char type = _boardMap.at(Coordinate(i, j, k));
				/*enter <if> only if cell is not empty and
				*is not part of a sequence that was already checked(from above or left or within)*/
				if (type != ' ' && 
					(i == 0 || _boardMap.at(Coordinate(i - 1, j, k)) != type) &&
					(j == 0 || _boardMap.at(Coordinate(i, j - 1, k)) != type) &&
					(k == 0 || _boardMap.at(Coordinate(i, j, k - 1)) != type))
				{
					std::vector<Coordinate> locV, locH, locD;
					std::pair<bool, bool> vert = isLegalSeqVert(i, j, k, locV);
					std::pair<bool, bool> horz = isLegalSeqHorz(i, j, k, locH);
					std::pair<bool, bool> deep = isLegalSeqDeep(i, j, k, locD);
					if (vert.second || horz.second || deep.second)
					{
						//adjancent ships
						errorsInBoard[boardErrorAdjShips] = true;
					}
					if (!vert.first)
					{
						//legal vert ship!
						ships.push_back(BattleShip(type, locV));
						continue;
					}
					if (!horz.first)
					{
						//legal vert ship!
						ships.push_back(BattleShip(type, locH));
						continue;
					}
					if (!deep.first)
					{
						//legal vert ship!
						ships.push_back(BattleShip(type, locD));
						continue;
					}
					errorsInBoard[Board::typeToErr(type)] = true;
				}
			}
		}
	}
}

bool Board::isLegalBoard() {
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
	return printErr(errors);
}

bool Board::createBoards(const std::string& path) {

	if (! parseBoards(path))
		return false;

	return isLegalBoard();
}

std::pair<AttackResult, int> Board::checkAttackResult(Coordinate& attackMove) {

	char symbol = _boardMap.at(attackMove);

	if (symbol == ' ' || symbol == 'X' || symbol == 'x')
		return std::make_pair(AttackResult::Miss, 2); // attacked the sea OR a sinked battleship. 

	if (symbol == 'h' || symbol == 'H')
		return std::make_pair(AttackResult::Hit, 2);; // attack allready hit battleship (did not sink yet)

	if (symbol == 'B' || symbol == 'P' || symbol == 'M' || symbol == 'D') {
		// B get a HIT
		return std::make_pair(AttackResult::Hit, 0);
	}
	if (symbol == 'b' || symbol == 'p' || symbol == 'm' || symbol == 'd') {
		// A get a Hit
		return std::make_pair(AttackResult::Hit, 1);
	}
	//shouldn't reach this line
	return std::make_pair(AttackResult::Miss, 2);
}

void Board::getBattleShips(std::vector<BattleShip>& ships, int playerID)
{
	std::vector<BattleShip> fromShips = (playerID == 0) ? _battleShipsA : _battleShipsB;
	for (std::vector<BattleShip>::iterator itr = fromShips.begin(); itr != fromShips.end(); ++itr)
		ships.push_back(*itr);
}

void Board::setSymbol(Coordinate& attackIndexes, char newSymbol) {
	_boardMap[attackIndexes] = newSymbol;
}

std::string to_string(Coordinate c) {
	return "(" + std::to_string(c.col) + ", " + std::to_string(c.row) + ", " + std::to_string(c.depth) + ")";
}

std::ostream& operator<<(std::ostream& out, const Coordinate& c) {
	return out << to_string(c);
}

// required for unordered_map
bool operator==(const Coordinate& c1, const Coordinate& c2) {
	return c1.col == c2.col && c1.row == c2.row && c1.depth == c2.depth;
}

// required for map
bool operator<(const Coordinate& c1, const Coordinate& c2) {
	if (c1.col == c2.col) {
		if (c1.row == c2.row) {
			return c1.depth < c2.depth;
		}
		return c1.row < c2.row;
	}
	return c1.col < c2.col;
}

char Board::charAt(Coordinate c) const
{
	if (c.row > 0 && c.row < _rows && c.col > 0 && c.col < _cols && c.depth > 0 && c.depth < _depth)
	{
		return _boardMap.at(c);
	}
	return '\0';
}


