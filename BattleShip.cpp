#include "BattleShip.h"


/*
* Ctor for BattleShip
* \param letter - name of the ship
* \param locations - vector of the locations of the ship on the board
*/
BattleShip::BattleShip(char letter, std::vector<Coordinate>& locations)
	: m_letter(letter)
	, m_locations(locations)
	, m_alive(true)
{
	m_currentLength = locations.size();
	switch (m_letter)
	{
	case ('b'): m_score = scoreB;
		break;
	case ('B'): m_score = scoreB;
		break;
	case ('p'): m_score = scoreP;
		break;
	case ('P'): m_score = scoreP;
		break;
	case ('m'): m_score = scoreM;
		break;
	case ('M'): m_score = scoreM;
		break;
	case ('d'): m_score = scoreD;
		break;
	case ('D'): m_score = scoreD;
	}
}


BattleShip::~BattleShip() {}

AttackResult BattleShip::shipAttackResult(Coordinate attackMove)
{

	for (auto i = 0; i < m_locations.size(); i++)
	{
		if ((m_locations[i].row == attackMove.row) && 
			(m_locations[i].col == attackMove.col) &&
			(m_locations[i].depth == attackMove.depth))
		{
			m_currentLength--;
			if (m_currentLength == 0)
			{
				m_alive = false;
				return AttackResult::Sink;
			}
			else
			{
				return AttackResult::Hit;
			}
		}
	}
	return AttackResult::Miss;
}

bool BattleShip::isLegalShip(char type, int size) {
	if (type == 'B' || type == 'b')
		return (size == sizeB);
	if (type == 'P' || type == 'p')
		return (size == sizeP);
	if (type == 'M' || type == 'm')
		return (size == sizeM);
	/*if (type == 'D' || type == 'd')*/
	return (size == sizeD);
}

bool BattleShip::isLegalSymbol(char type) {
	return (type == 'B' || type == 'b' || type == 'P' || type == 'p' ||
		type == 'M' || type == 'm' || type == 'D' || type == 'd');
}
