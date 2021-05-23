#include <string>
#include <wx/wx.h>
#include <wx/datectrl.h>
#include "account.h"
#include "transaction.h"

#ifndef DIALOG_H_
#define DIALOG_H_

/**
* A dialog which stores account information (which is then retrieved by
* calling AccountFrame).
*/
class AccountInfoDialog {
public:
	/**
	 * Trivial constructor.
	 */
	AccountInfoDialog() : name(""), balance(0), status("") {};

	// Data members
	std::string name;
	double balance;
	std::string status;

protected:
	/**
	 * Fills the dialog members with data from server response.
	 * @param[in]	response	String with server response
	 */
	void fillResponseFields(std::string& response);
};

/**
* A dialog which stores accounts retrieved from server (which
* are then retrieved by calling MainFrame).
*/
class AccountListDialog {
public:
	/**
	 * Trivial constructor.
	 */
	AccountListDialog() : email(""), accounts() {};

	// Data members
	std::string email;
	std::vector<account> accounts;

protected:
	/**
	 * Fills the dialog members with data from server response.
	 * @param[in]	response	String with server response
	 */
	void fillResponseFields(std::string& response);
};

/**
* A dialog responsible for login and registration.
*/
class LoginRegisterDialog : public wxDialog, public AccountListDialog
{
public:
	/**
	 * Constructor, sets up UI and assigns handlers.
	 * @param[in]	parent	Parent UI control
	 * @param[in]	id		Unique id
	 * @param[in]	title	Dialog title
	 * @param[in]	pos		Dialog position
	 * @param[in]	size	Dialog size
	 * @param[in]	style	Dialog style
	 */
	LoginRegisterDialog ( wxWindow * parent, wxWindowID id, const wxString & title,
						  const wxPoint & pos = wxDefaultPosition,
						  const wxSize & size = wxDefaultSize,
						  long style = wxDEFAULT_DIALOG_STYLE );

	// Dialog fields, buttons and labels
	wxStaticText* login_name = nullptr;
	wxTextCtrl* login_name_field = nullptr;
	wxStaticText* login_pswd = nullptr;
	wxTextCtrl* login_pswd_field = nullptr;
	wxButton* button_login = nullptr;
	wxButton* button_register = nullptr;

	/**
	 * Log in a user. Wrapper around loginRegister method.
	 * @param[in]	evt	Event
	 */
	void onLoginButtonClicked(wxCommandEvent& evt);

	/**
	 * Register a user. Wrapper around loginRegister method.
	 * @param[in]	evt	Event
	 */
	void onRegisterButtonClicked(wxCommandEvent& evt);

	DECLARE_EVENT_TABLE()

private:
	// Command IDs
	const std::string LOGIN_ID = "01";
	const std::string REGISTER_ID = "02";

	/**
	 * Send login / register request to server, process response, continue / error.
	 * @param[in]	id		Type of action (login / registration)
	 */
	void loginRegister(const std::string& id);
};


/**
* A dialog responsible for adding a new user account (don't mistake with user registration).
*/
class AddAccountDialog : public wxDialog, public AccountListDialog
{
public:
	/**
	 * Constructor, sets up UI and assigns handlers.
	 * @param[in]	email	User email
	 * @param[in]	parent	Parent UI control
	 * @param[in]	id		Unique id
	 * @param[in]	title	Dialog title
	 * @param[in]	pos		Dialog position
	 * @param[in]	size	Dialog size
	 * @param[in]	style	Dialog style
	 */
	AddAccountDialog(const std::string& email, wxWindow* parent, wxWindowID id, 
				   const wxString& title, const wxPoint& pos = wxDefaultPosition,
				   const wxSize& size = wxDefaultSize,
				   long style = wxDEFAULT_DIALOG_STYLE);

	// Dialog fields, buttons and labels
	wxTextCtrl* name_field = nullptr;
	wxButton* submit_button = nullptr;

	/**
	 * Request an account creation (with specified name) from server.
	 * @param[in]	evt	Event
	 */
	void onSubmitButtonClicked(wxCommandEvent& evt);

	DECLARE_EVENT_TABLE()

private:
	// Command ID
	const std::string ACTION_ID = "08";

