#pragma once

#include "IBattleshipGameAlgo.h"
#include <vector>

#define sizeB 1
#define sizeP 2
#define sizeM 3
#define sizeD 4
#define scoreB 2
#define scoreP 3
#define scoreM 7
#define scoreD 8

class BattleShip
{
public:
	BattleShip(char letter, std::vector<Coordinate>& locations);
	~BattleShip();

	// block assignment c'tor but not copy c'tor
	//BattleShip(const BattleShip&) = delete;
	BattleShip& operator=(const BattleShip&) = delete;

	// class functions
	std::vector<Coordinate> getLocations() const { return m_locations; }
	int getScore() const { return m_score; }
	char getType() const { return m_letter; }
	bool isAlive() const { return m_alive; }

	/*
	* returns result of attack in attackMoves place on this ship
	*/
	AttackResult shipAttackResult(Coordinate attackMove);

	/* returns true iff ship of type <type> has length <size> */
	static bool isLegalShip(char type, int size);

private:
	char m_letter;
	int m_score;
	std::vector<Coordinate> m_locations;
	size_t m_currentLength;
	bool m_alive; // set to false when m_currentLength == 0
};