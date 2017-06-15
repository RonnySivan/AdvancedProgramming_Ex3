#pragma once

//#include <time.h> //TODO: unused?
#include <vector>
#include <tuple>
//#include <iostream> //TODO: unused?
//#include <fstream> //TODO: unused?
//#include <sstream> //TODO: unused?
//#include <windows.h> //TODO: unused?
//#include <algorithm> //TODO: unused?
#include "IBattleshipGameAlgo.h"

#define MAX_ABS_PATH  1024

/*functions for mmapping Coordinate*/

// this is one way to define hash function for a type
// see: http://en.cppreference.com/w/cpp/utility/hash
struct MyHash {
	std::size_t operator()(const Coordinate& c) const {
		return c.row * 7 + c.col * 5 + c.depth * 11;
	}
};

std::string to_string(Coordinate c);

std::ostream& operator<<(std::ostream& out, const Coordinate& c);

// required for unordered_map
bool operator==(const Coordinate& c1, const Coordinate& c2);

// required for SmartPlayer
bool operator!=(const Coordinate& c1, const Coordinate& c2);

// required for map
bool operator<(const Coordinate& c1, const Coordinate& c2);



class Util
{
public:

	/*
	*concatenating relative path to dir path
	*/
	static std::string concatenateAbsolutePath(const std::string& dirPath, const std::string& fileName);


	/*
	* returns absolite path for the relative path given
	*/
	static std::string findAbsPath(const char* relativePath);


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
	* suffix - suffix a file we are looking for
	* filesList - a vector where we'll hold all the files in the given folder
	* foundFiles - an empty vector where we'll keep all file names with given suffix
	*/
	static void findAllFilesWithSuffix(std::vector<std::string>& filesList, std::vector<std::string> &foundFiles, std::string suffix);


	/*
	* copied from recitation
	* returns true iff value ends with ending
	*/
	static bool endsWith(std::string& value, std::string& ending);


	/*
	*splits string s to a vector using delim
	*/
	static std::vector<std::string> split(const std::string& s, char delim);


	/**
	* Get current date/time string, to append to the Logger oupput.
	* Format is YYYY-MM-DD.HH:mm:ss
	*/
	static std::string CurrentDateTime();


};