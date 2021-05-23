#include "bank_server.h"
#include <iostream>
#include <chrono>
#include <iomanip>
#include <memory>
#include "bank_exception.h"
#include "dto.h"

#define ASIO_STANDALONE
#include <asio.hpp>

using tcp = asio::ip::tcp;
using PaymentList = std::vector<std::unique_ptr<RecurringPayment>>;

const double BankServer::BLOCK_LIMIT = -10'000;

void BankServer::recurringPaymentExecute(std::atomic<bool>& done, const std::string& current_path)
{
	using namespace std::chrono_literals;
	try {
		Database database{};
		database.setup(current_path);

		for (;;) {
			PaymentList recurring_payments;
			PaymentList* payments_ptr = &recurring_payments;

			// Fill recurring_payments with recurring payment records in database
			database.gatherRecurringPayments(recurringPaymentsCallback, payments_ptr);

			// Process each recurring payment
			for (auto&& payment_unique : recurring_payments) {
				RecurringPayment payment = *payment_unique;
				BankServer::processRecurringPayment(payment, database);
			}

			std::this_thread::sleep_for(24h);
		}
		done = true;
	}
	catch (std::exception& e) {
		std::cerr << e.what() << std::endl;
		done = true;
	}
	catch (...) {
		done = true;
	}
}

int BankServer::recurringPaymentsCallback(void* data, int argc, char** argv, char** azColName)
{
	PaymentList* payments = (PaymentList*)data;
	payments->push_back(std::make_unique<RecurringPayment>());

	for(int i = 0; i < argc; i++){
		if (std::string(azColName[i]) == "account_source") {
			payments->back()->account_source_ = argv[i];
		}
		else if (std::string(azColName[i]) == "account_target") {
			payments->back()->account_target_ = argv[i];
		}
		else if (std::string(azColName[i]) == "name_source") {
			payments->back()->name_source_ = argv[i];
		}
		else if (std::string(azColName[i]) == "name_target") {
			payments->back()->name_target_ = argv[i];
		}
		else if (std::string(azColName[i]) == "next_payment") {
			payments->back()->next_payment_ = argv[i];
		}
		else if (std::string(azColName[i]) == "amount") {
			payments->back()->amount_ = std::stod(argv[i]);
		}
		else if (std::string(azColName[i]) == "interval") {
			switch (std::stoi(argv[i])) {
			case Database::DB_INTERVAL_DAY:
				payments->back()->interval_ = Interval::day;
				break;
			case Database::DB_INTERVAL_WEEK:
				payments->back()->interval_ = Interval::week;
				break;
			case Database::DB_INTERVAL_MONTH:
				payments->back()->interval_ = Interval::month;
				break;
			case Database::DB_INTERVAL_YEAR:
				payments->back()->interval_ = Interval::year;
				break;
			}
		}
		else if (std::string(azColName[i]) == "type") {
			switch (std::stoi(argv[i])) {
			case Database::DB_PAYMENT_STANDING_ORDER:
				payments->back()->type_ = PaymentType::standing_order;
				break;
			case Database::DB_PAYMENT_DIRECT_DEBIT:
				payments->back()->type_ = PaymentType::direct_debit;
				break;
			}
		}
	}

	return 0;
}

void BankServer::run(const std::string& current_path)
{
	database.setup(current_path);

	// Start a second thread responsible for periodical check of recurring payments
    std::atomic<bool> done(false);
	std::thread thread(recurringPaymentExecute, std::ref(done), current_path);
	
	asio::io_context io_context;
	tcp::endpoint endpoint = tcp::endpoint(tcp::v4(), 13);
	tcp::acceptor acceptor(io_context, endpoint);

	try {
		for (;;)
		{
			if (done) {
				throw std::exception("Secondary thread has exited");
			}

			tcp::socket socket(io_context);
			acceptor.accept(socket);
			
			std::string incoming = "";
			readIncoming(socket, incoming);

			std::string message = "";
			createResponseMessage(incoming, message);

			asio::write(socket, asio::buffer(message));
		}
	}
	catch (std::exception& e) {
		std::cerr << e.what() << std::endl;
		std::cerr << "Waiting for second thread to stop" << std::endl;

		acceptor.close();
		thread.join();
	}
	catch (...) {
		std::cerr << "An error occured, waiting for second thread to stop" << std::endl;

		acceptor.close();
		thread.join();
	}

	std::cerr << "Server has stopped" << std::endl;
}

