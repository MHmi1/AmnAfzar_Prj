

#ifndef _EXCEPTION_
#define _EXCEPTION_
#include <exception>
#include <string>

class inputException : public std::exception //special class for input exceptions checking
{
private:
	std::string m_error{"invalid command : you must precede one of this patterns  : \n client id_number \n dest_id message src_id \n "}; 

public:
	inputException(std::string_view error)
		: m_error{error} {}

	const char *what() const noexcept override { return m_error.c_str(); }
};

#endif