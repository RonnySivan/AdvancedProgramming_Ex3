#include "GameManager.h"


GameManager::GameManager(std::unique_ptr<IBattleshipGameAlgo> playerA_, std::unique_ptr<IBattleshipGameAlgo> playerB_, std::shared_ptr<OriginalBoard> originalBoard) :
	gameBoard(originalBoard), playerA(std::move(playerA_)), playerB(std::move(playerB_)), numOfShipsA(5), numOfShipsB(5), hasMoreMovesA(true), hasMoreMovesB(true), m_scorePlayerA(0), m_scorePlayerB(0)
{

	/* Notify the Players what is their Id */
	(playerA.get())->setPlayer(PLAYER_A);
	(playerB.get())->setPlayer(PLAYER_B);

	/* Set the Game and the Players boards */
	PlayerBoard playerBoardA(PLAYER_A, originalBoard);
	PlayerBoard playerBoardB(PLAYER_B, originalBoard);

	(playerA.get())->setBoard(playerBoardA);
	(playerB.get())->setBoard(playerBoardB);

	/* Set battleShipsA and battleShipsB */
	gameBoard.getBattleShips(battleShipsA, PLAYER_A);
	gameBoard.getBattleShips(battleShipsB, PLAYER_B);

}


GameManager::~GameManager()
{
	// empty d'tor
}


GameResult GameManager::runGame()
{
	auto movesCounter = 0;
	auto turn = PLAYER_A;
	GameResult gameResult(0, 0, 0);

	/* The Game Loop*/
	while (numOfShipsA > 0 && numOfShipsB > 0 && (hasMoreMovesA || hasMoreMovesB)) {
		movesCounter++;
		if (turn == PLAYER_A && hasMoreMovesA)
			turn = runPlayer(PLAYER_A);
		else if (turn == PLAYER_B && hasMoreMovesB)
			turn = runPlayer(PLAYER_B);

		// If the plyaers arent smart - stop the game after it had too many moves
		if (movesCounter > gameBoard.rows() * gameBoard.cols() * gameBoard.depth() * 2)
			break;
	}
	
	/* Create a GameResult Object to return to the Tournament Manager */
	gameResult.scorePlayerA = m_scorePlayerA;
	gameResult.scorePlayerB = m_scorePlayerB;
	if (numOfShipsA == 0)
		gameResult.winnerId = PLAYER_B;
	else if (numOfShipsB == 0)
		gameResult.winnerId = PLAYER_A;
	else gameResult.winnerId = 2;

	return gameResult;
}

//// Private Methods

int GameManager::runPlayer(int playerId)
{
	auto turn = playerId;
	auto attackCoordinate = (playerId) ? (playerB.get())->attack() : (playerA.get())->attack() ;

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
	auto attackResAndPlayer = gameBoard.checkAttackResult(attackCoordinate);
	turn = analyzeLegalAttack(playerId, turn, attackResAndPlayer, attackCoordinate);

	(playerA.get())->notifyOnAttackResult(playerId, attackCoordinate, attackResAndPlayer.first);
	(playerB.get())->notifyOnAttackResult(playerId, attackCoordinate, attackResAndPlayer.first);

	return turn;

}


bool GameManager::attackCoordinateLegal(Coordinate attackCoordinate) const {
	if (attackCoordinate.row > 0 && attackCoordinate.row <= gameBoard.rows() &&
	attackCoordinate.col > 0 && attackCoordinate.col <= gameBoard.cols() &&
	attackCoordinate.depth > 0 && attackCoordinate.depth <= gameBoard.depth())
		return true;
	return false;
}


int GameManager::analyzeLegalAttack(int playerId, int turn, std::pair<AttackResult, int>& attackResAndPlayer, Coordinate attackCoordinates)
{
	// player attack Missed.
	if (attackResAndPlayer.first == AttackResult::Miss)
	{
		// if the other player has more moves - pass the turn.
		if ((playerId == PLAYER_A && hasMoreMovesB) || (playerId == PLAYER_B && hasMoreMovesA))
			turn = (turn + 1) % 2;
	}
	else
	{ // player attack Hit
		if ((playerId == PLAYER_A && attackResAndPlayer.second == PLAYER_A) || (playerId == PLAYER_B && attackResAndPlayer.second == PLAYER_B))
		{ // self attack
			if ((playerId == PLAYER_A && hasMoreMovesB) || (playerId == PLAYER_B && hasMoreMovesA))
				turn = (turn + 1) % 2;
			attackResAndPlayer.first = checkHitResult(playerId, attackCoordinates, (playerId) ? battleShipsB : battleShipsA); // check if it is a Hit or a Sink
		}
		else if ((playerId == PLAYER_A && attackResAndPlayer.second == PLAYER_B) || (playerId == PLAYER_B && attackResAndPlayer.second == PLAYER_A))
		{ // Hit Other Player
			attackResAndPlayer.first = checkHitResult((playerId) ? PLAYER_A : PLAYER_B, attackCoordinates, (playerId) ? battleShipsA : battleShipsB); // check if it is a Hit or a Sink
		}
		else
		{ // The player attacked an allready hitted battleShip (that didn't sink yet)
			if ((playerId == PLAYER_A && hasMoreMovesB) || (playerId == PLAYER_B && hasMoreMovesA))
			turn = (turn + 1) % 2;
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


