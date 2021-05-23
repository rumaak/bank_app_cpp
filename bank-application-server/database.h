#include <string>
#include "sqlite/sqlite3.h"
#include  <vector>
#include  <memory>
#include "dto.h"

#ifndef DATABASE_H_
#define DATABASE_H_

using PaymentList = std::vector<std::unique_ptr<RecurringPayment>>;
using RecordList = std::vector<std::unique_ptr<Record>>;

/**
 * Database management, issuing SQL queries for actions. This class aims
 * to simplify and make the database access more abstract. It wraps
 * SQLite queries as methods on this class, making it easier for
 * BankServer to access database.
 */
class Database {
public:
	/**
	 * Ensure the database exists and contains the neccessary tables.
	 * @param[in]	current_path	path to the executable
	 */
	void setup(const std::string& current_path);

	/**
	 * Add new user to database.
	 * @param[in]	user	User to be added
	 */
	void addUser(const User& user);

	/**
	 * Add new account to database.
	 * @param[in]	account	Account to be added
	 */
	void addAccount(const Account& account);

	/**
	 * Add new record to database.
	 * @param[in]	record	Record to be added
	 */
	void addRecord(const Record& record);

	/**
	 * Add new user pair to database.
	 * @param[in]	pair	User pair to be added
	 */
	void addPrevious(const user_pair& pair);

	/**
	 * Return existing user from the database.
	 * @param[in]	email	Email identifying the user
	 * returns				The user
	 */
	User getUser(const std::string& email);

	/**
	 * Return recurring payment from the database.
	 * @param[in]	email	Email identifying the recurring payment
	 * returns				The recurring payment
	 */
	RecurringPayment getRecurringPayment(const std::string& email, const std::string& name);

	/**
	 * Update a value in the user table.
	 * @param[in]	email		Email identifying the user
	 * @param[in]	column_name	Column containing the value
	 * @param[in]	new_value	New value
	 */
	void changeValue(const std::string& email, const std::string& column_name, 
		const std::string& new_value);

	/**
	 * Update a value in the account table.
	 * @param[in]	email		Email identifying the user
	 * @param[in]	account		Name identifying the account
	 * @param[in]	column_name	Column containing the value
	 * @param[in]	new_value	New value
	 */
	void changeValueAccount(const std::string& email, const std::string& account, 
		const std::string& column_name, const std::string& new_value);

	/**
	 * Update the next payment date of recurring payment in the recurring_payment table.
	 * @param[in]	account_source	Email identifying the payment
	 * @param[in]	new_value		New date
	 */
	void updateRecurringPayment(const std::string& account_source, const std::string& name_source, const std::string& new_value);

	/**
	 * Add new recurring payment into the database.
	 * @param[in]	rp	Payment to be added to database
	 */
	void addRecurringPayment(const RecurringPayment& rp);

	/**
	 * List all the recurring payments in database.
	 * @param[in]	callback	A callback responsible for adding a recurring payment into the list
	 * @param[out]	payments	A list of of recurring payments	
	 */
	void gatherRecurringPayments(int (*callback)(void*,int,char**,char**), PaymentList* payments);

	/**
	 * List all the records in database corresponding to given account.
	 * @param[in]	email		User email
	 * @param[in]	account		Account name
	 * @param[out]	records		A list of of records
	 */
	void gatherRecords(const std::string& email, const std::string& account, RecordList* records);

	/**
	 * List all accounts an account has interacted with (as source, not target).
	 * @param[in]	email		User email
	 * @param[in]	account		Account name
	 * @param[out]	pairs		A list of of user pairs (this account, account it interacted with)
	 */
	void gatherPrevious(const std::string& email, const std::string& account, std::vector<user_pair>* pairs);

	// Integer representation of user state
	static const int DB_USER_OK = 0;
	static const int DB_USER_BLOCKED = 1;

	// Integer representation of recurring payment type
	static const int DB_PAYMENT_STANDING_ORDER = 0;
	static const int DB_PAYMENT_DIRECT_DEBIT = 1;

