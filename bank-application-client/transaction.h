#include <string>

#ifndef TRANSACTION_H_
#define TRANSACTION_H_

/**
* A struct holding data associated with a transaction.
*/
struct transaction {
	std::string date;
	std::string email_from;
	std::string email_to;
	std::string name_from;
	std::string name_to;
	double amount;
};

#endif
