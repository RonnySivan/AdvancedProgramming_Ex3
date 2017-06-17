#pragma once
#include <vector>
#include <set>
#include <random>
#include "IBattleshipGameAlgo.h"
#include "Battleship.h"

class SmartPlayer : public IBattleshipGameAlgo
{
	// Nested enum class for the algorithm possible states
	enum class State {
		Search, FirstUp, FirstDown, FirstRight, FirstLeft, FirstFwd, FirstBwd, Up1, Down1, Right1, Left1, Fwd1, Bwd1, Down2, Left2, Bwd2
	};

	// class members
	int m_id;
	State m_state; // algorithm state
	std::set< Coordinate > m_potential_attacks; // potential attack moves set
	Coordinate m_last_good_attack; // last attack that hit oponent's ship
	Coordinate m_cur_first_found; // first location found of an oponent's ship, to be currently attacked
	std::set< Coordinate > m_first_found_set; // first location found of an oponent's ship (potentially), to be attacked later
	std::mt19937 m_generator; //for random search
	BattleShip m_oponent_ship;

	// class private functions
	/**
	 * Push all potential attack moves to m_potential_attacks set. This function MUST be called after calling m_player.commonSetBoard()
	 */
	void init_potential_attacks(const BoardData& board);

	/**
	 * \brief finds all player's on board and removes illegal ships locations from m_potential_attacks set accordingly.
	 */
	void findShips(const BoardData& board);

	/*
	* Utility functions called from findShis, to remove illegal ships locations from m_potential_attacks set accordingly.
	*/
	void remove_illegal_horz(int r, int c, int d, std::vector<Coordinate>& locations, const BoardData& board);
	void remove_illegal_vert(int r, int c, int d, std::vector<Coordinate>& locations, const BoardData& board);
	void remove_illegal_deep(int r, int c, int d, std::vector<Coordinate>& locations, const BoardData& board);

	/**
	 * \brief calculates a random attack move
	 * \return random attack move from m_potential_attacks set
	 */
	Coordinate attackRand();
	
	/**
	 * \brief calculates next attack move according to m_state
	 * \return calculated attack move from m_potential_attacks set
	 */
	Coordinate attackState();

	/**
	 * \brief checks if attack's neighbors were already self-hit by oponent
	 * \param attack next attack move being examined in attackState()
	 */
	void check_neighbors(const Coordinate& attack);

	/**
	 * \brief checks if attack is in m_potential_attacks set
	 * \param attack next attack move being examined in attackState()
	 * \return true if attack is in m_potential_attacks set, else false
	 */
	bool check_attack(const Coordinate& attack);

	/**
	 * \brief calculates next m_state
	 * \param playerID id of last attacking player
	 * \param move last attack Coordinate
	 * \param last_attack_result last attack result
	 */
	void calc_state(int playerID, Coordinate move, const AttackResult& last_attack_result);
	
	/**
	 * \brief calculates m_state and updates m_potential_attacks set after oponent's attack
	 * \param playerID oponent's id
	 * \param move last attack Coordinate
	 * \param last_attack_result oponent's attack result
	 */
	void oponent_attack(int playerID, Coordinate move, const AttackResult& last_attack_result);

	/**
	 * \brief calculates next m_state if it is currently FirstUp, FirstDown, FirstRight or FirstLeft
	 * \param move last attack Coordinate
	 * \param last_attack_result last attack result
	 */
	void check_first(Coordinate move, const AttackResult& last_attack_result);

	/**
	 * \brief calculates next m_state if it is currently Up1, Down1, Right1 or Left1
	 * \param move last attack Coordinate
	 * \param last_attack_result last attack result
	 */
	void check_1(Coordinate move, const AttackResult& last_attack_result);

	/**
	 * \brief calculates next m_state if it is currently  Down2 or Left2
	 * \param move last attack Coordinate
	 * \param last_attack_result last attack result
	 */
	void check_2(Coordinate move, const AttackResult& last_attack_result);

	/**
	 * \brief updates SmartPlayer's members after a ship is sinked
	 */
	void sink_update(); 

	/**
	 * \brief removes the neighbors of an oponent's sinked ship
	 * \param ship an oponent's sinked ship
	 */
	void remove_ship_neighbors(BattleShip ship);
	
	/**
	 * \brief search val in set, and if found erase it from set.
	 * \param val value to search for in the set.
	 * \param set to search in
	 * \return true if val was found in set, else false
	 */
	static bool set_search_and_erase(const Coordinate& val, std::set<Coordinate>& set);

public:
	SmartPlayer();
	~SmartPlayer();

	// overriden functions from IBattleshipGameAlgo
	void setPlayer(int player) override;				// called every time the player changes his order
	void setBoard(const BoardData& board) override;		// called once at the beginning of each new game
	Coordinate attack() override;						// ask player for his move
	void notifyOnAttackResult(int player, Coordinate move, AttackResult result) override; // last move result

	// block copy and assignment
	SmartPlayer(const SmartPlayer&) = delete;
	SmartPlayer& operator=(const SmartPlayer&) = delete;
};
