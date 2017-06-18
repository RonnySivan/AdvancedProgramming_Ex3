#include "SmartPlayer.h"
#include "Util.h"

SmartPlayer::SmartPlayer() : m_id(-1),
                             m_state(State::Search),
							 m_last_good_attack(0, 0, 0),
                             m_cur_first_found(0, 0, 0),
                             m_generator(std::random_device{}())
{
}


SmartPlayer::~SmartPlayer()
{
}

void SmartPlayer::setPlayer(int player)
{
	m_id = player;
}

void SmartPlayer::setBoard(const BoardData& board)
{
	// initialize player's members for a new board:
	m_state = State::Search;
	m_potential_attacks.clear();
	init_potential_attacks(board);
	findShips(board);
	m_last_good_attack = m_cur_first_found = Coordinate(0, 0, 0);
	m_first_found_set.clear();
	m_oponent_ship.clearLocations();
}

void SmartPlayer::init_potential_attacks(const BoardData& board)
{
	for (auto i = 1; i <= board.rows(); ++i)
	{
		for (auto j = 1; j <= board.cols(); ++j)
		{
			for (auto k = 1; k <= board.depth(); ++k)
			{
				Coordinate cur(i, j, k);
				if (board.charAt(cur) == ' ')
				{
					m_potential_attacks.insert(cur);
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
	
	// get that random coordinate from m_potential_attacks
	auto it(m_potential_attacks.cbegin());
	advance(it, random);
	auto ans = *it; // keep the attack move to be returned before erasing it from vector
	m_potential_attacks.erase(it);
	return ans;
}

Coordinate SmartPlayer::attackState()
{
	Coordinate ans{0, 0, 0};
	bool done;

	if (!m_first_found_set.empty() && 
		(m_state == State::FirstUp || m_state == State::FirstDown || m_state == State::FirstRight ||
		m_state == State::FirstLeft || m_state == State::FirstFwd || m_state == State::FirstBwd))
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
		else if (m_state == State::FirstBwd || m_state == State::Bwd1 || m_state == State::Bwd2) {
			ans.depth--;
			done = check_attack(ans);
			if (!done){ // for safety
				sink_update();
				return attackRand();
			}
			break;
		}

		done = check_attack(ans);
			
	} while (!done);

	return  ans;
}

void SmartPlayer::check_neighbors(const Coordinate& attack)
{
	// check if attack's neighbors were already self-hit by oponent - belongs to the same ship being currently attacked
	std::vector<Coordinate> neighbors(6, { 0, 0, 0 });
	neighbors[0] = neighbors[1] = neighbors[2] = neighbors[3] = neighbors[4] = neighbors[5] = attack;
	neighbors[0].row--;
	neighbors[1].row++;
	neighbors[2].col++;
	neighbors[3].col--;
	neighbors[4].depth++;
	neighbors[5].depth--;

	for (auto i = 0; i < neighbors.size(); ++i)
	{
		if (set_search_and_erase(neighbors[i], m_first_found_set)) {
			m_last_good_attack = neighbors[i];
			m_oponent_ship.addLocation(neighbors[i]);
			switch (i)
			{
				case 0:	m_state = State::Up1;		break;
				case 1:	m_state = State::Down1;		break;
				case 2:	m_state = State::Right1;	break;
				case 3:	m_state = State::Left1;		break;
				case 4:	m_state = State::Fwd1;		break;
				case 5:	m_state = State::Bwd1;		break;
				default: m_state = State::Search;	break; // for safety
			}
			return;
		}
	}
}

bool SmartPlayer::check_attack(const Coordinate& attack)
{	
	// check if attack is in potential attacks vector
	if (set_search_and_erase(attack, m_potential_attacks)) 
		return true;  // attack was found in potential attacks vector end removed from it
	
	// else - attack is not a potential attack, go to next state
	calc_state(m_id, attack, AttackResult::Miss);
	return false;
}

void SmartPlayer::calc_state(int playerID, Coordinate move, const AttackResult& last_attack_result)
{
	if (m_state == State::Search) {
		if (last_attack_result != AttackResult::Miss) {
			m_oponent_ship.addLocation(move);
			if (last_attack_result == AttackResult::Hit) { // found oponent's ship for the first time 
				m_cur_first_found = m_last_good_attack = move;
				m_state = State::FirstUp; // try to go up
			} else if (last_attack_result == AttackResult::Sink) // self sink of oponent's ship of size 1 - stay in search state
				sink_update();
		} // else: missed - stay in search state
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
	if (set_search_and_erase(move, m_potential_attacks)) {
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
			default:				m_state = State::Search;		break; // for safety
		}
	}
	else {
		m_last_good_attack  = move;
		m_oponent_ship.addLocation(move);
		if (last_attack_result == AttackResult::Hit) { // continue going at the same direction
			switch (m_state) {
				case State::FirstUp:	m_state = State::Up1;		break;
				case State::FirstDown:	m_state = State::Down1;		break;
				case State::FirstRight:	m_state = State::Right1;	break;
				case State::FirstLeft:	m_state = State::Left1;		break;
				case State::FirstFwd:	m_state = State::Fwd1;		break;
				case State::FirstBwd:	m_state = State::Bwd1;		break;
				default:				m_state = State::Search;	break; // for safety
			}
		}
		else // Sink
			sink_update();
	}
}

void SmartPlayer::check_1(Coordinate move, const AttackResult & last_attack_result)
{
	if (last_attack_result == AttackResult::Miss) { // not done with ship - go at opposite direction
		m_last_good_attack = m_cur_first_found;
		switch (m_state) {
			case State::Up1:	m_state = State::Down2;		break;
			case State::Right1:	m_state = State::Left2;		break;
			case State::Fwd1:	m_state = State::Bwd2;		break;
			default:			m_state = State::Search;	break; // for safety
		}
	}
	else {
		m_oponent_ship.addLocation(move);
		if (last_attack_result == AttackResult::Sink)
			sink_update();
		else // Hit - update last successful attack and continue going at the same direction
			m_last_good_attack = move;
	}
}

void SmartPlayer::check_2(Coordinate move, const AttackResult & last_attack_result)
{
	if (last_attack_result == AttackResult::Sink) 
		sink_update();
	else // Hit - update last successful attack and continue going at the same direction
		m_last_good_attack  = move;
}

void SmartPlayer::sink_update() {
	remove_ship_neighbors(m_oponent_ship);

	m_last_good_attack = m_cur_first_found = Coordinate(0, 0, 0); // reset
	m_oponent_ship.clearLocations();
	m_state = State::Search; // done with ship
	if (!m_first_found_set.empty()) { // we already found other oponent's ships
		auto first = m_first_found_set.cbegin();
		auto found = *first;
		m_first_found_set.erase(first);
		calc_state(m_id, found, AttackResult::Hit);
	}
}

void SmartPlayer::remove_ship_neighbors(BattleShip ship)
{
	std::vector<Coordinate> neighbors(6, { 0, 0, 0 });
	

	for (auto location : ship.getLocations())
	{
		neighbors[0] = neighbors[1] = neighbors[2] = neighbors[3] = neighbors[4] = neighbors[5] = location;
		neighbors[0].row--;
		neighbors[1].row++;
		neighbors[2].col++;
		neighbors[3].col--;
		neighbors[4].depth++;
		neighbors[5].depth--;

		for (auto neighbor : neighbors) {
			// if neighbor is in m_first_found_set - belongs to a sinked ship, should remove it and erase it's neighbours
			if (!m_first_found_set.empty() && set_search_and_erase(neighbor, m_first_found_set)) {
				BattleShip temp_ship{};
				temp_ship.addLocation(neighbor);
				remove_ship_neighbors(temp_ship);
			}
			set_search_and_erase(neighbor, m_potential_attacks);
		}
	}
}

bool SmartPlayer::set_search_and_erase(const Coordinate& val, std::set<Coordinate>& set) {
	auto it = set.find(val);
	if (it != set.end()) 	{
		set.erase(val);
		return true;
	}
	return false;
}

void SmartPlayer::remove_illegal_horz(int r, int c, int d, std::vector<Coordinate>& locations, const BoardData& board)
{
	auto len = 0;
	auto type = board.charAt(Coordinate(r, c, d));
	if (c > 1)
	{
		set_search_and_erase(Coordinate(r, c - 1, d), m_potential_attacks);
	}
	for (auto k = c; k <= board.cols(); k++)
	{
		auto cur = board.charAt(Coordinate(r, k, d));
		if (cur != type)
		{
			set_search_and_erase(Coordinate(r, k, d), m_potential_attacks);
			break;
		}
		set_search_and_erase(Coordinate(r - 1, k, d), m_potential_attacks);
		set_search_and_erase(Coordinate(r + 1, k, d), m_potential_attacks);
		set_search_and_erase(Coordinate(r, k, d - 1), m_potential_attacks);
		set_search_and_erase(Coordinate(r, k, d + 1), m_potential_attacks);
		locations.push_back(Coordinate(r, k, d));
		len++;
	}
}


void SmartPlayer::remove_illegal_vert(int r, int c, int d, std::vector<Coordinate>& locations, const BoardData& board)
{
	auto len = 0;
	auto type = board.charAt(Coordinate(r, c, d));
	if (r > 1)
	{
		set_search_and_erase(Coordinate(r - 1, c, d), m_potential_attacks);
	}
	for (auto k = r; k <= board.rows(); k++)
	{
		auto cur = board.charAt(Coordinate(k, c, d));
		if (cur != type)
		{
			set_search_and_erase(Coordinate(k, c, d), m_potential_attacks);
			break;
		}
		set_search_and_erase(Coordinate(k, c, d - 1), m_potential_attacks);
		set_search_and_erase(Coordinate(k, c, d + 1), m_potential_attacks);
		set_search_and_erase(Coordinate(k, c - 1, d), m_potential_attacks);
		set_search_and_erase(Coordinate(k, c + 1, d), m_potential_attacks);
		locations.push_back(Coordinate(k, c, d));
		len++;
	}
}


void SmartPlayer::remove_illegal_deep(int r, int c, int d, std::vector<Coordinate>& locations, const BoardData& board)
{
	auto len = 0;
	auto type = board.charAt(Coordinate(r, c, d));
	if (d > 1)
	{
		set_search_and_erase(Coordinate(r, c, d - 1), m_potential_attacks);
	}
	for (auto k = d; k <= board.depth(); k++)
	{
		char cur = board.charAt(Coordinate(r, c, k));
		if (cur != type)
		{
			set_search_and_erase(Coordinate(r, c, k), m_potential_attacks);
			break;
		}
		set_search_and_erase(Coordinate(r - 1, c, k), m_potential_attacks);
		set_search_and_erase(Coordinate(r + 1, c, k), m_potential_attacks);
		set_search_and_erase(Coordinate(r, c - 1, k), m_potential_attacks);
		set_search_and_erase(Coordinate(r, c + 1, k), m_potential_attacks);
		locations.push_back(Coordinate(r, c, k));
		len++;
	}
}


void SmartPlayer::findShips(const BoardData& board)
{
	for (auto k = 1; k <= board.depth(); k++)
	{
		for (auto i = 1; i <= board.rows(); i++)
		{
			for (auto j = 1; j <= board.cols(); j++)
			{
				auto type = board.charAt(Coordinate(i, j, k));
				if (type == ' ') continue;
				/*enter <if> only if cell is not empty and
				*is not part of a sequence that was already checked(from above or left or within)*/
				if (type != ' ' &&
					(i == 1 || board.charAt(Coordinate(i - 1, j, k)) != type) &&
					(j == 1 || board.charAt(Coordinate(i, j - 1, k)) != type) &&
					(k == 1 || board.charAt(Coordinate(i, j, k - 1)) != type))
				{
					std::vector<Coordinate> locV, locH, locD;
					remove_illegal_vert(i, j, k, locV, board);
					remove_illegal_horz(i, j, k, locH, board);
					remove_illegal_deep(i, j, k, locD, board);
				}
			}
		}
	}
}

IBattleshipGameAlgo* GetAlgorithm()
{
	return new SmartPlayer();
}