#include "wx/wx.h"
#include "connection_manager.h"
#include "dialog.h"
#include "account.h"
#include "main_frame.h"

#ifndef ACCOUNT_FRAME_H_ 
#define ACCOUNT_FRAME_H_

/**
* Frame displaying user account information.
*/
class AccountFrame : public wxFrame
{
public:
	/**
	 * Constructor sets up the UI and loads the data from calling
	 * MainFrame object.
	 * @param[in]	acc		User account being displayed
	 * @param[in]	mf		Calling MainFrame object
	 */
	AccountFrame(account acc, MainFrame* mf);

	// Info
	wxTextCtrl* name_control = nullptr;
	wxTextCtrl* balance_control = nullptr;
	wxTextCtrl* state_control = nullptr;

	// Actions
	wxButton* transfer_button = nullptr;
	wxButton* direct_debit_button = nullptr;
	wxButton* standing_order_button = nullptr;
	wxButton* add_money_button = nullptr;
	wxButton* history_button = nullptr;

	// Calling MainFrame
	MainFrame* main_frame = nullptr;

	/**
	 * Handler for transfer button. A TransferDialog is created
	 * and shown, after closing it the info in this frame is updated.
	 * @param[in]	evt		Event
	 */
	void onTransferButtonClicked(wxCommandEvent& evt);

	/**
	 * Handler for direct debit button. A RecurringPaymentDialog is
	 * created and shown.
	 * @param[in]	evt		Event
	 */
	void onDirectDebitButtonClicked(wxCommandEvent& evt);

	/**
	 * Handler for standing order button. A RecurringPaymentDialog is
	 * created and shown.
	 * @param[in]	evt		Event
	 */
	void onStandingOrderButtonClicked(wxCommandEvent& evt);

	/**
	 * Handler for add money button. An AddMoneyDialog is created and
	 * shown, after closing it the info in this frame is updated.
	 * @param[in]	evt		Event
	 */
	void onAddMoneyButtonClicked(wxCommandEvent& evt);

	/**
	 * Handler for transaction history button. A TransactionHistoryDialog
	 * is created and shown.
	 * @param[in]	evt		Event
	 */
	void onHistoryButtonClicked(wxCommandEvent& evt);

	/**
	 * Handler called when this frame is closed. If the original MainFrame
	 * still exists, update its data and let it know this windows is being
	 * closed.
	 * @param[in]	evt		Event
	 */
	void onClose(wxCloseEvent& evt);

private: 
	// GUI constants
	const int OFFSET = 200;
	const int RIGHT_CTRL_WIDTH = 120;

	// email of current user
	const std::string current_email;

	/**
	 * UI setup for the information section.
	 * @param[in]	sizer	Parent control sizer
	 */
	void setupInfo(wxBoxSizer* sizer);

	/**
	 * Setup UI and handlers for the actions section.
	 * @param[in]	sizer	Parent control sizer
	 */
	void setupActions(wxBoxSizer* sizer);

	/**
	 * Setup UI and handlers for the extra section.
	 * @param[in]	sizer	Parent control sizer
	 */
	void setupExtra(wxBoxSizer* sizer);

	/**
	 * Update information in this frame with data from server.
	 * @param[in]	dlg		Dialog window to extract the updated info from
	 */
	void loadResponseData(AccountInfoDialog* dlg);

	/**
	 * Update the information in this frame to reflect given account.
	 * @param[in]	acc		Account to be displayed
	 */
	void loadMainFrameData(account acc);

	/**
	 * Change the text font to bold.
	 * @param[out]	text	Text to make bold
	 */
	void makeBold(wxStaticText* text);

	/**
	 * Creates a single row in the information section.
	 * @param[in]	label_name	The name of element in this row
	 * @param[out]	control		The value (readonly) control
	 * @returns Row represented by panel
	 */
	wxPanel* createInfoPanel(std::string label_name, wxTextCtrl*& control);

	/**
	 * Creates the transaction history row in the information section.
	 * @returns		Row represented by panel
	 */
	wxPanel* createHistoryPanel();

	/**
	 * String representation of given double to 2 decimal places.
	 * @param[in]	amount	Double value
	 * @returns				2 decimal places representation
	 */
	std::string format_amount(double amount);

	wxDECLARE_EVENT_TABLE();
};

#endif
