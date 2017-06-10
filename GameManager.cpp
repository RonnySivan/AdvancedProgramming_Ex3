#include "GameManager.h"


GameManager::GameManager(IBattleshipGameAlgo* playerA, IBattleshipGameAlgo* playerB, std::shared_ptr<OriginalBoard> originalBoard) :
	gameBoard(originalBoard), playerA(playerA), playerB(playerB), numOfShipsA(5), numOfShipsB(5), hasMoreMovesA(true), hasMoreMovesB(true), m_scorePlayerA(0), m_scorePlayerB(0)
{

	/* Notify the Players what is their Id */
	playerA->setPlayer(PLAYER_A);
	playerB->setPlayer(PLAYER_B);

	/* Set the Game and the Players boards */
	PlayerBoard playerBoardA(PLAYER_A, originalBoard);
	PlayerBoard playerBoardB(PLAYER_B, originalBoard);

	playerA->setBoard(playerBoardA);
	playerB->setBoard(playerBoardB);

	/* Set battleShipsA and battleShipsB */
	gameBoard.getBattleShips(battleShipsA, PLAYER_A);
	gameBoard.getBattleShips(battleShipsB, PLAYER_B);

}

GameManager::~GameManager()
{
	// todo - empty dtor? 
}


GameResult GameManager::runGame()
{
	auto turn = PLAYER_A;
	GameResult gameResult(0, 0, 0);

	/* The Game Loop*/
	while (numOfShipsA > 0 && numOfShipsB > 0 && (hasMoreMovesA || hasMoreMovesB)) {

	if (turn == PLAYER_A && hasMoreMovesA)
		turn = runPlayer(PLAYER_A);
	else if (turn == PLAYER_B && hasMoreMovesB)
		turn = runPlayer(PLAYER_B);
	}
	
	/* Create a GameResult Object to return to the Tournament Manager */
	gameResult.scorePlayerA = m_scorePlayerA;
	gameResult.scorePlayerB = m_scorePlayerB;
	gameResult.winnerId = (numOfShipsA == 0) ? PLAYER_B : PLAYER_A;

	return gameResult;
}

//// Private Methods

int GameManager::runPlayer(int playerId)
{
	auto turn = playerId;
	auto attackCoordinate = (playerId) ? playerB->attack() : playerA->attack() ;

	// End of moves for player
	if (attackCoordinate.row == -1 && attackCoordinate.col == -1 && attackCoordinate.depth == -1)
	{
		(playerId) ? hasMoreMovesB = false : hasMoreMovesA = false;
		return (turn + 1) % 2;
	}

	// Check that the attack is legal
	if (!attackCoordinateLegal(attackCoordinate)) {
		return ((playerId == PLAYER_A && hasMoreMovesB) || (playerId == PLAYER_B && hasMoreMovesA)) ? (turn + 1) % 2 : turn;
	}

	// Check if the attak is a Hit or a Miss
	std::pair<AttackResult, int> attackResAndPlayer = gameBoard.checkAttackResult(attackCoordinate);
	turn = analyzeLegalAttack(playerId, turn, attackResAndPlayer, attackCoordinate);

	playerA->notifyOnAttackResult(playerId, attackCoordinate, attackResAndPlayer.first);
	playerB->notifyOnAttackResult(playerId, attackCoordinate, attackResAndPlayer.first);

	return turn;

}


bool GameManager::attackCoordinateLegal(Coordinate attackCoordinate) { // TODO - resharper message
	if (attackCoordinate.row > 0 && attackCoordinate.row <= gameBoard.rows() &&
	attackCoordinate.col > 0 && attackCoordinate.col <= gameBoard.cols() &&
	attackCoordinate.depth > 0 && attackCoordinate.depth <= gameBoard.depth())
		return true;
	return false;
}


int GameManager::analyzeLegalAttack(int playerId, int turn, std::pair<AttackResult, int>& attackResAndPlayer, Coordinate& attackCoordinates)
{
	// player attack Missed.
	if (attackResAndPlayer.first == AttackResult::Miss)
	{
		// if the other player has more moves - pass the turn.
		if ((playerId == PLAYER_A && hasMoreMovesB) || (playerId == PLAYER_B && hasMoreMovesA))
			turn = (turn + 1) % 2;
		std::cout << "Miss" << std::endl; // TODO - remove before submit - here for debug!
	}
	else
	{ // player attack Hit
		if ((playerId == PLAYER_A && attackResAndPlayer.second == PLAYER_A) || (playerId == PLAYER_B && attackResAndPlayer.second == PLAYER_B))
		{ // self attack
			if ((playerId == PLAYER_A && hasMoreMovesB) || (playerId == PLAYER_B && hasMoreMovesA))
				turn = (turn + 1) % 2;
			attackResAndPlayer.first = checkHitResult(playerId, attackCoordinates, (playerId) ? battleShipsB : battleShipsA); // check if it is a Hit or a Sink
			// TODO - remove before submit - here for debug! :
			if (attackResAndPlayer.first == AttackResult::Sink)	std::cout << "Self Attack - Sink" << std::endl;
			else std::cout << "Self Attack - Hit" << std::endl;
		}
		else if ((playerId == PLAYER_A && attackResAndPlayer.second == PLAYER_B) || (playerId == PLAYER_B && attackResAndPlayer.second == PLAYER_A))
		{ // Hit Other Player
			attackResAndPlayer.first = checkHitResult((playerId) ? PLAYER_A : PLAYER_B, attackCoordinates, (playerId) ? battleShipsA : battleShipsB); // check if it is a Hit or a Sink
			// TODO - remove before submit - here for debug! :
			if (attackResAndPlayer.first == AttackResult::Sink) std::cout << "Sink other player battleShip" << std::endl;
			else std::cout << "Hit other player battleShip" << std::endl;

		}
		else
		{ // The player attacked an allready hitted battleShip (that didn't sink yet)
			if ((playerId == PLAYER_A && hasMoreMovesB) || (playerId == PLAYER_B && hasMoreMovesA))
			turn = (turn + 1) % 2;
			std::cout << "Hit battleShip, again..." << std::endl; // TODO - remove before submit - here for debug!
		}
	}
	return turn;
}



AttackResult GameManager::checkHitResult(int playerId, Coordinate attackMove, std::vector<BattleShip> &battleShips) {
	auto res = AttackResult::Hit;

	for (auto i = 0; i < battleShips.size(); ++i)
	{
		res = battleShips[i].shipAttackResult(attackMove);

		if (res != AttackResult::Miss)
		{
			if (res == AttackResult::Sink) {
			(playerId) ? numOfShipsB-- : numOfShipsA--;
			(playerId) ? m_scorePlayerA += battleShips[i].getScore() : m_scorePlayerB += battleShips[i].getScore();
			updateSinkShipInBoard(playerId, battleShips[i]);
			}
			else { // res = Hit
				gameBoard.setSymbol(attackMove, playerId ? 'h' : 'H');
			}
			break;
		}
	}

	return res;
}

void GameManager::updateSinkShipInBoard(int playerID, BattleShip& battleShip) {
	for (auto i = 0; i < battleShip.getLocations().size(); i++)
	{
		gameBoard.setSymbol(battleShip.getLocations()[i], (playerID ? 'x' : 'X'));
	}
}