void BankServer::readIncoming(tcp::socket& socket, std::string& incoming)
{
	for (;;) {
		char buf[128] = {};
		size_t len = socket.read_some(asio::buffer(buf));

		// Read until terminal character '\n' is reached
		incoming.append(buf);
		if (incoming.at(incoming.size()-1) == '\n') {
			break;
		}
	}
}

void BankServer::createResponseMessage(std::string& incoming, std::string& message)
{
	int id = std::stoi(incoming.substr(0, 2));
	incoming = incoming.substr(2);

	switch (id) {

	// Log in
	case 1:
		login(incoming, message);
		break;

	// Register
	case 2:
	{
		registration(incoming, message);
		break;
	}
	// Transfer TO
	case 3:
		transfer(incoming, message, "TO");
		break;

	// Transfer FROM
	case 4:
		transfer(incoming, message, "FROM");
		break;

	// Direct debit
	case 5:
		recurringPayment(incoming, message, PaymentType::direct_debit);
		break;

	// Standing order 
	case 6:
		recurringPayment(incoming, message, PaymentType::standing_order);
		break;

	// Add money 
	case 7:
		addMoney(incoming, message);
		break;

	// Add account
	case 8:
		addAccount(incoming, message);
		break;

	// Get transactions of given account
	case 9:
		transactionHistory(incoming, message);
		break;

	// Get user accounts
	case 10:
		listAccounts(incoming, message);
		break;

	// Get previously selected target users
	case 11:
		previousTargets(incoming, message);
		break;

	default:
		message += REJECTED;
	}
	
	message += END;
}

void BankServer::registration(std::string& incoming, std::string& message)
{
	std::string mail = "";
	std::string passwd = "";
	double balance = 0;

	fillField(incoming, mail, SEPARATOR);
	fillField(incoming, passwd, END);

	User user = database.getUser(mail);
	if (!user.correct_) {
		std::vector<Account> accounts;
		user = User(mail, passwd, accounts);
		database.addUser(user);

		message += ACCEPTED;
		message += mail + SEPARATOR;
		messageAccounts(user, message);
	}
	else {
		message += REJECTED;
		message += "User already exists";
	}
}

void BankServer::login(std::string& incoming, std::string& message)
{
	std::string mail = "";
	std::string passwd = "";

	fillField(incoming, mail, SEPARATOR);
	fillField(incoming, passwd, END);

	User user = database.getUser(mail);
	if (user.correct_) {
		if (user.password_ == passwd) {
			message += ACCEPTED;
			message += user.mail_ + SEPARATOR;
			messageAccounts(user, message);
		}
		else {
			message += REJECTED;
			message += "Wrong password";
		}
	}
	else {
		message += REJECTED;
		message += "User does not exist";
	}
}

void BankServer::addMoney(std::string& incoming, std::string& message)
{
	std::string email = "";
	std::string account = "";
	std::string amount_str = "";

	fillField(incoming, email, SEPARATOR);
	fillField(incoming, account, SEPARATOR);
	fillField(incoming, amount_str, END);

	double amount = std::stod(amount_str);

	// Ensure account exists, retrieve information about it
	User user = database.getUser(email);
	Account acc{};
	bool found = false;
	for (auto&& a : user.accounts_) {
		if (a.name_ == account) {
			found = true;
			acc = a;
		}
	}

	if (found) {
		double new_balance = acc.balance_ + amount;

		// if new balance of an account would change it's state, change the state and inform user
		if ((new_balance >= BLOCK_LIMIT) && (acc.state_ == State::blocked)) {
			database.changeValueAccount(user.mail_, acc.name_, "state", std::to_string(Database::DB_USER_OK));
			acc.state_ = State::ok;
			emailStateChanged(user.mail_, acc.name_, acc.state_);
		}

		database.changeValueAccount(user.mail_, acc.name_, "balance", std::to_string(new_balance));

		Record record{"-", user.mail_, "-", acc.name_, amount, currentDate()};
		database.addRecord(record);

		message += ACCEPTED;
		message += user.mail_ + SEPARATOR;
		message += acc.name_ + SEPARATOR;
		message += std::to_string(new_balance) + SEPARATOR;
		message += getStateString(acc.state_);
	}
	else {
		message += REJECTED;
		message += "Account does not exist";
	}
}

