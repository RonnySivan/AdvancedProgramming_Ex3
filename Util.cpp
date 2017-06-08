#include "Util.h"

std::string Util::concatenateAbsolutePath(const std::string& dirPath, const std::string& fileName)
{
	if (dirPath.empty()) //no dir path is given
		return fileName;
	if (dirPath.back() == '/') //dir path includes '/'
		return dirPath + fileName;

	return dirPath + "\\" + fileName;
}

std::string Util::findAbsPath(const char* relativePath) {
	char full[MAX_ABS_PATH];
	if (_fullpath(full, relativePath, MAX_ABS_PATH) != NULL)
		return full;
	return relativePath;
}

bool Util::findFiles(const std::string& path, std::vector<std::string>& filesList)
{
	char buffer[MAX_ABS_PATH];
	std::string fullPath = "\"" + path + "\"";
	std::string cd_command = "2>NUL cd " + fullPath;

	if (system(cd_command.c_str()))
		return false;

	if (system("2>NUL cd ../"))
		return false;

	std::string dir_command = "2>NUL dir /a-d /b \"" + fullPath + "\"";
	FILE* fp = _popen(dir_command.c_str(), "r");
	while (fgets(buffer, 4095, fp))
	{
		std::string  data_str = std::string(buffer);
		data_str.resize(data_str.size() - 1); //fgets reads newline
		filesList.push_back(concatenateAbsolutePath(path, data_str));
	}

	_pclose(fp);
	return true;
}

std::string Util::findSuffix(std::vector<std::string>& filesList, std::string suffix, int num)
{
	for (std::vector<std::string>::iterator itr = filesList.begin(); itr != filesList.end(); ++itr)
	{
		if (Util::endsWith(*itr, suffix)) {
			if (num == 1)
			{
				return *itr;;
			}
			num--;
		}
	}
	return "";
}

void Util::findAllFilesWithSuffix(std::vector<std::string>& filesList, std::vector<std::string> foundFiles, std::string suffix)
{
	for (std::vector<std::string>::iterator itr = filesList.begin(); itr != filesList.end(); ++itr)
	{
		if (Util::endsWith(*itr, suffix)) {
			foundFiles.push_back(*itr);
		}
	}
}

//checks if value ends with ending
bool Util::endsWith(std::string& value, std::string& ending)
{
	if (ending.size() > value.size())
	{
		//basic check
		return false;
	}

	return equal(ending.rbegin(), ending.rend(), value.rbegin()); //std function for checking equality of strings.
}

std::vector<std::string> Util::split(const std::string& s, char delim)
{
	std::vector<std::string> elems;
	std::stringstream strSt(s);
	std::string item;

	while (std::getline(strSt, item, delim))
	{
		elems.push_back(item);
	}

	return elems;
}

bool Util::existsFiles(std::string& fileBoard, std::string& fileDllA, std::string& fileDllB,
	const std::string& path, bool isLegalBoard)
{
	bool validFiles = true;
	std::string dir;
	(path.compare("") == 0) ? dir = "Working Directory" : dir = path;

	if (fileBoard.compare("") == 0) {
		std::cout << "Missing board file (*.sboard) looking in path: " << dir << std::endl;
		validFiles = false;
	}
	if (fileDllA.compare("") == 0 || fileDllB.compare("") == 0)
	{
		std::cout << "Missing an algorithm (dll) file looking in path: " << dir << std::endl;
		validFiles = false;
	}

	return validFiles && isLegalBoard;
}

bool Util::setDefaultArgs(std::vector<std::string>& filesList, int& numOfThreads)
{
	std::string configFile = findSuffix(filesList, ".config", 1);
	if (configFile.compare("") == 0) {
		//std::cout << "Error: *.config file in missing " << boardPath << std::endl;
		return false;
	}
	std::string line;
	std::ifstream fin(configFile);

	if (!fin.is_open()) {
		//std::cout << "Error: Cannot open *.config file in " << boardPath << std::endl;
		return false;
	}
	while (getline(fin, line))
	{
		std::vector<std::string> tokens = split(line, ' ');
		if (tokens.size() < 2)
		{
			//std::cout << "Error: configuration file doesn't contain a default argument for number of threads " << boardPath << std::endl;
			return false;
		}
		try
		{
			int value = std::stoi(tokens.at(1));
			if (tokens.at(0).compare("-threads") == 0)
			{
				numOfThreads = value;
				return true;
			}
		}
		catch (std::invalid_argument)
		{
			// Using default value
		}
		catch (std::out_of_range)
		{
			// Using default value
		}
	}
	return false;
}

/*functions for mmapping Coordinate*/

std::string to_string(Coordinate c) {
	return "(" + std::to_string(c.col) + ", " + std::to_string(c.row) + ", " + std::to_string(c.depth) + ")";
}

std::ostream& operator<<(std::ostream& out, const Coordinate& c) {
	return out << to_string(c);
}

// required for unordered_map
bool operator==(const Coordinate& c1, const Coordinate& c2) {
	return c1.col == c2.col && c1.row == c2.row && c1.depth == c2.depth;
}

// required for map
bool operator<(const Coordinate& c1, const Coordinate& c2) {
	if (c1.row == c2.row) {
		if (c1.col == c2.col) {
			return c1.depth < c2.depth;
		}
		return c1.col < c2.col;
	}
	return c1.row < c2.row;
}


void Util::initMain(int argc, char* argv[], std::string& path, bool& quiet, int& delay)
{
	for (auto i = 1; i < argc; ++i)
	{

		if (!strcmp(argv[i], "-quiet"))
		{
			quiet = true;
		}
		else if (!strcmp(argv[i], "-delay"))
		{
			if ((i + 1) < argc)
			{
				try
				{
					delay = std::stoi(argv[i + 1]);
				}
				catch (std::invalid_argument)
				{
					// Using default value
				}
				catch (std::out_of_range)
				{
					// Using default value
				}
			}
		}
		else if (i == 1)
		{
			path = argv[i];
		}
	}

	path.erase(std::remove(path.begin(), path.end(), ' '), path.end());
	path = Util::findAbsPath(path.c_str());
}


