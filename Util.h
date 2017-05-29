#pragma once

#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <windows.h>
#include <algorithm>

// for printing:
#define A_COLOR	0xb0
#define B_COLOR	0xe0
#define ATTACK_COLOR 0xcf
#define DEFAULT_DELAY 1000
#define MAX_ABS_PATH  1024

class Util
{
public:
	/*
	* path - string of the path of the folder containing the files to be used
	* filesList - a vector where we'll hold all the files in the given folder
	* return true if succedded and false otherwise (invalid path)
	*/
	static bool findFiles(const std::string& path, std::vector<std::string>& filesList);

	/*
	* suffix - suffix a file we are looking for
	* filesList - a vector where we'll hold all the files in the given folder
	* return the first file name with the given suffix, or "" if there isn't one
	*/
	static std::string findSuffix(std::vector<std::string>& filesList, std::string suffix, int num);

	/*
	* returns absolite path for the relative path given
	*/
	static std::string findAbsPath(const char* relativePath);

	/*
	* copied from recitation
	* returns true iff value ends with ending
	*/
	static bool endsWith(std::string& value, std::string& ending);

	/*
	*concatenating relative path to dir path
	*/
	static std::string concatenateAbsolutePath(const std::string& dirPath, const std::string& fileName);

	/*
	*splits string s to a vector using delim
	*/
	static std::vector<std::string> split(const std::string& s, char delim);

	/*checks the fienames aren't empty
	* if one or more are empty, prints rellevent massage
	* returns true if all strings != ""
	*/
	static bool existsFiles(std::string& fileBoard, std::string& fileDllA, std::string& fileDllB, const std::string& path, bool isLegalBoard);

	/*
	* hides the cursor in console
	*/
	static void hideCursor(bool flag);

	/*
	 * gets the names of all the files in the path given and extracts the default parameters from config file
	 * returns true iff succedded
	 */
	bool setDefaultArgs(std::vector<std::string>& filesList, bool& quiet, int& delay, int& numOfThreads);

	/**
	* initializes program veriables
	*/
	static void initMain(int argc, char* argv[], std::string& path, bool& quiet, int& delay);

};