#pragma once
#include <utility> // for std::pair
#include <vector>
#include <set>
#include <random>
#include <algorithm>
#include "IBattleshipGameAlgo.h"
#include "Battleship.h"
#include "Util.h"

class SmartPlayer : public IBattleshipGameAlgo
{
	// Nested enum class for the algorithm possible states
	enum class State {
		Search, FirstUp, FirstDown, FirstRight, FirstLeft, FirstFwd, FirstBwd, Up1, Down1, Right1, Left1, Fwd1, Bwd1, Down2, Left2, Bwd2
	};

	// class members
	const BoardData* m_board;
	int m_id;
	State m_state; // algorithm state
	std::set< Coordinate > m_potential_attacks; // potential attack moves set
	Coordinate m_last_good_attack; // last attack that hit oponent's ship
	Coordinate m_cur_first_found; // first location found of an oponent's ship, to be currently attacked
	std::set< Coordinate > m_first_found_set; // first location found of an oponent's ship (potentially), to be attacked later
	Coordinate m_ship_edge;
	std::mt19937 m_generator; //for random search
	std::vector<BattleShip> ships;

	// class private functions
	/**
	 * Push all potential attack moves to m_potential_attacks set. This function MUST be called after calling m_player.commonSetBoard()
	 */
	void init_potential_attacks();

	
	/**
	 * \brief finds all player's ships on board and puts them in ships vector
	 * \param board player's board accepted in setBoard() function, and should be valid
	 * \param ships vector to hold player's ships
	 */
	void findShips(const BoardData& board, std::vector<BattleShip>& ships); // TODO

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
	 * \param ship_start location of one edge of the ship
	 * \param ship_end location of the other edge of the ship
	 */
	void sink_update(const Coordinate& ship_start, const Coordinate& ship_end);

	/**
	 * \brief updates m_potential_attacks set after a ship is sinked
	 * \param ship_start location of one edge of the ship 
	 * \param ship_end location of the other edge of the ship
	 * \param direction 0 if ship was vertical, 1 if ship was horizontal, else 2 (had depth)
	 */
	void update_potential_attacks(const Coordinate& ship_start, const Coordinate& ship_end, int direction);

	/**
	 * \brief removes neighbors of a coordinate from m_potential_attacks set
	 * \param location one of the sinked ship's locations
	 * \param direction 0 if ship was vertical, 1 if ship was horizontal, else 2 (had depth)
	 */
	void remove_coordinate_neighbors(const Coordinate& location, int direction);
	
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
	virtual void setPlayer(int player) override;				// called every time the player changes his order
	virtual void setBoard(const BoardData& board) override;		// called once at the beginning of each new game
	virtual Coordinate attack() override;						// ask player for his move
	virtual void notifyOnAttackResult(int player, Coordinate move, AttackResult result) override; // last move result

	// block copy and assignment
	SmartPlayer(const SmartPlayer&) = delete;
	SmartPlayer& operator=(const SmartPlayer&) = delete;
};