void BankServer::transfer(std::string& incoming, std::string& message, const std::string& direction)
{
	std::string current_email = "";
	std::string selected_email = "";
	std::string current_acc = "";
	std::string selected_acc = "";
	std::string amount_str = "";

	// swap current and selected accounts if needed
	if (direction == "TO") {
		fillField(incoming, current_email, SEPARATOR);
		fillField(incoming, selected_email, SEPARATOR);
		fillField(incoming, current_acc, SEPARATOR);
		fillField(incoming, selected_acc, SEPARATOR);
	}
	else {
		fillField(incoming, selected_email, SEPARATOR);
		fillField(incoming, current_email, SEPARATOR);
		fillField(incoming, selected_acc, SEPARATOR);
		fillField(incoming, current_acc, SEPARATOR);
	}
	fillField(incoming, amount_str, END);

	double amount = std::stod(amount_str);

	User user_current = database.getUser(current_email);
	User user_selected = database.getUser(selected_email);
	
	Account acc_current{};
	Account acc_selected{};

	// Get source account from database
	bool current_exists = false;
	for (auto&& acc : user_current.accounts_) {
		if (acc.name_ == current_acc) {
			current_exists = true;
			acc_current = acc;
		}
	}

	// Get target account from database
	bool selected_exists = false;
	for (auto&& acc : user_selected.accounts_) {
		if (acc.name_ == selected_acc) {
			selected_exists = true;
			acc_selected = acc;
		}
	}

	if (!current_exists || !selected_exists) {
		message += REJECTED;
		message += "One of the accounts does not exist";
		return;
	}

	if (user_selected.correct_ && user_current.correct_) {
		if (acc_current.state_ == State::ok) {

			// In order to transfer from someone he needs to have a direct debit set up
			if (direction == "FROM") {
				RecurringPayment rp = database.getRecurringPayment(user_current.mail_, acc_current.name_);

				if (rp.correct_ && (rp.type_ == PaymentType::direct_debit)) {
					std::time_t next_payment = timeFromString(rp.next_payment_);
					std::time_t now = std::time(nullptr);
					double time_diff = std::difftime(next_payment, now);

					if (time_diff <= 0 && rp.amount_ >= amount) {
						tm* next_payment_new = localtime(&next_payment);
						addInterval(rp, next_payment_new);

						std::string next_payment_str = stringFromTime(next_payment_new);
						database.updateRecurringPayment(user_current.mail_, acc_current.name_, next_payment_str);
					}
					else {
						message += REJECTED;
						message += "The direct debit has already been spent or it is too low";
						return;
					}
				}
				else {
					message += REJECTED;
					message += "No direct debit present";
					return;
				}
			}

			double user_current_new_balance = acc_current.balance_ - amount;
			double user_selected_new_balance = acc_selected.balance_ + amount;

			// if new balance of an account would change it's state, change the state and inform user
			if ((user_current_new_balance < BLOCK_LIMIT) && (acc_current.state_ == State::ok)) {
				database.changeValueAccount(user_current.mail_, acc_current.name_, "state", 
					std::to_string(Database::DB_USER_BLOCKED));
				acc_current.state_ = State::blocked;
				emailStateChanged(user_current.mail_, acc_current.name_, acc_current.state_);
			}

			// if new balance of an account would change it's state, change the state and inform user
			if ((user_selected_new_balance >= BLOCK_LIMIT) && (acc_selected.state_ == State::blocked)) {
				database.changeValueAccount(user_selected.mail_, acc_selected.name_, "state", 
					std::to_string(Database::DB_USER_OK));
				acc_selected.state_ = State::ok;
				emailStateChanged(user_selected.mail_, acc_selected.name_, acc_selected.state_);
			}

			database.changeValueAccount(user_current.mail_, acc_current.name_, "balance", std::to_string(user_current_new_balance));
			database.changeValueAccount(user_selected.mail_, acc_selected.name_, "balance", std::to_string(user_selected_new_balance));

			Record record{user_current.mail_, user_selected.mail_, acc_current.name_, acc_selected.name_, amount, currentDate()};
			database.addRecord(record);

			if (direction == "TO") {
				user_pair pair = {
					acc_current.mail_, // email_source
					acc_selected.mail_, // email_target
					acc_current.name_, // name_source
					acc_selected.name_ // name_target
				};

				database.addPrevious(pair);
				
				message += ACCEPTED;
				message += user_current.mail_ + SEPARATOR;
				message += acc_current.name_ + SEPARATOR;
				message += std::to_string(user_current_new_balance) + SEPARATOR;
				message += getStateString(acc_current.state_);
			}
			else {
				user_pair pair = {
					acc_selected.mail_, // email_source
					acc_current.mail_, // email_target
					acc_selected.name_, // name_source
					acc_current.name_ // name_target
				};
				database.addPrevious(pair);
			
				message += ACCEPTED;
				message += user_selected.mail_ + SEPARATOR;
				message += acc_selected.name_ + SEPARATOR;
				message += std::to_string(user_selected_new_balance) + SEPARATOR;
				message += getStateString(acc_selected.state_);
			}
		}
		else {
			message += REJECTED;
			message += "Payers account is blocked";
		}
	}
	else {
		message += REJECTED;
		message += "One of the accounts doesn't exist";
	}
}

