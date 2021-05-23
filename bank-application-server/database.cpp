#include "database.h"
#include "bank_exception.h"
#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

void Database::setup(const std::string& current_path)
{
	// Resolve database path
	setupPath(current_path);

	// Try opening / creating the database
	sqlite3* db = open();

	// Make sure tables exist
	if (!tableExists(db, "user")) {
		createUserTable(db);
	}
	if (!tableExists(db, "recurring_payment")) {
		createRecurringPaymentTable(db);
	}
	if (!tableExists(db, "account")) {
		createAccountTable(db);
	}
	if (!tableExists(db, "record")) {
		createRecordTable(db);
	}
	if (!tableExists(db, "previous")) {
		createPreviousTable(db);
	}

	// Free the database
	sqlite3_close(db);
}

void Database::setupPath(const std::string& current_path)
{
	fs::path p(current_path);
	p = p.parent_path().parent_path();
	p /= "db";
	p /= "bank_db";

	path = (p.parent_path().parent_path() / "db" / "bank.db").string();
}

void Database::createUserTable(sqlite3* db)
{
	char *zErrMsg = 0;
	std::string command = "CREATE TABLE user("  \
		"email			TEXT		NOT NULL	UNIQUE," \
		"password		TEXT		NOT NULL );";

	int error_code = sqlite3_exec(db, command.c_str(), NULL, 0, &zErrMsg);
	errorCheck(error_code, zErrMsg);
}

bool Database::tableExists(sqlite3* db, const std::string& table_name)
{
	char *zErrMsg = 0;
	std::string command = "SELECT name FROM sqlite_master " \
		"WHERE type='table' AND name=";
	command += "'" + table_name + "';";

	bool exists = false;
	bool* exists_ptr = &exists;

	int error_code = sqlite3_exec(db, command.c_str(), callbackTableExists, (void*) exists_ptr, &zErrMsg);
	errorCheck(error_code, zErrMsg);

	return *exists_ptr;
}

void Database::createRecurringPaymentTable(sqlite3* db)
{
	char *zErrMsg = 0;
	std::string command = "CREATE TABLE recurring_payment("  \
		"account_source	TEXT		NOT NULL," \
		"account_target	TEXT		NOT NULL," \
		"name_source	TEXT		NOT NULL," \
		"name_target	TEXT		NOT NULL," \
		"next_payment	TEXT		NOT NULL," \
		"amount			DOUBLE		NOT NULL," \
		"interval		INT			NOT NULL," \
		"type			INT			NOT NULL," \
		"UNIQUE(account_source, name_source));";

	int error_code = sqlite3_exec(db, command.c_str(), NULL, 0, &zErrMsg);
	errorCheck(error_code, zErrMsg);
}

void Database::createAccountTable(sqlite3* db)
{
	char *zErrMsg = 0;
	std::string command = "CREATE TABLE account("  \
		"name			TEXT		NOT NULL," \
		"email			TEXT		NOT NULL," \
		"balance		DOUBLE		NOT NULL," \
		"state			INT			NOT NULL," \
		"UNIQUE(email, name));";

	int error_code = sqlite3_exec(db, command.c_str(), NULL, 0, &zErrMsg);
	errorCheck(error_code, zErrMsg);
}

void Database::createRecordTable(sqlite3* db)
{
	char *zErrMsg = 0;
	std::string command = "CREATE TABLE record("  \
		"account_source		TEXT		NOT NULL," \
		"account_target		TEXT		NOT NULL," \
		"name_source		TEXT		NOT NULL," \
		"name_target		TEXT		NOT NULL," \
		"amount				DOUBLE		NOT NULL," \
		"date				TEXT		NOT NULL );";

	int error_code = sqlite3_exec(db, command.c_str(), NULL, 0, &zErrMsg);
	errorCheck(error_code, zErrMsg);
}

void Database::createPreviousTable(sqlite3* db)
{
	char *zErrMsg = 0;
	std::string command = "CREATE TABLE previous("  \
		"account_source		TEXT		NOT NULL," \
		"account_target		TEXT		NOT NULL," \
		"name_source		TEXT		NOT NULL," \
		"name_target		TEXT		NOT NULL," \
		"UNIQUE(account_source, account_target, name_source, name_target));";

	int error_code = sqlite3_exec(db, command.c_str(), NULL, 0, &zErrMsg);
	errorCheck(error_code, zErrMsg);
}

