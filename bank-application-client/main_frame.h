#include "wx/wx.h"
#include "dialog.h"
#include "account.h"
#include <map>

#ifndef MAIN_FRAME_H_
#define MAIN_FRAME_H_

class AccountFrame;
class MainFrame;

/**
* Scrollable panel for viewing accounts.
*/
class AccountsPane : public wxScrolledWindow
{
public:
	/**
	 * Scrollable panel constructor.
	 * @param[in]	parent		Parent UI control
	 * @param[in]	id			Unique identification
	 */
	AccountsPane(wxWindow* parent, wxWindowID id);

	/**
	 * Update the list of accounts using data from MainFrame.
	 * @param[in]	main_frame	MainFrame with accounts member
	 */
	void updateAccounts(MainFrame* main_frame);

	// resolution of 'open' button for given account
	std::map<int, std::string> button_id_to_name;

private:
	// UI constant
	const int OFFSET = 200;

	// sizer responsible for structure of individual accounts in panel
	wxBoxSizer* sizer = nullptr;

	/**
	 * Create a single account box.
	 * @param[in]	name		Account name
	 * @param[in]	amount		Account balance
	 * @param[in]	state		Account state
	 * @param[in]	main_frame	MainFrame with the action handler
	 * @returns					Account panel
	 */
	wxPanel* createAccountPanel(const std::string& name, double amount, const std::string& state, MainFrame* main_frame);

	/**
	 * Create a single row in the account box. A row consists of description of
	 * what is displayed (name) and the value itself (value)
	 * @param[in]	parent			Account box to which we are adding the row
	 * @param[in]	parent_sizer	Sizer to which we will add the row
	 * @param[in]	name			What does the value represent
	 * @param[in]	value			The value to be displayed
	 */
	void addAccountPanelRow(wxPanel* parent, wxBoxSizer* parent_sizer, const std::string& name, const std::string& value);

	/**
	 * String representation of double with two decimals.
	 * @param[in]	amount		The number to be formatted
	 * @returns					Formated number
	 */
	std::string format_amount(double amount);
};

/**
* Frame listing accounts of user.
*/
class MainFrame : public wxFrame
{
public:
	/**
	 * Constructor sets up the UI and prompts the user to log in / register.
	 */
	MainFrame();

	// whether login / registration was successful
	bool init_success = false;

	// a pointer to currently opened account window
	AccountFrame* open_window = nullptr;

	// UI controls
	AccountsPane* pane;
	wxButton* new_account_button = nullptr;

	/**
	 * Request an updated list of accounts from server
	 */
	void updateData();

	/**
	 * Handler for the new_account_button. Prompts the user with
	 * AddAccountDialog and updates the accounts afterward.
	 * @param[in]	evt		Event
	 */
	void onNewAccountButtonClicked(wxCommandEvent& evt);

	/**
	 * Handler for accounts button. First the account name is resolved
	 * using the button id, then a new window with selected account
	 * is opened (AccountFrame).
	 * @param[in]	evt		Event
	 */
	void onOpenButtonClick(wxCommandEvent& evt);

	/**
	 * Handler called when this frame is closed. If there is an
	 * account window open, it notifies it that this window is being
	 * closed.
	 * @param[in]	evt		Event
	 */
	void onClose(wxCloseEvent& evt);

	// data corresponding to given user
	std::string email;
	std::vector<account> accounts;

private:
	// action id corresponding to the action of listing accounts
	const std::string LIST_ACCOUNTS_ID = "10";

	/**
	 * Prompt the user to log in / register, then fill the email and accounts
	 * members with data from server.
	 */
	void loginRegister();

	/**
	 * Update displayed accounts using data stored in dialog.
	 * @param[in]	dlg		Dialog with accounts data
	 */
	void loadResponseData(AccountListDialog* dlg);

	wxDECLARE_EVENT_TABLE();
};

#endif