void BankServer::recurringPayment(std::string& incoming, std::string& message, PaymentType pt)
{
	std::string email_source = "";
	std::string email_target = "";
	std::string acc_source = "";
	std::string acc_target = "";
	std::string amount_str = "";
	std::string next_payment = "";
	std::string interval_str = "";

	fillField(incoming, email_source, SEPARATOR);
	fillField(incoming, email_target, SEPARATOR);
	fillField(incoming, acc_source, SEPARATOR);
	fillField(incoming, acc_target, SEPARATOR);
	fillField(incoming, amount_str, SEPARATOR);
	fillField(incoming, next_payment, SEPARATOR);
	fillField(incoming, interval_str, END);

	Interval interval;
	bool correct = true;
	int interval_int = std::stoi(interval_str);
	switch (interval_int) {
	case 0:
		interval = Interval::day;
		break;
	case 1:
		interval = Interval::week;
		break;
	case 2:
		interval = Interval::month;
		break;
	case 3:
		interval = Interval::year;
		break;
	default:
		correct = false;
		break;
	}

	if (correct) {
		RecurringPayment existing = database.getRecurringPayment(email_source, acc_source);

		if (!existing.correct_) {
			RecurringPayment rp{ email_source, email_target, acc_source, acc_target, next_payment, std::stod(amount_str),
				interval, pt };

			database.addRecurringPayment(rp);

			user_pair pair = {
				email_source, // email_source
				email_target, // email_target
				acc_source, // name_source
				acc_target // name_target
			};
			database.addPrevious(pair);

			message += ACCEPTED;
		}
		else {
			message += REJECTED;
			message += "Number of recurring payments for user exceeded";
		}
	}
	else {
		message += REJECTED;
		message += "Internal error: unknown time interval";
	}
}

void BankServer::listAccounts(std::string& incoming, std::string& message)
{
	std::string mail = "";

	fillField(incoming, mail, END);

	User user = database.getUser(mail);
	if (user.correct_) {
		message += ACCEPTED;
		message += user.mail_ + SEPARATOR;
		messageAccounts(user, message);
	}
	else {
		message += REJECTED;
		message += "User does not exist";
	}
}

void BankServer::addAccount(std::string& incoming, std::string& message)
{
	std::string mail = "";
	std::string name = "";

	double balance = 0;

	fillField(incoming, mail, SEPARATOR);
	fillField(incoming, name, END);

	User user = database.getUser(mail);
	if (user.correct_) {
		bool acc_exists = false;
		for (auto&& acc : user.accounts_) {
			if (acc.name_ == name) {
				acc_exists = true;
			}
		}

		if (!acc_exists) {
			Account acc = Account(mail, name, balance, State::ok);
			database.addAccount(acc);
			user.accounts_.push_back(acc);

			message += ACCEPTED;
			message += mail + SEPARATOR;
			messageAccounts(user, message);
		}
		else {
			message += REJECTED;
			message += "Account already exists";
		}
	}
	else {
		message += REJECTED;
		message += "User doesn't exist";
	}
}