void Database::addUser(const User& user)
{
	int error_code = SQLITE_BUSY;
	char *zErrMsg = 0;

	sqlite3* db = open();

	std::string command = "INSERT INTO user (email,password) VALUES (";
	command += "'" + user.mail_ + "',";
	command += "'" + user.password_ + "');";

	int i = 0;
	while ((error_code == SQLITE_BUSY) && (i < TIMEOUT_ATEMPTS)) {
		error_code = sqlite3_exec(db, command.c_str(), NULL, 0, &zErrMsg);
		i++;
	}

	sqlite3_close(db);
	errorCheck(error_code, zErrMsg);
}

void Database::addAccount(const Account& account)
{
	int error_code = SQLITE_BUSY;
	char *zErrMsg = 0;

	sqlite3* db = open();

	int state;
	switch (account.state_) {
	case State::ok:
		state = DB_USER_OK;
		break;
	case State::blocked:
		state = DB_USER_BLOCKED;
		break;
	default:
		state = DB_USER_BLOCKED;
		break;
	}

	std::string command = "INSERT INTO account (name,email,balance,state) VALUES (";
	command += "'" + account.name_ + "',";
	command += "'" + account.mail_ + "',";
	command += std::to_string(account.balance_) + ",";
	command += std::to_string(state) + ");";

	int i = 0;
	while ((error_code == SQLITE_BUSY) && (i < TIMEOUT_ATEMPTS)) {
		error_code = sqlite3_exec(db, command.c_str(), NULL, 0, &zErrMsg);
		i++;
	}

	sqlite3_close(db);
	errorCheck(error_code, zErrMsg);
}

void Database::addRecord(const Record& record)
{
	int error_code = SQLITE_BUSY;
	char *zErrMsg = 0;

	sqlite3* db = open();

	std::string command = "INSERT INTO record ";
	command += "(account_source,account_target,name_source,name_target,amount,date) VALUES (";
	command += "'" + record.account_source_ + "',";
	command += "'" + record.account_target_ + "',";
	command += "'" + record.name_source_ + "',";
	command += "'" + record.name_target_ + "',";
	command += std::to_string(record.amount_) + ",";
	command += "'" + record.date_ + "');";

	int i = 0;
	while ((error_code == SQLITE_BUSY) && (i < TIMEOUT_ATEMPTS)) {
		error_code = sqlite3_exec(db, command.c_str(), NULL, 0, &zErrMsg);
		i++;
	}

	sqlite3_close(db);
	errorCheck(error_code, zErrMsg);
}

void Database::addPrevious(const user_pair& pair)
{
	int error_code = SQLITE_BUSY;
	char *zErrMsg = 0;

	sqlite3* db = open();

	std::string command = "INSERT INTO previous ";
	command += "(account_source,account_target,name_source,name_target) VALUES (";
	command += "'" + pair.email_source + "',";
	command += "'" + pair.email_target+ "',";
	command += "'" + pair.name_source + "',";
	command += "'" + pair.name_target + "');";

	int i = 0;
	while ((error_code == SQLITE_BUSY) && (i < TIMEOUT_ATEMPTS)) {
		error_code = sqlite3_exec(db, command.c_str(), NULL, 0, &zErrMsg);
		i++;
	}

	sqlite3_close(db);

	// This pair of users already has a record in the `previous` table
	if (error_code == SQLITE_CONSTRAINT) {
		return;
	}

	errorCheck(error_code, zErrMsg);
}

User Database::getUser(const std::string& email)
{
	int error_code = SQLITE_BUSY;
	char *zErrMsg = 0;

	sqlite3* db = open();

	std::string command = "SELECT * FROM user WHERE email = ";
	command += "'" + email + "';";

	User user{};

	// Fill password
	int i = 0;
	while ((error_code == SQLITE_BUSY) && (i < TIMEOUT_ATEMPTS)) {
		error_code = sqlite3_exec(db, command.c_str(), callbackGetUser, &user, &zErrMsg);
		i++;
	}

	// Fill rest of the fields
	if (error_code == SQLITE_OK) {
		error_code = SQLITE_BUSY;

		command = "SELECT * FROM account WHERE email = ";
		command += "'" + email + "';";

		i = 0;
		while ((error_code == SQLITE_BUSY) && (i < TIMEOUT_ATEMPTS)) {
			error_code = sqlite3_exec(db, command.c_str(), callbackFillAccounts, &user, &zErrMsg);
			i++;
		}
	}

	sqlite3_close(db);
	errorCheck(error_code, zErrMsg);

	return user;
}

