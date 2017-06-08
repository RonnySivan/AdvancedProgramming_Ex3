#include "SmartPlayer.h"
#include <iostream> // TODO: this is only for debug prints, remove before submission


SmartPlayer::SmartPlayer() : m_board(nullptr), // TODO: move to GeneralPlayer if we keep m_player
							 m_id(-1),
                             m_state(State::Search),
							 m_last_good_attack(0, 0, 0),
                             m_cur_first_found(0, 0, 0),
                             m_ship_edge(0, 0, 0),
                             m_generator(std::random_device{}())
{
}


SmartPlayer::~SmartPlayer()
{
	std::cout << "in Dtor of SmartPlayer - should get here BEFORE BoardData is dead!!!" << std::endl; // TODO: DELETE
}

void SmartPlayer::setPlayer(int player)
{
	m_id = player;
}

//void SmartPlayer::setBoard(int player, const char** board, int numRows, int numCols) TODO: DELETE
void SmartPlayer::setBoard(const BoardData& board)
{
	// initialize player's members for a new board:
	m_board = &board;
	m_state = State::Search;
	m_potential_attacks.clear();
	init_potential_attacks();
	m_last_good_attack = m_cur_first_found = m_ship_edge = Coordinate(0, 0, 0);
	m_first_found_set.clear();
}

void SmartPlayer::init_potential_attacks()
{
	// init a sorted vector for efficient find by value (binary search using lower_bound)
	// TODO: set range 1-10 instead of 0-9?
	for (auto i = 1; i <= m_board->rows(); ++i)
	{
		for (auto j = 1; j <= m_board->cols(); ++j)
		{
			for (auto k = 1; k <= m_board->depth(); ++k)
			{
				Coordinate cur(i, j, k);
				if (m_board->charAt(cur) == ' ')
				{
					m_potential_attacks.push_back(cur);
				}
			}
		}
	}
}

Coordinate SmartPlayer::attack()
{
	if (m_potential_attacks.empty()) // no potential attacks - out of moves
		return {-1, -1, -1};
	if (m_state == State::Search) // no oponent's ship was found yet
		return attackRand();
	return attackState(); // else - oponent's ship was found
}

void SmartPlayer::notifyOnAttackResult(int player, Coordinate move, AttackResult result)
{
	if (player == m_id)
		calc_state(player, move, result);
	else
		oponent_attack(player, move, result);
}

Coordinate SmartPlayer::attackRand()
{
	auto range = m_potential_attacks.size() - 1;
	// generate a random number from 1 to range, assuming uniform distribution
	std::uniform_int_distribution<size_t> distribution(0, range);
	auto random = distribution(m_generator);

	auto ans = m_potential_attacks[random]; // keep the attack move to be returned before erasing it from vector
	m_potential_attacks.erase(m_potential_attacks.cbegin() + random);
	return ans;
}

Coordinate SmartPlayer::attackState()
{
	Coordinate ans{0, 0, 0};
	bool done;

	if (m_state == State::FirstUp || m_state == State::FirstDown || m_state == State::FirstRight || m_state == State::FirstLeft || m_state == State::FirstFwd || m_state == State::FirstBwd)
		check_neighbors(m_last_good_attack);
	
	do
	{
		ans = m_last_good_attack;
		if (m_state == State::FirstUp || m_state == State::Up1)
			ans.row--;
		else if (m_state == State::FirstDown || m_state == State::Down1 || m_state == State::Down2)
			ans.row++;
		else if (m_state == State::FirstRight || m_state == State::Right1)
			ans.col++;
		else if (m_state == State::FirstLeft || m_state == State::Left1 || m_state == State::Left2)
			ans.col--;
		else if (m_state == State::FirstFwd || m_state == State::Fwd1)
			ans.depth++;
		else if (m_state == State::FirstBwd || m_state == State::Bwd1 || m_state == State::Bwd2)
			ans.depth--;

		done = check_attack(ans);
	} while (!done);
	
	
	return  ans;
}

void SmartPlayer::check_neighbors(const Coordinate& attack)
{
	if (m_first_found_set.empty())
		return;

	// check if attack's neighbors were already self-hit by oponent - belongs to the same ship being currently attacked
	std::vector<Coordinate> neighbors(6);
	neighbors[0] = neighbors[1] = neighbors[2] = neighbors[3] = neighbors[4] = neighbors[5] = attack;
	neighbors[0].row--;
	neighbors[1].row++;
	neighbors[2].col++;
	neighbors[3].col--;
	neighbors[4].depth++;
	neighbors[5].depth--;

	for (auto i = 0; i < neighbors.size(); ++i)
	{
		if (binary_search_and_erase(m_first_found_set.cbegin(), m_first_found_set.cend(), neighbors[i], m_first_found_set)) {
			m_last_good_attack = neighbors[i];
			switch (i)
			{
				case 0:	m_state = State::Up1;		break;
				case 1:	m_state = State::Down1;		break;
				case 2:	m_state = State::Right1;	break;
				case 3:	m_state = State::Left1;		break;
				case 4:	m_state = State::Fwd1;		break;
				case 5:	m_state = State::Bwd1;		break;
			}
			return;
		}
	}
}