	// Email of current user
	std::string current_account;
};

/**
* A dialog responsible for transfer of money.
*/
class TransferDialog : public wxDialog, public AccountInfoDialog
{
public:
	/**
	 * Constructor sets up the dialog window.
	 * @param[in]	email	Transfer source email
	 * @param[in]	account	Transfer source account name
	 * @param[in]	parent	Parent window
	 * @param[in]	id		Window id
	 * @param[in]	title	Window title
	 * @param[in]	pos		Window position
	 * @param[in]	size	Window size
	 * @param[in]	style	Window style
	 */
	TransferDialog(const std::string& email, const std::string& name, wxWindow* parent, 
				   wxWindowID id, const wxString& title,
				   const wxPoint& pos = wxDefaultPosition,
				   const wxSize& size = wxDefaultSize,
				   long style = wxDEFAULT_DIALOG_STYLE);

	// Dialog fields, buttons and labels
	wxTextCtrl* email_field = nullptr;
	wxTextCtrl* name_field = nullptr;
	wxTextCtrl* amount_field = nullptr;
	wxButton* submit_button = nullptr;
	wxRadioBox* to_from_rb = nullptr;

	/**
	 * Check the validity of amount, delegate to transfer()
	 * @param[in]	evt	Event
	 */
	void onSubmitButtonClicked(wxCommandEvent& evt);

	DECLARE_EVENT_TABLE()

private:
	// Command IDs
	const std::string TO_ID = "03";
	const std::string FROM_ID = "04";
	const std::string SUGGEST_ID = "11";

	// Current user email and account name
	std::string current_email;
	std::string current_name;

	// Suggested target email / account
	wxArrayString suggested_emails;
	wxArrayString suggested_names;

	/**
	 * Send transfer request to server, process response, continue / error
	 * @param[in]	id		Type of action (transfer to / transfer from)
	 */
	void transfer(const std::string& id);

	/**
	 * Request previously used targets from server. On success, call
	 * fillSuggestions method.
	 */
	void setupSuggestions();

	/**
	 * Fill members suggested_emails and suggested_names, that are then used
	 * as corresponding suggestions.
	 * @param[in]	response	Server response with suggestions
	 */
	void fillSuggestions(std::string& response);
};

/**
* A dialog for standing order and direct debit setup.
*/
class RecurringPaymentDialog : public wxDialog
{
public:
	/**
	 * Constructor sets up the dialog window.
	 * @param[in]	email	Recurring payment source user email
	 * @param[in]	account	Recurring payment source account name
	 * @param[in]	type	Type of recurring payment (standing order / direct debit)
	 * @param[in]	parent	Parent window
	 * @param[in]	id		Window id
	 * @param[in]	title	Window title
	 * @param[in]	pos		Window position
	 * @param[in]	size	Window size
	 * @param[in]	style	Window style
	 */
	RecurringPaymentDialog(const std::string& email, const std::string& name,
						   const std::string& type, wxWindow* parent, 
						   wxWindowID id, const wxString& title, 
						   const wxPoint& pos = wxDefaultPosition,
						   const wxSize& size = wxDefaultSize,
						   long style = wxDEFAULT_DIALOG_STYLE);

	// Dialog fields, buttons and labels
	wxTextCtrl* email_field = nullptr;
	wxTextCtrl* name_field = nullptr;
	wxTextCtrl* amount_field = nullptr;
	wxButton* submit_button = nullptr;
	wxDatePickerCtrl* startson_datepicker = nullptr;
	wxRadioBox* interval_rb = nullptr;

	/**
	 * Check the validity of amount, delegate to transfer()
	 * @param[in]	evt	Event
	 */
	void onSubmitButtonClicked(wxCommandEvent& evt);

	DECLARE_EVENT_TABLE()

private:
	// Command IDs
	const std::string DIRECT_DEBIT_ID = "05";
	const std::string STANDING_ORDER_ID = "06";

	/**
	 * Send transfer request to server, process response, continue / error
	 * @param[in]	id		Type of action (direct debit / standing order)
	 */
	void transfer(const std::string& id);

	// Type of recurring payment, transaction source account
	std::string action_type;
	std::string current_email;
	std::string current_name;
};