void BankServer::transactionHistory(std::string& incoming, std::string& message)
{
	std::string mail = "";
	std::string name = "";
	std::string date_from = "";
	std::string date_to = "";

	fillField(incoming, mail, SEPARATOR);
	fillField(incoming, name, SEPARATOR);
	fillField(incoming, date_from, SEPARATOR);
	fillField(incoming, date_to, END);

	RecordList records;
	RecordList* records_ptr = &records;
	database.gatherRecords(mail, name, records_ptr);

	std::string result = "";
	int count = 0;
	for (auto&& record_unique : records) {
		Record record = *record_unique;

		std::time_t date_from_t = timeFromString(date_from);
		std::time_t date_to_t = timeFromString(date_to);
		std::time_t record_t = timeFromString(record.date_);

		double rec_diff_from = std::difftime(record_t, date_from_t);
		double to_diff_rec = std::difftime(date_to_t, record_t);

		// Inclusive on both ends
		if ((rec_diff_from >= 0) && (to_diff_rec >= 0)) {
			if (count > 0) {
				result += SEPARATOR;
			}

			count += 1;
			result += record.account_source_ + SEPARATOR;
			result += record.account_target_ + SEPARATOR;
			result += record.name_source_ + SEPARATOR;
			result += record.name_target_ + SEPARATOR;
			result += std::to_string(record.amount_) + SEPARATOR;
			result += record.date_;
		}
	}

	message += ACCEPTED;
	message += std::to_string(count) + SEPARATOR;
	message += result;
}

void BankServer::previousTargets(std::string& incoming, std::string& message)
{
	std::string mail = "";
	std::string name = "";

	fillField(incoming, mail, SEPARATOR);
	fillField(incoming, name, END);

	std::vector<user_pair> pairs;
	std::vector<user_pair>* pairs_ptr = &pairs;
	database.gatherPrevious(mail, name, pairs_ptr);

	std::string result = "";
	int count = 0;
	for (auto&& pair : pairs) {
		if (count > 0) {
			result += SEPARATOR;
		}

		result += pair.email_source + SEPARATOR;
		result += pair.email_target + SEPARATOR;
		result += pair.name_source + SEPARATOR;
		result += pair.name_target;

		count += 1;
	}

	message += ACCEPTED;
	message += std::to_string(count) + SEPARATOR;
	message += result;
}

void BankServer::messageAccounts(const User& user, std::string& message)
{
	int count = user.accounts_.size();
	message += std::to_string(count) + SEPARATOR;

	int i = 1;
	for (auto&& acc : user.accounts_) {
		message += acc.name_ + SEPARATOR;
		message += std::to_string(acc.balance_) + SEPARATOR;
		message += getStateString(acc.state_);

		if (i != count) {
			message += SEPARATOR;
		}

		i++;
	}
}

void BankServer::fillField(std::string& source, std::string& field, const char separator)
{
	int i = 0;
	for (;;) {
		if (source[i] == separator) {
			break;
		}
		field += source[i];
		i++;
	}
	source.erase(0, i+1);
}

std::string BankServer::getStateString(State state)
{
	switch (state) {
	case State::ok:
		return "OK";
		break;
	case State::blocked:
		return "BLOCKED";
		break;
	default:
		return "BLOCKED";
		break;
	}
}

std::time_t BankServer::timeFromString(const std::string& time_str)
{
	struct std::tm tm = {0};
	std::istringstream ss(time_str);
	// %F doesn't work for some reason
	ss >> std::get_time(&tm, "%Y-%m-%d");	
	std::time_t time = mktime(&tm);
	return time;
}

std::string BankServer::stringFromTime(std::tm* time)
{
	std::time_t time_time_t = mktime(time);
	tm* time_corrected = localtime(&time_time_t);

	char buffer[32];
	std::strftime(buffer, 32, "%Y-%m-%d", time_corrected);
	std::string time_str(buffer);
	return time_str;
}