bool SmartPlayer::check_attack(const Coordinate& attack)
{	
	// check if attack is in potential attacks vector
	if (binary_search_and_erase(m_potential_attacks.cbegin(), m_potential_attacks.cend(), attack, m_potential_attacks)) 
		return true;  // attack was found in potential attacks vector end removed from it
	
	// else - attack is not a potential attack, go to next state
	calc_state(m_id, attack, AttackResult::Miss);
	return false;
}

void SmartPlayer::calc_state(int playerID, Coordinate move, const AttackResult& last_attack_result)
{
	if (m_state == State::Search) {
		if (last_attack_result == AttackResult::Hit) { // found oponent's ship for the first time 
			m_cur_first_found = m_last_good_attack = move;
			m_state = State::FirstUp; // try to go up
		} // else: missed or sinked ship of size 1 - stay in search state
		else if (last_attack_result == AttackResult::Sink) // self sink of oponent's ship of size 1
			update_potential_attacks(move, move, true);
	} 
	else {
		if (playerID != m_id && last_attack_result == AttackResult::Hit) {
			m_first_found_set.insert(move); // oponent's self hit - save the location for later
			return;
		}

		if (m_state == State::FirstUp || m_state == State::FirstDown || m_state == State::FirstRight || m_state == State::FirstLeft || m_state == State::FirstFwd || m_state == State::FirstBwd) {
			check_first(move, last_attack_result);
		}
		else if (m_state == State::Up1 || m_state == State::Down1 || m_state == State::Right1 || m_state == State::Left1 || m_state == State::Fwd1 || m_state == State::Bwd1) {
			check_1(move, last_attack_result);
		}
		else if (m_state == State::Down2 || m_state == State::Left2 || m_state == State::Bwd2) {
			check_2(move, last_attack_result);
		}
	}
}

void SmartPlayer::oponent_attack(int playerID, Coordinate move, const AttackResult & last_attack_result)
{
	// check if attack move is in potential attacks vector
	if (binary_search_and_erase(m_potential_attacks.cbegin(), m_potential_attacks.cend(), move, m_potential_attacks)) {
		// attack move was found in potential attacks vector and removed from it
		if (last_attack_result != AttackResult::Miss) // oponent's self hit/sink
			calc_state(playerID, move, last_attack_result);
	}
}

void SmartPlayer::check_first(Coordinate move, const AttackResult& last_attack_result)
{
	if (last_attack_result == AttackResult::Miss) { // try a different direction
		switch (m_state) {
			case State::FirstUp:	m_state = State::FirstDown;		break;
			case State::FirstDown:	m_state = State::FirstRight;	break;
			case State::FirstRight:	m_state = State::FirstLeft;		break;
			case State::FirstLeft:	m_state = State::FirstFwd;		break;
			case State::FirstFwd:	m_state = State::FirstBwd;		break;
			//case State::FirstBwd: // last check - should not get here
		}
	}
	else {
		m_last_good_attack  = move;
		if (last_attack_result == AttackResult::Hit) { // continue going at the same direction
			switch (m_state) {
				case State::FirstUp:	m_state = State::Up1;		break;
				case State::FirstDown:	m_state = State::Down1;		break;
				case State::FirstRight:	m_state = State::Right1;	break;
				case State::FirstLeft:	m_state = State::Left1;		break;
				case State::FirstFwd:	m_state = State::Fwd1;		break;
				case State::FirstBwd:	m_state = State::Bwd1;		break;
			}
		}
		else // Sink
			sink_update(m_cur_first_found, m_last_good_attack);
	}
}

void SmartPlayer::check_1(Coordinate move, const AttackResult & last_attack_result)
{
	if (last_attack_result == AttackResult::Miss) { // not done with ship - go at opposite direction
		m_ship_edge = m_last_good_attack;
		m_last_good_attack = m_cur_first_found;
		switch (m_state) {
			case State::Up1:	m_state = State::Down2;	break;
			//case State::Down1: // should not get here
			case State::Right1:	m_state = State::Left2;	break;
			//case State::Left1: // should not get here
			case State::Fwd1:	m_state = State::Bwd2;	break;
			//case State::Bwd1: // should not get here
		}
	}
	else {
		if (last_attack_result == AttackResult::Sink)
			sink_update(m_cur_first_found, move);
		else // Hit - update last successful attack and continue going at the same direction
			m_last_good_attack = move;
	}
}