RecurringPayment Database::getRecurringPayment(const std::string& email, const std::string& name)
{
	int error_code = SQLITE_BUSY;
	char *zErrMsg = 0;

	sqlite3* db = open();

	std::string command = "SELECT * FROM recurring_payment WHERE account_source = ";
	command += "'" + email + "'";
	command += " AND ";
	command += "name_source = ";
	command += "'" + name + "';";

	RecurringPayment rp{};

	int i = 0;
	while ((error_code == SQLITE_BUSY) && (i < TIMEOUT_ATEMPTS)) {
		error_code = sqlite3_exec(db, command.c_str(), callbackGetPayment, &rp, &zErrMsg);
		i++;
	}

	sqlite3_close(db);
	errorCheck(error_code, zErrMsg);

	return rp;
}

void Database::changeValue(const std::string& email, const std::string& column_name, const std::string& new_value)
{
	int error_code = SQLITE_BUSY;
	char* zErrMsg = 0;

	sqlite3* db = open();

	std::string command = "UPDATE user SET ";
	command += column_name + "=";
	command += new_value;
	command += " WHERE email=";
	command += "'" + email + "';";

	int i = 0;
	while ((error_code == SQLITE_BUSY) && (i < TIMEOUT_ATEMPTS)) {
		error_code = sqlite3_exec(db, command.c_str(), NULL, 0, &zErrMsg);
		i++;
	}

	sqlite3_close(db);
	errorCheck(error_code, zErrMsg);
}

void Database::changeValueAccount(const std::string& email, const std::string& account, const std::string& column_name, const std::string& new_value)
{
	int error_code = SQLITE_BUSY;
	char* zErrMsg = 0;

	sqlite3* db = open();

	std::string command = "UPDATE account SET ";
	command += column_name + "=";
	command += new_value;
	command += " WHERE email=";
	command += "'" + email + "'";
	command += " AND ";
	command += "name=";
	command += "'" + account + "';";

	int i = 0;
	while ((error_code == SQLITE_BUSY) && (i < TIMEOUT_ATEMPTS)) {
		error_code = sqlite3_exec(db, command.c_str(), NULL, 0, &zErrMsg);
		i++;
	}

	sqlite3_close(db);
	errorCheck(error_code, zErrMsg);
}

void Database::updateRecurringPayment(const std::string& account_source, const std::string& name_source, const std::string& new_value)
{
	int error_code = SQLITE_BUSY;
	char* zErrMsg = 0;

	sqlite3* db = open();

	std::string command = "UPDATE recurring_payment SET ";
	command += "next_payment=";
	command += "'" + new_value + "'";
	command += " WHERE account_source=";
	command += "'" + account_source + "'";
	command += " AND ";
	command += "name_source=";
	command += "'" + name_source + "';";

	int i = 0;
	while ((error_code == SQLITE_BUSY) && (i < TIMEOUT_ATEMPTS)) {
		error_code = sqlite3_exec(db, command.c_str(), NULL, 0, &zErrMsg);
		i++;
	}

	sqlite3_close(db);
	errorCheck(error_code, zErrMsg);
}

void Database::addRecurringPayment(const RecurringPayment& rp)
{
	int error_code = SQLITE_BUSY;
	char *zErrMsg = 0;

	sqlite3* db = open();

	int type;
	switch (rp.type_) {
	case PaymentType::standing_order:
		type = DB_PAYMENT_STANDING_ORDER;
		break;
	case PaymentType::direct_debit:
		type = DB_PAYMENT_DIRECT_DEBIT;
		break;
	default:
		type = DB_PAYMENT_STANDING_ORDER;
		break;
	}

	int interval;
	switch (rp.interval_) {
	case Interval::day:
		interval = DB_INTERVAL_DAY;
		break;
	case Interval::week:
		interval = DB_INTERVAL_WEEK;
		break;
	case Interval::month:
		interval = DB_INTERVAL_MONTH;
		break;
	case Interval::year:
		interval = DB_INTERVAL_YEAR;
		break;
	default:
		interval = DB_INTERVAL_DAY;
		break;
	}

	std::string command = "INSERT INTO recurring_payment ";
	command += "(account_source,account_target,name_source,name_target,next_payment,amount,interval,type) ";
	command += "VALUES (";
	command += "'" + rp.account_source_ + "',";
	command += "'" + rp.account_target_ + "',";
	command += "'" + rp.name_source_ + "',";
	command += "'" + rp.name_target_ + "',";
	command += "'" + rp.next_payment_ + "',";
	command += std::to_string(rp.amount_) + ",";
	command += std::to_string(interval) + ",";
	command += std::to_string(type) + ");";

	int i = 0;
	while ((error_code == SQLITE_BUSY) && (i < TIMEOUT_ATEMPTS)) {
		error_code = sqlite3_exec(db, command.c_str(), NULL, 0, &zErrMsg);
		i++;
	}

	sqlite3_close(db);
	errorCheck(error_code, zErrMsg);
}

