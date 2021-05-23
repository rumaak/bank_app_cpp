#include <string>
#include <vector>

#ifndef	DTO_H_
#define	DTO_H_

enum class State {
	ok, blocked
};

/**
 * Class representing an account.
 */
class Account {
public:
	/**
	 * A "proper" constructor.
	 * 
	 * @param[in]	mail		Email of account owner
	 * @param[in]	name		Name of the account
	 * @param[in]	balance		Balance of an account
	 * @param[in]	state		State of the account (ok / blocked)
	 */
	Account(std::string mail, std::string name, double balance, State state)
		: mail_(mail), name_(name), balance_(balance), state_(state) {};

	/**
	 * An "improper" constructor (it is expected that fields will be manually filled later).
	 */
	Account() : mail_(""), name_(""), balance_(0), state_(State::ok) {};

	// User data
	std::string mail_;
	std::string name_;
	double balance_;
	State state_;
};

/**
 * Class representing a transaction.
 */
class Record {
public:
	/**
	 * A "proper" constructor.
	 * 
	 * @param[in]	account_source	Source account email
	 * @param[in]	account_target	Target account email
	 * @param[in]	name_source		Source account name
	 * @param[in]	name_target		Target account name
	 * @param[in]	amount			Amount associated with the transaction
	 * @param[in]	date			When was the transaction executed
	 */
	Record(const std::string& account_source, const std::string& account_target, const std::string& name_source, 
		const std::string& name_target, double amount, const std::string& date)
		: account_source_(account_source), account_target_(account_target), name_source_(name_source),
		  name_target_(name_target), amount_(amount), date_(date) {};

	/**
	 * An "improper" constructor (it is expected that fields will be manually filled later).
	 */
	Record() : account_source_(""), account_target_(""), name_source_(""), name_target_(""), amount_(0), 
		date_("") {};

	std::string account_source_;
	std::string account_target_;
	std::string name_source_;
	std::string name_target_;
	double amount_;
	std::string date_;
};

enum class PaymentType {
	direct_debit, standing_order
};

enum class Interval {
	day, week, month, year
};

/**
 * Class representing a recurring payment.
 */
class RecurringPayment {
public:
	/**
	 * A "proper" constructor creating a valid recurring payment.
	 * 
	 * @param[in]	account_source	Source account email
	 * @param[in]	account_target	Target account email
	 * @param[in]	name_source		Source account name
	 * @param[in]	name_target		Target account name
	 * @param[in]	next_payment	The date after which the payment may be executed
	 * @param[in]	amount			The amount of money the recurring payment can transfer per period
	 * @param[in]	interval		How often the payment may be executed
	 * @param[in]	type			Payment type (direct debit / standing order)
	 */
	RecurringPayment(const std::string& account_source, const std::string& account_target,
		const std::string& name_source, const std::string& name_target, const std::string& next_payment, 
		double amount, Interval interval, PaymentType type) 
		: account_source_(account_source), account_target_(account_target), name_source_(name_source), 
		  name_target_(name_target), next_payment_(next_payment), amount_(amount), interval_(interval), 
		  type_(type), correct_(true) {};

	/**
	 * An "improper" constructor creating an incomplete recurring payment.
	 */
	RecurringPayment() : account_source_(""), account_target_(""), name_source_(""), name_target_(""), 
		next_payment_(""), amount_(0), interval_(Interval::day), type_(PaymentType::standing_order),
		correct_(false) {};

	// Recurring payment data
	std::string account_source_;
	std::string account_target_;
	std::string name_source_;
	std::string name_target_;
	std::string next_payment_;
	double amount_;
	bool correct_;
	Interval interval_;
	PaymentType type_;
};

/**
 * Class representing a user.
 */
class User {
public:
	/**
	 * A "proper" constructor creating a valid user.
	 * 
	 * @param[in]	mail		User email
	 * @param[in]	password	User password
	 * @param[in]	accounts	Vector of accounts associated with given user
	 */
	User(std::string mail, std::string password, const std::vector<Account>& accounts)
		: mail_(mail), password_(password), correct_(true), accounts_(accounts) {};

	/**
	 * An "improper" constructor creating an incomplete user.
	 */
	User() : mail_(""), password_(""), correct_(false), accounts_() {};

	// User data
	std::string mail_;
	std::string password_;
	std::vector<Account> accounts_;
	bool correct_;
};

/**
 * Struct for storing pairs of accounts.
 */
struct user_pair {
	std::string email_source;
	std::string email_target;
	std::string name_source;
	std::string name_target;
};

#endif