void SmartPlayer::check_2(Coordinate move, const AttackResult & last_attack_result)
{
	if (last_attack_result == AttackResult::Sink) 
		sink_update(m_ship_edge, move);
	else // Hit - update last successful attack and continue going at the same direction
		m_last_good_attack  = move;
}

void SmartPlayer::sink_update(const Coordinate& ship_start, const Coordinate& ship_end) {
	if (m_state == State::FirstUp || m_state == State::Up1
		|| m_state == State::FirstDown || m_state == State::Down1 || m_state == State::Down2) // vertical
		update_potential_attacks(ship_start, ship_end, 0);
	else if (m_state == State::FirstRight || m_state == State::Right1
		|| m_state == State::FirstLeft || m_state == State::Left1 || m_state == State::Left2) // horizontal
		update_potential_attacks(ship_start, ship_end, 1);
	else // ship with depth (or ship of size 2)
		update_potential_attacks(ship_start, ship_end, 2);

	m_last_good_attack = m_cur_first_found = Coordinate(0, 0, 0); // reset
	m_state = State::Search; // done with ship
	if (!m_first_found_set.empty()) { // we already found other ships
		auto first = m_first_found_set.cbegin();
		auto found = *first;
		m_first_found_set.erase(first);
		calc_state(m_id, found, AttackResult::Hit);
	}
}

void SmartPlayer::update_potential_attacks(const Coordinate& ship_start, const Coordinate& ship_end, int direction)
{ // TODO: update to receive ship as a parameter
	// remove neighbors from the edges // TODO: update for 3D
	remove_coordinate_neighbors(ship_start, true);
	remove_coordinate_neighbors(ship_start, false);

	//if (ship_start != ship_end) // TODO: check why it doesn't compile - use global operator == from Util after Tiana's push (Amir's example)
	if ((ship_start.row != ship_end.row) || (ship_start.col != ship_end.col) || (ship_start.depth != ship_end.depth))
	{
		for (auto i = 0; i < 2; ++i)
		{
			remove_coordinate_neighbors(ship_end, i);
		}

		// remove neighbors from the middle
		int range;
		Coordinate cur{ 0, 0, 0 };
		if (direction == 0) { // vertical
			range = std::abs(ship_end.row - ship_start.row);
			if (range > 1) {
				cur = ship_end.row > ship_start.row ? ship_start : ship_end;
				for (auto i = 1; i < range; ++i)
				{
					cur.row += i;
					remove_coordinate_neighbors(cur, 0);
				}
			}
		}
		else if (direction == 1) { // horizontal
			range = std::abs(ship_end.col - ship_start.col);
			if (range > 1) {
				cur = ship_end.col > ship_start.col ? ship_start : ship_end;
				for (auto i = 1; i < range; ++i)
				{
					cur.col += i;
					remove_coordinate_neighbors(cur, 1);
				}
			}

		}
		else { // direction == 2 - depth
			// TODO
		}
		
	}
}

void SmartPlayer::remove_coordinate_neighbors(const Coordinate& location, int direction) { // TODO: update
	std::vector<Coordinate> neighbors(4);
	neighbors[0] = neighbors[1] = neighbors[2] = neighbors[3] = location;
	switch (direction)
	{
		case 0:	// vertical
			neighbors[0].col -= 1;
			neighbors[1].col += 1;
			neighbors[2].depth -= 1;
			neighbors[3].depth += 1;
			break;
		case 1: // horizontal
			neighbors[0].row -= 1;
			neighbors[1].row += 1;
			neighbors[2].depth -= 1;
			neighbors[3].depth += 1;
			break;
		case 2: // depth
			neighbors[0].col -= 1;
			neighbors[1].col += 1;
			neighbors[2].row -= 1;
			neighbors[3].row += 1;
			break;
		default: 
			std::cout << "@ remove_coordinate_neighbors got wrong direction = " << direction << std::endl; // TODO: DELETE
	}
	
	for (auto neighbor : neighbors) {
		// if neighbor is in m_first_found_set - belongs to a sinked ship, should remove it and erase it's neighbours
		if (!m_first_found_set.empty() && binary_search_and_erase(m_first_found_set.cbegin(), m_first_found_set.cend(), neighbor, m_first_found_set)) {
			remove_coordinate_neighbors(neighbor, direction);
		}
		binary_search_and_erase(m_potential_attacks.cbegin(), m_potential_attacks.cend(), neighbor, m_potential_attacks);
	}
}

template <class ForwardIterator, class T, class Container>
bool SmartPlayer::binary_search_and_erase(ForwardIterator first, ForwardIterator last, const T& val, Container& cont) {
	first = std::lower_bound(first, last, val);
	bool ans = (first != last && !(val < *first));
	if (ans)
		cont.erase(first);
	return ans;
}

IBattleshipGameAlgo* GetAlgorithm()
{
	return new SmartPlayer();
}