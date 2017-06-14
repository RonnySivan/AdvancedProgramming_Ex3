#include "Logger.h"

const std::string CLogger::m_sFileName = "game.log";
CLogger* CLogger::m_pThis = nullptr;
std::ofstream CLogger::m_Logfile;
CLogger::CLogger()
{

}

CLogger* CLogger::GetLogger() {
	if (m_pThis == nullptr) {
		m_pThis = new CLogger();
		m_Logfile.open(m_sFileName.c_str(), std::ios::app);
	}
	return m_pThis;
}

void CLogger::CloseLogger() {
	if (m_pThis != nullptr) {
		m_Logfile.close();
		m_pThis = nullptr;
	}
}


void CLogger::Log(const char * format, ...)
{
	char* sMessage = nullptr;
	auto nLength = 0;
	va_list args;
	va_start(args, format);
	//  Return the number of characters in the string referenced the list of arguments.
	// _vscprintf doesn't count terminating '\0' (that's why +1)
	nLength = _vscprintf(format, args) + 1;
	sMessage = new char[nLength]; // TODO - try to avoid using "new"
	vsprintf_s(sMessage, nLength, format, args);
	//vsprintf(sMessage, format, args);
	m_Logfile << Util::CurrentDateTime() << ":\t";
	m_Logfile << sMessage << "\n";
	va_end(args);

	delete[] sMessage;
}

void CLogger::Log(const std::string& sMessage)
{
	m_Logfile << Util::CurrentDateTime() << ":\t";
	m_Logfile << sMessage << "\n";
}

CLogger& CLogger::operator<<(const std::string& sMessage)
{
	m_Logfile << "\n" << Util::CurrentDateTime() << ":\t";
	m_Logfile << sMessage << "\n";
	return *this;
}