void Database::gatherRecurringPayments(int(*callback)(void*, int, char**, char**), PaymentList* payments)
{
	int error_code = SQLITE_BUSY;
	char *zErrMsg = 0;

	sqlite3* db = open();
	std::string command = "SELECT * FROM recurring_payment;";

	int i = 0;
	while ((error_code == SQLITE_BUSY) && (i < TIMEOUT_ATEMPTS)) {
		error_code = sqlite3_exec(db, command.c_str(), callback, payments, &zErrMsg);
		i++;
	}

	sqlite3_close(db);
	errorCheck(error_code, zErrMsg);
}

void Database::gatherRecords(const std::string& email, const std::string& account, RecordList* records)
{
	int error_code = SQLITE_BUSY;
	char *zErrMsg = 0;

	sqlite3* db = open();

	std::string command = "SELECT * FROM record WHERE (account_source=";
	command += "'" + email + "'";
	command += " AND ";
	command += "name_source=";
	command += "'" + account + "')";
	command += " OR ";
	command += "(account_target=";
	command += "'" + email + "'";
	command += " AND ";
	command += "name_target=";
	command += "'" + account + "')";
	command += "ORDER BY date;";

	int i = 0;
	while ((error_code == SQLITE_BUSY) && (i < TIMEOUT_ATEMPTS)) {
		error_code = sqlite3_exec(db, command.c_str(), callbackGatherRecords, records, &zErrMsg);
		i++;
	}

	sqlite3_close(db);
	errorCheck(error_code, zErrMsg);
}

void Database::gatherPrevious(const std::string& email, const std::string& account, std::vector<user_pair>* pairs)
{
	int error_code = SQLITE_BUSY;
	char *zErrMsg = 0;

	sqlite3* db = open();

	std::string command = "SELECT * FROM previous WHERE account_source=";
	command += "'" + email + "'";
	command += " AND ";
	command += "name_source=";
	command += "'" + account + "';";

	int i = 0;
	while ((error_code == SQLITE_BUSY) && (i < TIMEOUT_ATEMPTS)) {
		error_code = sqlite3_exec(db, command.c_str(), callbackGatherPrevious, pairs, &zErrMsg);
		i++;
	}

	sqlite3_close(db);
	errorCheck(error_code, zErrMsg);
}

int Database::callbackTableExists(void* data, int argc, char** argv, char** azColName)
{
	// Because the callback is called only if a table was found, this is sufficient
	bool* kk = (bool*) data;
	*kk = true;
	return 0;
}

int Database::callbackGetUser(void* data, int argc, char** argv, char** azColName)
{
	User* user = (User*) data;
	user->correct_ = true;
   
	for(int i = 0; i < argc; i++){
		if (std::string(azColName[i]) == "email") {
			user->mail_ = argv[i];
		}
		else if (std::string(azColName[i]) == "password") {
			user->password_ = argv[i];
		}
	}
	return 0;
}

int Database::callbackFillAccounts(void* data, int argc, char** argv, char** azColName)
{
	User* user = (User*) data;
	Account account{};
   
	for(int i = 0; i < argc; i++){
		if (std::string(azColName[i]) == "email") {
			account.mail_ = argv[i];
		}
		else if (std::string(azColName[i]) == "name") {
			account.name_ = argv[i];
		}
		else if (std::string(azColName[i]) == "balance") {
			account.balance_ = std::stod(argv[i]); 
		}
		else if (std::string(azColName[i]) == "state") {
			if (std::stoi(argv[i]) == DB_USER_OK) {
				account.state_ = State::ok;
			}
			else if (std::stoi(argv[i]) == DB_USER_BLOCKED) {
				account.state_ = State::blocked;
			}
		}
	}
	user->accounts_.push_back(account);

	return 0;
}

