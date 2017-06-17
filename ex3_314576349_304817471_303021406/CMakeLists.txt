#############################################################
### YOU SHOULD EDIT THE BELOW WITH YOUR DETAILS AND FILES ###
#############################################################

set(user1 ronnysivan)
set(user2 tatianal)
set(user3 sapirbenor)
set(id1 "304817471")
set(id2 "314576349")
set(id3 "303021406")

# All source files needed to compile your exe
set (EXE_SOURCE_FILES
	BattleShip.h
	BattleShip.cpp
	GameBoard.h
	GameBoard.cpp
	GameManager.h
	GameManager.cpp
	iBattleshipGameAlgo.h
	Logger.h
	Logger.cpp
	main.cpp
	OriginalBoard.h
	OriginalBoard.cpp
	PlayerBoard.h
	PlayerBoard.cpp
	TournamentManager.h
	TournamentManager.cpp
	Util.h
	Util.cpp
)
# All source files needed to compile your smart algorithm's .dll
set (SMART_ALGO_SOURCE_FILES
	BattleShip.h
	BattleShip.cpp
	iBattleshipGameAlgo.h
	SmartPlayer.h
	SmartPlayer.cpp
	Util.h
	Util.cpp
)

##################################
### DO NOT CHANGE FROM HERE ON ###
##################################

message("user: ${user1}, id: ${id1}")

add_definitions(-DUNICODE -D_UNICODE)

set(EX ex3)

# Set exe build parameters
set(EXE_NAME "${EX}.${user1}")
add_executable( ${EXE_NAME}	${EXE_SOURCE_FILES})
set_property(TARGET ${EXE_NAME} PROPERTY FOLDER "${EX}/${user1}")
set_property(TARGET ${EXE_NAME} PROPERTY OUTPUT_NAME "${user1}")

# Set smart algorithm's .dll build parameters
set(ALG3_NAME "${EX}.${user1}.smart")
add_library(${ALG3_NAME} SHARED ${SMART_ALGO_SOURCE_FILES})
set_property(TARGET ${ALG3_NAME} PROPERTY FOLDER "${EX}/${user1}")
set_property(TARGET ${ALG3_NAME} PROPERTY OUTPUT_NAME "${user1}.smart")
target_compile_definitions(${ALG3_NAME} PRIVATE -DALGO_EXPORTS) # preprocessor flag saying we are the one compiling the header now