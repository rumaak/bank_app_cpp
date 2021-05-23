#include <SDKDDKVer.h>

#define ASIO_STANDALONE
#include <asio.hpp>
#include "database.h"
#include "mail_client.h"

#ifndef BANK_SERVER_H_
#define BANK_SERVER_H_

using tcp = asio::ip::tcp;

/**
 * Server class responsible for handling communication, processing requests and delegating them 
 * to database. This class defines what database operations are executed for each possible
 * request and how the execution output is then presented to client.
 */
class BankServer
{
public:
	/**
	 * Empty database object; setup is done in `run` method.
	 */
	BankServer() : database() {};

	/**
	 * Periodically execute recurring payments (if neccessary). Queries the database for all
	 * recurring payments, checks whether they are due, and if yes, executes them.
	 * @param[in]	current_path	Path to the executable
	 * @param[out]	done			True if an exception occured and the thread has terminated
	 */
	static void recurringPaymentExecute(std::atomic<bool>& done, const std::string& current_path);

	/**
	 * Database query callback, creates a recurring payment object for each recurring payment in database.
	 * @param[out]	data		List of recurring payment unique pointers
	 * @param[in]	argc		Number of columns
	 * @param[in]	argv		Values in columns
	 * @param[in]	azColName	Column names
	 */
	static int recurringPaymentsCallback(void* data, int argc, char** argv, char** azColName);

	/**
	 * Setup database, issue periodical payment checks and listen for requests. There are two threads
	 * running (second one spawned in `run` method): secondary thread that periodically executes due
	 * recurring payments (direct debit, standing order) and primary thread, that listens on TCP port
	 * for incoming requests. Requests are then forwarded to `createResponseMessage`.
	 * @param[in]	current_path	Path to the executable
	 */
	void run(const std::string& current_path);

	/**
	 * Create time_t object from string representation.
	 * @param[in]	time_str	String representation of time
	 * @returns					Corresponding time_t object
	 */
	static std::time_t timeFromString(const std::string& time_str);

	/**
	 * Create string representing time from tm object.
	 * @param[in]	time	Time object
	 * @returns				String representation of time
	 */
	static std::string stringFromTime(std::tm* time);

	/**
	 * Execute recurring payment and update the next date of execution if it is due.
	 * @param[in]	payment		Payment to be (potentially) executed
	 * @param[in]	database	Bank database
	 */
	static void processRecurringPayment(const RecurringPayment& payment, Database& database);

	/**
	 * Add the recurring payment interval to the time object.
	 * @param[in]	payment		Payment with interval
	 * @param[out]	time		Time to be extended by interval
	 */
	static void addInterval(const RecurringPayment& payment, tm* time);

	/**
	 * Send email to user with given email address informing him his account state has changed.
	 * @param[in]	email	Email address of user
	 * @param[in]	state	New state of his account
	 */
	static void emailStateChanged(const std::string& email, const std::string& name, State state);

	/**
	 * Get current date.
	 * @returns				String representation of current date
	 */
	static std::string currentDate();

	// Account with less money than BLOCK_LIMIT will have its state changed to blocked
	static const double BLOCK_LIMIT;

private:
	// Network communication constants
	static const char SEPARATOR = ';';
	static const char END = '\n';
	const std::string ACCEPTED = "SUC";
	const std::string REJECTED = "ERR";

	// Object for database manipulation
	Database database;

	/**
	 * Read data from socket and write them to string.
	 * @param[in]	socket		Socket for network communication with client
	 * @param[out]	incoming	String to store the request
	 */
	void readIncoming(tcp::socket& socket, std::string& incoming);

	/**
	 * Serve the request and create response.
	 * @param[in]	incoming	The request
	 * @param[out]	message		The response
	 */
	void createResponseMessage(std::string& incoming, std::string& message);

	/**
	 * Load (and delete) all characters until separator from source string into field string.
	 * @param[in]	source		Input string
	 * @param[out]	field		Output string
	 * @param[in]	separator	Separator 
	 */
	void fillField(std::string& source, std::string& field, const char separator);

	/**
	 * Get string representation of state.
	 * @param[in]	state		The state
	 * returns					String representation
	 */
	std::string getStateString(State state);

	/**
	 * Create and log in new user. Attempts to add new user to the database
	 * and informs client whether it was successful.
	 * @param[in]	incoming	Request
	 * @param[out]	message		Response
	 */
	void registration(std::string& incoming, std::string& message);

	/**
	 * Log in user. Check whether given user exists and whether the
	 * password is correct.
	 * @param[in]	incoming	Request
	 * @param[out]	message		Response
	 */
	void login(std::string& incoming, std::string& message);

	/**
	 * Add funds to the user's account. Increases the accounts
	 * balance by specified amount.
	 * @param[in]	incoming	Request
	 * @param[out]	message		Response
	 */
	void addMoney(std::string& incoming, std::string& message);

	/**
	 * Money transfer between accounts. If all the criteria are met
	 * (eg accounts exists) the balance of the source account is lowered by
	 * given amount and the balance of the target account is increased by
	 * the same amount.
	 * @param[in]	incoming	Request
	 * @param[out]	message		Response
	 * @param[in]	direction	Direction of transfer (from user / to user)
	 */
	void transfer(std::string& incoming, std::string& message, const std::string& direction);

	/**
	 * Create a recurring payment.
	 * @param[in]	incoming	Request
	 * @param[out]	message		Response
	 * @param[in]	tp			Type of payment (Direct debit / standing order)
	 */
	void recurringPayment(std::string& incoming, std::string& message, PaymentType pt);

	/**
	 * Send client a list of accounts corresponding to given email.
	 * @param[in]	incoming	Request
	 * @param[out]	message		Response
	 */
	void listAccounts(std::string& incoming, std::string& message);

	/**
	 * Create a new account for user with given name.
	 * @param[in]	incoming	Request
	 * @param[out]	message		Response
	 */
	void addAccount(std::string& incoming, std::string& message);

	/**
	 * Send client a sorted sequence of transactions such that the user
	 * was either payer or payee.
	 * @param[in]	incoming	Request
	 * @param[out]	message		Response
	 */
	void transactionHistory(std::string& incoming, std::string& message);

	/**
	 * Send client a sequence of accounts (email + name) such that the user
	 * has sent a transaction to these accounts before.
	 * @param[in]	incoming	Request
	 * @param[out]	message		Response
	 */
	void previousTargets(std::string& incoming, std::string& message);

	/**
	 * Append message with sequence of accounts corresponding to given
	 * user.
	 * @param[in]	user		User whose accounts we would like to know
	 * @param[out]	message		String augmented by said accounts
	 */
	void messageAccounts(const User& user, std::string& message);
};

#endif