int Database::callbackGetPayment(void* data, int argc, char** argv, char** azColName)
{
	RecurringPayment* rp = (RecurringPayment*) data;
	rp->correct_ = true;
   
	for(int i = 0; i < argc; i++){
		if (std::string(azColName[i]) == "account_source") {
			rp->account_source_ = argv[i];
		}
		else if (std::string(azColName[i]) == "account_target") {
			rp->account_target_ = argv[i];
		}
		else if (std::string(azColName[i]) == "name_source") {
			rp->name_source_ = argv[i];
		}
		else if (std::string(azColName[i]) == "name_target") {
			rp->name_target_ = argv[i];
		}
		else if (std::string(azColName[i]) == "next_payment") {
			rp->next_payment_ = argv[i];
		}
		else if (std::string(azColName[i]) == "amount") {
			rp->amount_ = std::stod(argv[i]);
		}
		else if (std::string(azColName[i]) == "interval") {
			if (std::stoi(argv[i]) == DB_INTERVAL_DAY) {
				rp->interval_ = Interval::day;
			}
			else if (std::stoi(argv[i]) == DB_INTERVAL_WEEK) {
				rp->interval_ = Interval::week;
			}
			else if (std::stoi(argv[i]) == DB_INTERVAL_MONTH) {
				rp->interval_ = Interval::month;
			}
			else if (std::stoi(argv[i]) == DB_INTERVAL_YEAR) {
				rp->interval_ = Interval::year;
			}
		}
		else if (std::string(azColName[i]) == "type") {
			if (std::stoi(argv[i]) == DB_PAYMENT_STANDING_ORDER) {
				rp->type_ = PaymentType::standing_order;
			}
			else if (std::stoi(argv[i]) == DB_PAYMENT_DIRECT_DEBIT) {
				rp->type_ = PaymentType::direct_debit;
			}
		}
	}

	return 0;
}

int Database::callbackGatherRecords(void* data, int argc, char** argv, char** azColName)
{
	RecordList* records = (RecordList*)data;
	records->push_back(std::make_unique<Record>());

	for(int i = 0; i < argc; i++){
		if (std::string(azColName[i]) == "account_source") {
			records->back()->account_source_ = argv[i];
		}
		else if (std::string(azColName[i]) == "account_target") {
			records->back()->account_target_ = argv[i];
		}
		else if (std::string(azColName[i]) == "name_source") {
			records->back()->name_source_ = argv[i];
		}
		else if (std::string(azColName[i]) == "name_target") {
			records->back()->name_target_ = argv[i];
		}
		else if (std::string(azColName[i]) == "amount") {
			records->back()->amount_ = std::stod(argv[i]);
		}
		else if (std::string(azColName[i]) == "date") {
			records->back()->date_ = argv[i];
		}
	}

	return 0;
}

int Database::callbackGatherPrevious(void* data, int argc, char** argv, char** azColName)
{
	std::vector<user_pair>* pairs = (std::vector<user_pair>*)data;
	user_pair pair{"", "", "", ""};

	for(int i = 0; i < argc; i++){
		if (std::string(azColName[i]) == "account_source") {
			pair.email_source = argv[i];
		}
		else if (std::string(azColName[i]) == "account_target") {
			pair.email_target = argv[i];
		}
		else if (std::string(azColName[i]) == "name_source") {
			pair.name_source = argv[i];
		}
		else if (std::string(azColName[i]) == "name_target") {
			pair.name_target = argv[i];
		}
	}

	pairs->push_back(pair);

	return 0;
}

sqlite3* Database::open()
{
	sqlite3* db;
	int error_code = sqlite3_open(path.c_str(), &db);
	if (error_code) {
		std::string message = "can't open database (";
		message += sqlite3_errmsg(db);
		message += ")";
		throw db_exception(message);
	}
	return db;
}

void Database::errorCheck(int error_code, char* zErrMsg)
{
	if (error_code != SQLITE_OK ) {
		std::string message(zErrMsg);
		sqlite3_free(zErrMsg);
		throw db_exception(message);
	}
}
