#include <stdexcept>

#ifndef	BANK_EXCEPTION_H_
#define BANK_EXCEPTION_H_

/**
 * Database-related exception.
 */
class db_exception : public std::exception {
public:
	db_exception(const std::string& message) : s_("SQL error: ") { s_ += message; };
	virtual const char* what() const override { return s_.c_str(); };
private:
	std::string s_;
};

/**
 * Mail-related exception.
 */
class mail_exception : public std::exception {
public:
	mail_exception(const std::string& message) : s_("Mail client error: ") { s_ += message; };
	virtual const char* what() const override { return s_.c_str(); };
private:
	std::string s_;
};

#endif

