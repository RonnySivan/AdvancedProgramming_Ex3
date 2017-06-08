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
	{
		std::cout << "Wrong path: " << ((path.compare("") == 0) ? "Working Directory" : path) << std::endl;
		return false;
	}

	if (system("2>NUL cd ../")) ///TODO - can remove this? 
	{
		std::cout << "Wrong path: " << ((path.compare("") == 0) ? "Working Directory" : path) << std::endl;
		return false;
	}

	std::string dir_command = "2>NUL dir /a-d /b \"" + fullPath + "\"";
	FILE* fp = _popen(dir_command.c_str(), "r");
	while (fgets(buffer, 4095, fp))
	{
		std::string  data_str = std::string(buffer);
		data_str.resize(data_str.size() - 1); //fgets reads newline
		filesList.push_back(concatenateAbsolutePath(path, data_str));
	}

	_pclose(fp);

	if (!Util::findBoardAndDlls(path, filesList))
		return false;

	return true;
}

bool Util::findBoardAndDlls(const std::string& path, std::vector<std::string>& filesList)
{
	std::string fileBoard = Util::findSuffix(filesList, ".sboard", 1);
	std::string fileDll1 = Util::findSuffix(filesList, ".dll", 1);
	std::string fileDll2 = Util::findSuffix(filesList, ".dll", 2);

	if (fileBoard.compare("") == 0) {
		std::cout << "No board files (*.sboard) looking in path: " << path << std::endl;
		return false;
	}

	if (fileDll1.compare("") == 0 || fileDll2.compare("") == 0)
	{
		std::cout << "Missing algorithm (dll) files looking in path: " << path << " (needs at least two)" << std::endl;
		return false;
	}

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

void Util::findAllFilesWithSuffix(std::vector<std::string>& filesList, std::vector<std::string> &foundFiles, std::string suffix)
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


bool Util::setDefaultArgs(std::vector<std::string>& filesList, int& numOfThreads)
{
	std::string configFile = findSuffix(filesList, ".config", 1);
	if (configFile.compare("") == 0) {
		//std::cout << "Error: *.config file in missing " << boardPath << std::endl; //TODO write to logger
		return false;
	}
	std::string line;
	std::ifstream fin(configFile);

	if (!fin.is_open()) {
		//std::cout << "Error: Cannot open *.config file in " << boardPath << std::endl; //TODO write to logger
		return false;
	}
	while (getline(fin, line))
	{
		std::vector<std::string> tokens = split(line, ' ');
		if (tokens.size() < 2)
		{
			//std::cout << "Error: configuration file doesn't contain a default argument for number of threads " << boardPath << std::endl;
			//TODO write to logger
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

// required for map and set
bool operator<(const Coordinate& c1, const Coordinate& c2) {
	if (c1.row == c2.row) {
		if (c1.col == c2.col) {
			return c1.depth < c2.depth;
		}
		return c1.col < c2.col;
	}
	return c1.row < c2.row;
}


void Util::initMain(int argc, char* argv[], std::string& path, int& threads)
{

	for (auto i = 1; i < argc; ++i)
	{
		if (!strcmp(argv[i], "-threads"))
		{
			if ((i + 1) < argc)
			{
				try
				{
					threads = std::stoi(argv[i + 1]);
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


std::string Util::CurrentDateTime()
{
	auto     now = time(nullptr);
	struct tm  tstruct;
	char       buf[80];
	localtime_s(&tstruct, &now);
	strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);
	return buf;
}