	// Integer representation of recurring payment interval
	static const  int DB_INTERVAL_DAY = 0;
	static const  int DB_INTERVAL_WEEK = 1;
	static const  int DB_INTERVAL_MONTH = 2;
	static const  int DB_INTERVAL_YEAR = 3;

private:
	// Path to the database file
	std::string path = "";

	// An exception is thrown if the same SQL query fails too many times because the database is busy 
	const int TIMEOUT_ATEMPTS = 5;

	/**
	 * Sets the appropriate directory where the database file ought to reside.
	 * @param[in]	current_path	Path to the executable
	 */
	void setupPath(const std::string& current_path);

	/**
	 * Check whether table is present in the database.
	 * @param[in]	db			Database
	 * @param[in]	table_name	Name of the table
	 */
	bool tableExists(sqlite3* db, const std::string& table_name);

	/**
	 * Create user table in database.
	 * @param[in]	db			Database
	 */
	void createUserTable(sqlite3* db);

	/**
	 * Create recurring payment table in database.
	 * @param[in]	db			Database
	 */
	void createRecurringPaymentTable(sqlite3* db);

	/**
	 * Create account table in database.
	 * @param[in]	db			Database
	 */
	void createAccountTable(sqlite3* db);

	/**
	 * Create record table in database.
	 * @param[in]	db			Database
	 */
	void createRecordTable(sqlite3* db);

	/**
	 * Create previous table in database.
	 * @param[in]	db			Database
	 */
	void createPreviousTable(sqlite3* db);

	/**
	 * Database query callback, sets boolean to true if table exists.
	 * @param[out]	data		Boolean indicating existence of table
	 */
	static int callbackTableExists(void* data, int argc, char** argv, char** azColName);

	/**
	 * Database query callback, creates a user corresponding to user in database (if exists).
	 * @param[out]	data		User object to fill with database data
	 * @param[in]	argc		Number of columns
	 * @param[in]	argv		Values in columns
	 * @param[in]	azColName	Column names
	 */
	static int callbackGetUser(void* data, int argc, char** argv, char** azColName);

	/**
	 * Database query callback, creates an account object for each account in database.
	 * @param[out]	data		Pointer to user object (whose accounts we want)
	 * @param[in]	argc		Number of columns
	 * @param[in]	argv		Values in columns
	 * @param[in]	azColName	Column names
	 */
	static int callbackFillAccounts(void* data, int argc, char** argv, char** azColName);

	/**
	 * Database query callback, creates a payment corresponding to payment in database (if exists).
	 * @para[out]	data		Recurring payment object to fill with database data
	 * @para[in]	argc		Number of columns
	 * @para[in]	argv		Values in columns
	 * @para[in]	azColName	Column names
	 */
	static int callbackGetPayment(void* data, int argc, char** argv, char** azColName);

	/**
	 * Database query callback, creates a record object for each record in database.
	 * @param[out]	data		Pointer to (initially empty) vector of records
	 * @param[in]	argc		Number of columns
	 * @param[in]	argv		Values in columns
	 * @param[in]	azColName	Column names
	 */
	static int callbackGatherRecords(void* data, int argc, char** argv, char** azColName);
	
	/**
	 * Database query callback, creates a previous object for each 'previous' in database.
	 * @param[out]	data		Pointer to (initially empty) vector of user pairs
	 * @param[in]	argc		Number of columns
	 * @param[in]	argv		Values in columns
	 * @param[in]	azColName	Column names
	 */
	static int callbackGatherPrevious(void* data, int argc, char** argv, char** azColName);

	/**
	 * Open a database connection.
	 * @returns		Database connection
	 */
	sqlite3* open();

	/**
	 * Throw an exception if an error occured during execution of SQL query.
	 * @param[in]	error_code	Error type (or indication that no error occured)
	 * @param[in]	zErrMsg		Detailed error message
	 */
	void errorCheck(int error_code, char* zErrMsg);
};

#endif