/**
* A dialog to add money to currently used account.
*/
class AddMoneyDialog : public wxDialog, public AccountInfoDialog
{
public:
	/**
	 * Constructor sets up the dialog window.
	 * @param[in]	email	Email of user the money will be added to
	 * @param[in]	account	Account name the money will be added to
	 * @param[in]	parent	Parent window
	 * @param[in]	id		Window id
	 * @param[in]	title	Window title
	 * @param[in]	pos		Window position
	 * @param[in]	size	Window size
	 * @param[in]	style	Window style
	 */
	AddMoneyDialog(const std::string& email, const std::string& name, wxWindow* parent, 
				   wxWindowID id, const wxString& title, const wxPoint& pos = wxDefaultPosition,
				   const wxSize& size = wxDefaultSize,
				   long style = wxDEFAULT_DIALOG_STYLE);

	// Dialog fields, buttons and labels
	wxTextCtrl* amount_field = nullptr;
	wxButton* submit_button = nullptr;

	/**
	 * Check amount validity, send transfer request to server, process response, continue / error
	 * @param[in]	evt	Event
	 */
	void onSubmitButtonClicked(wxCommandEvent& evt);

	DECLARE_EVENT_TABLE()

private:
	// Command ID
	const std::string ACTION_ID = "07";

	// Current user email and account name
	std::string current_email;
	std::string current_name;
};

class TransactionsPane;

/**
* A dialog that shows the history of transactions for given account.
*/
class TransactionHistoryDialog : public wxDialog
{
public:
	/**
	 * Constructor sets up the dialog window.
	 * @param[in]	email	User email
	 * @param[in]	account	Account name
	 * @param[in]	parent	Parent window
	 * @param[in]	id		Window id
	 * @param[in]	title	Window title
	 * @param[in]	pos		Window position
	 * @param[in]	size	Window size
	 * @param[in]	style	Window style
	 */
	TransactionHistoryDialog(const std::string& email, const std::string& name, wxWindow* parent, 
				   wxWindowID id, const wxString& title,
				   const wxPoint& pos = wxDefaultPosition,
				   const wxSize& size = wxDefaultSize,
				   long style = wxDEFAULT_DIALOG_STYLE);

	// Dialog fields, buttons, datepickers
	wxDatePickerCtrl* since_datepicker = nullptr;
	wxDatePickerCtrl* until_datepicker = nullptr;
	wxButton* submit_button = nullptr;
	TransactionsPane* transactions = nullptr;
	
	/**
	 * Send request to server, process response, show the data
	 * @param[in]	evt	Event
	 */
	void onSubmitButtonClicked(wxCommandEvent& evt);

	DECLARE_EVENT_TABLE()

private:
	// Command ID
	const std::string ACTION_ID = "09";

	// Current user email and account name
	std::string current_email;
	std::string current_name;

	/**
	 * Create the left panel of the window, fill with controls, setup
	 * handlers. 
	 * @returns			The left panel
	 */
	wxPanel* leftPanelSetup();

	/**
	 * Create the right panel of the window, fill with controls, setup
	 * handlers. 
	 * @returns			The right panel
	 */
	wxPanel* rightPanelSetup();

	/**
	 * Create a vector of transactions using data from server.
	 * @param[in]	response	Server response
	 * @returns					The vector of transactions
	 */
	std::vector<transaction> extractAccounts(std::string& response);
};

/**
* Scrollable panel for viewing transactions.
*/
class TransactionsPane : public wxScrolledWindow
{
public:
	/**
	 * Scrollable panel constructor.
	 * @param[in]	parent		Parent UI control
	 * @param[in]	id			Unique identification
	 */
	TransactionsPane(wxWindow* parent, wxWindowID id);

	/**
	 * Create a row for each transaction displaying information about the
	 * transaction.
	 * @param[in]	transaction		The transactions
	 */
	void fillTransactions(const std::vector<transaction>& transactions);

private:
	// sizer responsible for structure of individual accounts in panel
	wxBoxSizer* sizer = nullptr;

	/**
	 * Create a single transaction row
	 * @param[in]	t	The transaction to display
	 * @returns			Row summarizing the transaction
	 */
	wxPanel* createTransactionPanel(transaction t);
};

#endif