void BankServer::processRecurringPayment(const RecurringPayment& rp, Database& database)
{
	std::time_t next_payment = BankServer::timeFromString(rp.next_payment_);
	std::time_t now = std::time(nullptr);

	// We don't care about payments that are in future
	double time_diff = std::difftime(next_payment, now);
	if (time_diff <= 0) {
		if (rp.type_ == PaymentType::standing_order) {
			User user_current = database.getUser(rp.account_source_);
			User user_selected = database.getUser(rp.account_target_);

			Account acc_current{};
			Account acc_selected{};

			bool current_exists = false;
			for (auto&& acc : user_current.accounts_) {
				if (acc.name_ == rp.name_source_) {
					current_exists = true;
					acc_current = acc;
				}
			}

			bool selected_exists = false;
			for (auto&& acc : user_selected.accounts_) {
				if (acc.name_ == rp.name_target_) {
					selected_exists = true;
					acc_selected = acc;
				}
			}

			// Improper recurring payment
			if (!current_exists || !selected_exists) {
				return;
			}
			
			if (user_selected.correct_ && user_current.correct_) {
				if (acc_current.state_ == State::ok) {
					double user_current_new_balance = acc_current.balance_ - rp.amount_;
					double user_selected_new_balance = acc_selected.balance_ + rp.amount_;

					// if new balance of an account would change it's state, change the state and inform user
					if ((user_current_new_balance < BLOCK_LIMIT) && (acc_current.state_ == State::ok)) {
						database.changeValueAccount(user_current.mail_, acc_current.name_, "state", 
							std::to_string(Database::DB_USER_BLOCKED));
						acc_current.state_ = State::blocked;
						emailStateChanged(user_current.mail_, acc_current.name_, acc_current.state_);
					}

					// if new balance of an account would change it's state, change the state and inform user
					if ((user_selected_new_balance >= BLOCK_LIMIT) && (acc_selected.state_ == State::blocked)) {
						database.changeValueAccount(user_selected.mail_, acc_selected.name_, "state", 
							std::to_string(Database::DB_USER_OK));
						acc_selected.state_ = State::ok;
						emailStateChanged(user_selected.mail_, acc_selected.name_, acc_selected.state_);
					}

					database.changeValueAccount(user_current.mail_, acc_current.name_, "balance", std::to_string(user_current_new_balance));
					database.changeValueAccount(user_selected.mail_, acc_selected.name_, "balance", std::to_string(user_selected_new_balance));

					Record record{user_current.mail_, user_selected.mail_, acc_current.name_, acc_selected.name_, rp.amount_, currentDate()};
					database.addRecord(record);
				}
			}

			tm* next_payment_new = localtime(&next_payment);
			addInterval(rp, next_payment_new);

			std::string next_payment_str = stringFromTime(next_payment_new);
			database.updateRecurringPayment(user_current.mail_, acc_current.name_, next_payment_str);

		}
		else if (rp.type_ == PaymentType::direct_debit) {
			tm* next_payment_new = localtime(&next_payment);
			addInterval(rp, next_payment_new);
			std::time_t next_payment_new_corrected = mktime(next_payment_new);
			time_diff = std::difftime(next_payment_new_corrected, now);

			// This direct debit wasn't used in the time of the interval, so we update next payment
			if (time_diff <= 0) {
				std::string next_payment_str = stringFromTime(next_payment_new);
				database.updateRecurringPayment(rp.account_source_, rp.name_source_, next_payment_str);
			}
		}
	}
}

void BankServer::addInterval(const RecurringPayment& payment, tm* time)
{
	switch (payment.interval_)
	{
	case Interval::day:
		time->tm_mday = time->tm_mday + 1;
		break;
	case Interval::week:
		time->tm_mday = time->tm_mday + 7;
		break;
	case Interval::month:
		time->tm_mon = time->tm_mon + 1;
		break;
	case Interval::year:
		time->tm_year = time->tm_year + 1;
		break;
	default:
		break;
	}
}

void BankServer::emailStateChanged(const std::string& email, const std::string& name, State state)
{
	std::string state_str = "";
	if (state == State::blocked) {
		state_str += "blocked";
	}
	else {
		state_str += "ok";
	}

	std::string message = "";
	message += "Dear user,\n";
	message += "The status of your account " + name + " has changed to: " + state_str + "\n";

	// Failing to send an email isn't a reason to drop the whole server
	MailClient mail_client{};
	try {
		mail_client.sendEmail("<" + email + ">", message);
	}
	catch (mail_exception& e) {
		std::cerr << e.what() << std::endl;
	}
}

std::string BankServer::currentDate()
{
	std::time_t now = std::time(nullptr);
	tm* now_tm = localtime(&now);
	return stringFromTime(now_tm);
}
