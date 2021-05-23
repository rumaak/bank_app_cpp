#include "dialog.h"
#include "wx/busyinfo.h"
#include "connection_manager.h"
#include "validator.h"
#include <sstream>
#include <iomanip>

void AccountInfoDialog::fillResponseFields(std::string& response)
{
	std::string email = "";
	ConnectionManager::fillField(response, email, ConnectionManager::SEPARATOR);
	ConnectionManager::fillField(response, name, ConnectionManager::SEPARATOR);

	std::string balance_str = "";
	ConnectionManager::fillField(response, balance_str, ConnectionManager::SEPARATOR);
	balance = std::stod(balance_str);

	ConnectionManager::fillField(response, status, ConnectionManager::END);
}

// Map buttons to handlers
wxBEGIN_EVENT_TABLE(TransferDialog, wxDialog)
	EVT_BUTTON(31, onSubmitButtonClicked)
wxEND_EVENT_TABLE()

TransferDialog::TransferDialog(const std::string& email, const std::string& name, wxWindow* parent, 
							   wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, 
							   long style)
: wxDialog( parent, id, title, pos, size, style), AccountInfoDialog(), current_email(email), current_name(name),
  suggested_emails(), suggested_names()
{
	// Ask server for previously used targets and setup autocomplete
	setupSuggestions();

	wxStaticText* email_label = new wxStaticText(this, wxID_ANY, "User email");
	email_field = new wxTextCtrl( this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, 0L, 
		Validator::email());
	email_field->AutoComplete(suggested_emails);

	wxStaticText* name_label = new wxStaticText(this, wxID_ANY, "Account name");
	name_field = new wxTextCtrl( this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, 0L, 
		Validator::accountName());
	name_field->AutoComplete(suggested_names);

	wxStaticText* amount_label = new wxStaticText(this, wxID_ANY, "Amount");
	amount_field = new wxTextCtrl( this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, 0L, 
		wxTextValidator(wxFILTER_NUMERIC));

	wxStaticText* to_from_label = new wxStaticText(this, wxID_ANY, "Transfer type");
	wxArrayString* choices = new wxArrayString();
	choices->Add(wxString("To"));
	choices->Add(wxString("From"));
	to_from_rb = new wxRadioBox(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, *choices);

	submit_button = new wxButton(this, 31, "Send");

	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

	sizer->Add(email_label, 0, wxALL, 5);
	sizer->Add(email_field, 0, wxALL, 5);
	sizer->Add(name_label, 0, wxALL, 5);
	sizer->Add(name_field, 0, wxALL, 5);
	sizer->Add(amount_label, 0, wxALL, 5);
	sizer->Add(amount_field, 0, wxALL, 5);
	sizer->Add(to_from_label, 0, wxLEFT | wxRIGHT | wxTOP, 5);
	sizer->Add(to_from_rb, 0, wxLEFT | wxRIGHT | wxBOTTOM, 5);
	sizer->Add(submit_button, 0, wxALL, 5);

	sizer->SetSizeHints(this);

	SetSizer(sizer);
}

void TransferDialog::onSubmitButtonClicked(wxCommandEvent& evt)
{
	if (!Validator::checkProperDouble(amount_field->GetValue().ToStdString())) {
		std::string message = "'Amount' isn't a valid nonnegative numeric value";
		wxMessageDialog* dlg = new wxMessageDialog(this, message, "Error", wxICON_ERROR | wxOK);
		dlg->ShowModal();
		return;
	}

	int selected = to_from_rb->GetSelection();
	// std::cout << "Selected option id: " << selected << std::endl;
	if (selected == 0) {
		transfer(TO_ID);
	}
	else {
		transfer(FROM_ID);
	}
}

void TransferDialog::transfer(const std::string& id)
{
	std::string response;
	std::string message_status;

	{
		wxWindowDisabler disable_all;
		wxBusyInfo wait("Processing request ...");

		std::string message = id;
		message += current_email + ConnectionManager::SEPARATOR;
		message += email_field->GetValue() + ConnectionManager::SEPARATOR;
		message += current_name + ConnectionManager::SEPARATOR;
		message += name_field->GetValue() + ConnectionManager::SEPARATOR;
		message += amount_field->GetValue() + ConnectionManager::END;

		response = ConnectionManager::sendMessage(message);
		message_status = response.substr(0, 3);
		response = response.erase(0, 3);
	}

	if (message_status == ConnectionManager::ACCEPTED) {
		fillResponseFields(response);
		EndModal(wxID_OK);
	}
	else {
		std::string message = "An error occurred during transfer:\n" + response;
		wxMessageDialog* dlg = new wxMessageDialog(this, message, "Error", wxICON_ERROR | wxOK);
		dlg->ShowModal();
	}
}

void TransferDialog::setupSuggestions()
{
	std::string response;
	std::string message_status;

	{
		wxWindowDisabler disable_all;
		wxBusyInfo wait("Retrieving previously used accounts...");

		std::string message = SUGGEST_ID;
		message += current_email + ConnectionManager::SEPARATOR;
		message += current_name + ConnectionManager::END;

		response = ConnectionManager::sendMessage(message);
		message_status = response.substr(0, 3);
		response = response.erase(0, 3);
	}

	// If successful, fill suggestions
	if (message_status == ConnectionManager::ACCEPTED) {
		fillSuggestions(response);
	}
}

void TransferDialog::fillSuggestions(std::string& response)
{
	std::string count_str = "";
	ConnectionManager::fillField(response, count_str, ConnectionManager::SEPARATOR);

	int count = std::stoi(count_str);
	for (int i = 0; i < count; i++) {
		std::string email_source = "";
		std::string email_target = "";
		std::string name_source = "";
		std::string name_target = "";

		ConnectionManager::fillField(response, email_source, ConnectionManager::SEPARATOR);
		ConnectionManager::fillField(response, email_target, ConnectionManager::SEPARATOR);
		ConnectionManager::fillField(response, name_source, ConnectionManager::SEPARATOR);
		if (i == (count - 1)) {
			ConnectionManager::fillField(response, name_target, ConnectionManager::END);
		}
		else {
			ConnectionManager::fillField(response, name_target, ConnectionManager::SEPARATOR);
		}

		suggested_emails.Add(email_target);
		suggested_names.Add(name_target);
	}
}

// Map buttons to handlers
wxBEGIN_EVENT_TABLE(RecurringPaymentDialog, wxDialog)
	EVT_BUTTON(41, onSubmitButtonClicked)
wxEND_EVENT_TABLE()

RecurringPaymentDialog::RecurringPaymentDialog(const std::string& email, const std::string& name, 
											   const std::string& type, wxWindow* parent, wxWindowID id, 
											   const wxString& title, const wxPoint& pos, 
											   const wxSize& size, long style)
: wxDialog( parent, id, title, pos, size, style), action_type(type), current_email(email),
  current_name(name)
{
	wxStaticText* email_label = new wxStaticText(this, wxID_ANY, "User email");
	email_field = new wxTextCtrl( this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, 0L, 
		Validator::email());

	wxStaticText* name_label = new wxStaticText(this, wxID_ANY, "Account name");
	name_field = new wxTextCtrl( this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, 0L, 
		Validator::accountName());

	wxStaticText* amount_label = new wxStaticText(this, wxID_ANY, "Amount");
	amount_field = new wxTextCtrl( this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, 0L, 
		wxTextValidator(wxFILTER_NUMERIC));

	wxStaticText* startson_label = new wxStaticText(this, wxID_ANY, "Starts on");
	startson_datepicker = new wxDatePickerCtrl(this, wxID_ANY);

	wxStaticText* interval_label = new wxStaticText(this, wxID_ANY, "Every");
	wxArrayString* choices = new wxArrayString();
	choices->Add(wxString("Day"));
	choices->Add(wxString("Week"));
	choices->Add(wxString("Month"));
	choices->Add(wxString("Year"));
	interval_rb = new wxRadioBox(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, *choices);

	submit_button = new wxButton(this, 41, "Send");

	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
	sizer->Add(email_label, 0, wxALL, 5);
	sizer->Add(email_field, 0, wxALL, 5);

	sizer->Add(name_label, 0, wxALL, 5);
	sizer->Add(name_field, 0, wxALL, 5);

	sizer->Add(amount_label, 0, wxALL, 5);
	sizer->Add(amount_field, 0, wxALL, 5);

	sizer->Add(startson_label, 0, wxALL, 5);
	sizer->Add(startson_datepicker, 0, wxALL, 5);

	sizer->Add(interval_label, 0, wxLEFT | wxRIGHT | wxTOP, 5);
	sizer->Add(interval_rb, 0, wxLEFT | wxRIGHT | wxBOTTOM, 5);

	sizer->Add(submit_button, 0, wxALL, 5);

	sizer->SetSizeHints(this);

	SetSizer(sizer);
}

void RecurringPaymentDialog::onSubmitButtonClicked(wxCommandEvent& evt)
{
	if (!Validator::checkProperDouble(amount_field->GetValue().ToStdString())) {
		std::string message = "'Amount' isn't a valid nonnegative numeric value";
		wxMessageDialog* dlg = new wxMessageDialog(this, message, "Error", wxICON_ERROR | wxOK);
		dlg->ShowModal();
		return;
	}

	if (action_type == "direct_debit") {
		transfer(DIRECT_DEBIT_ID);
	}
	else {
		transfer(STANDING_ORDER_ID);
	}
}

void RecurringPaymentDialog::transfer(const std::string& id)
{
	std::string response;
	std::string message_status;

	{
		wxWindowDisabler disable_all;
		wxBusyInfo wait("Processing request ...");

		std::string message = id;
		message += current_email + ConnectionManager::SEPARATOR;
		message += email_field->GetValue() + ConnectionManager::SEPARATOR;
		message += current_name + ConnectionManager::SEPARATOR;
		message += name_field->GetValue() + ConnectionManager::SEPARATOR;
		message += amount_field->GetValue() + ConnectionManager::SEPARATOR;
		message += startson_datepicker->GetValue().FormatISODate() + ConnectionManager::SEPARATOR;
		message += std::to_string(interval_rb->GetSelection()) + ConnectionManager::END;

		std::cout << "Message from client: " << message << std::endl;

		response = ConnectionManager::sendMessage(message);
		message_status = response.substr(0, 3);
		response = response.erase(0, 3);
	}

	if (message_status == ConnectionManager::ACCEPTED) {
		EndModal(wxID_OK);
	}
	else {
		std::string message = "An error occurred during recurring payment setup:\n" + response;
		wxMessageDialog* dlg = new wxMessageDialog(this, message, "Error", wxICON_ERROR | wxOK);
		dlg->ShowModal();
	}
}

// Map buttons to handlers
wxBEGIN_EVENT_TABLE(AddMoneyDialog, wxDialog)
	EVT_BUTTON(51, onSubmitButtonClicked)
wxEND_EVENT_TABLE()

AddMoneyDialog::AddMoneyDialog(const std::string& email, const std::string& name, wxWindow* parent,
							   wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, 
							   long style)
: wxDialog( parent, id, title, pos, size, style), AccountInfoDialog(), current_email(email), current_name(name)
{
	wxStaticText* amount_label = new wxStaticText(this, wxID_ANY, "Amount");
	amount_field = new wxTextCtrl( this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, 0L, 
		wxTextValidator(wxFILTER_NUMERIC));

	submit_button = new wxButton(this, 51, "Add");

	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
	sizer->Add(amount_label, 0, wxALL, 5);
	sizer->Add(amount_field, 0, wxALL, 5);

	sizer->Add(submit_button, 0, wxALL, 5);

	sizer->SetSizeHints(this);

	SetSizer(sizer);
}

void AddMoneyDialog::onSubmitButtonClicked(wxCommandEvent& evt)
{
	if (!Validator::checkProperDouble(amount_field->GetValue().ToStdString())) {
		std::string message = "'Amount' isn't a valid nonnegative numeric value";
		wxMessageDialog* dlg = new wxMessageDialog(this, message, "Error", wxICON_ERROR | wxOK);
		dlg->ShowModal();
		return;
	}

	std::string response;
	std::string message_status;

	{
		wxWindowDisabler disable_all;
		wxBusyInfo wait("Processing request ...");

		std::string message = ACTION_ID;

		message += current_email + ConnectionManager::SEPARATOR;
		message += current_name + ConnectionManager::SEPARATOR;
		message += amount_field->GetValue() + ConnectionManager::END;

		response = ConnectionManager::sendMessage(message);
		message_status = response.substr(0, 3);
		response = response.erase(0, 3);
	}

	if (message_status == ConnectionManager::ACCEPTED) {
		fillResponseFields(response);
		EndModal(wxID_OK);
	}
	else {
		std::string message = "An error occurred during adding money:\n" + response;
		wxMessageDialog* dlg = new wxMessageDialog(this, message, "Error", wxICON_ERROR | wxOK);
		dlg->ShowModal();
	}
}

// Map buttons to handlers
wxBEGIN_EVENT_TABLE(LoginRegisterDialog, wxDialog)
	EVT_BUTTON(71, onLoginButtonClicked)
	EVT_BUTTON(72, onRegisterButtonClicked)
wxEND_EVENT_TABLE()

LoginRegisterDialog::LoginRegisterDialog(wxWindow* parent, wxWindowID id, 
	const wxString& title, const wxPoint& pos, const wxSize& size, long style)
	: wxDialog( parent, id, title, pos, size, style), AccountListDialog()
{
	login_name = new wxStaticText(this, wxID_ANY, "Email");
	login_name_field = new wxTextCtrl( this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, 0L, 
		Validator::email());
	login_pswd = new wxStaticText(this, wxID_ANY, "Password");
	login_pswd_field = new wxTextCtrl( this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize,
		wxTE_PASSWORD, Validator::passwd() );

	wxPanel* button_panel = new wxPanel(this, wxID_ANY);

	button_login = new wxButton(button_panel, 71, "login");
	button_register = new wxButton(button_panel, 72, "register");

	wxBoxSizer* panel_sizer = new wxBoxSizer(wxHORIZONTAL);
	panel_sizer->Add(button_login, 0, wxALL, 5);
	panel_sizer->Add(button_register, 0, wxALL, 5);

	button_panel->SetSizer(panel_sizer);

	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
	sizer->Add(login_name, 0, wxALL, 5);
	sizer->Add(login_name_field, 0, wxALL, 5);
	sizer->Add(login_pswd, 0, wxALL, 5);
	sizer->Add(login_pswd_field, 0, wxALL, 5);
	sizer->Add(button_panel, 0, wxALL, 5);

	sizer->SetSizeHints(this);

	SetSizer(sizer);
}

void LoginRegisterDialog::onLoginButtonClicked(wxCommandEvent& evt)
{
	loginRegister(LOGIN_ID);
}

void LoginRegisterDialog::onRegisterButtonClicked(wxCommandEvent& evt)
{
	loginRegister(REGISTER_ID);
}

void LoginRegisterDialog::loginRegister(const std::string& id)
{
	if (login_name_field->GetValue().IsEmpty() || login_pswd_field->GetValue().IsEmpty()) {
		std::string message = "Name and password have to be nonempty";
		wxMessageDialog* dlg = new wxMessageDialog(this, message, "Error", wxICON_ERROR | wxOK);
		dlg->ShowModal();
		return;
	}

	std::string response;
	std::string message_status;

	{
		wxWindowDisabler disable_all;
		wxBusyInfo wait("Logging in / registering ...");

		std::string message = id;
		message += login_name_field->GetValue() + ConnectionManager::SEPARATOR;
		message += login_pswd_field->GetValue() + ConnectionManager::END;

		response = ConnectionManager::sendMessage(message);
		message_status = response.substr(0, 3);
		response = response.erase(0, 3);
	}

	if (message_status == ConnectionManager::ACCEPTED) {
		fillResponseFields(response);
		EndModal(wxID_OK);
	}
	else {
		std::string message = "An error occurred during login / registration:\n" + response;
		wxMessageDialog* dlg = new wxMessageDialog(this, message, "Error", wxICON_ERROR | wxOK);
		dlg->ShowModal();
	}
}

void AccountListDialog::fillResponseFields(std::string& response)
{
	accounts.clear();
	std::string count_str = "";
	ConnectionManager::fillField(response, email, ConnectionManager::SEPARATOR);
	ConnectionManager::fillField(response, count_str, ConnectionManager::SEPARATOR);

	int count = std::stoi(count_str);
	for (int i = 0; i < count; i++) {
		std::string name = "";
		std::string amount_str = "";
		std::string state = "";

		ConnectionManager::fillField(response, name, ConnectionManager::SEPARATOR);
		ConnectionManager::fillField(response, amount_str, ConnectionManager::SEPARATOR);
		if (i == (count - 1)) {
			ConnectionManager::fillField(response, state, ConnectionManager::END);
		}
		else {
			ConnectionManager::fillField(response, state, ConnectionManager::SEPARATOR);
		}

		double amount = std::stod(amount_str);
		account a{name, amount, state};

		accounts.push_back(a);
	}
}

// Map buttons to handlers
wxBEGIN_EVENT_TABLE(AddAccountDialog, wxDialog)
	EVT_BUTTON(81, onSubmitButtonClicked)
wxEND_EVENT_TABLE()

AddAccountDialog::AddAccountDialog(const std::string& email, wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style)
: wxDialog( parent, id, title, pos, size, style), AccountListDialog(), current_account(email)
{
	wxStaticText* name_label = new wxStaticText(this, wxID_ANY, "Account name");
	name_field = new wxTextCtrl( this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, 0L, 
		Validator::accountName());

	submit_button = new wxButton(this, 81, "Send");

	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
	sizer->Add(name_label, 0, wxALL, 5);
	sizer->Add(name_field, 0, wxALL, 5);
	sizer->Add(submit_button, 0, wxALL, 5);

	sizer->SetSizeHints(this);

	SetSizer(sizer);
}

void AddAccountDialog::onSubmitButtonClicked(wxCommandEvent& evt)
{
	if (name_field->GetValue().IsEmpty()) {
		std::string message = "Account name has to be a nonempty string";
		wxMessageDialog* dlg = new wxMessageDialog(this, message, "Error", wxICON_ERROR | wxOK);
		dlg->ShowModal();
		return;
	}

	std::string response;
	std::string message_status;

	{
		wxWindowDisabler disable_all;
		wxBusyInfo wait("Processing request ...");

		std::string message = ACTION_ID;
		
		message += current_account + ConnectionManager::SEPARATOR;
		message += name_field->GetValue() + ConnectionManager::END;

		response = ConnectionManager::sendMessage(message);
		message_status = response.substr(0, 3);
		response = response.erase(0, 3);
	}

	if (message_status == ConnectionManager::ACCEPTED) {
		fillResponseFields(response);
		EndModal(wxID_OK);
	}
	else {
		std::string message = "An error occurred during adding money:\n" + response;
		wxMessageDialog* dlg = new wxMessageDialog(this, message, "Error", wxICON_ERROR | wxOK);
		dlg->ShowModal();
	}
}

// Map buttons to handlers
wxBEGIN_EVENT_TABLE(TransactionHistoryDialog, wxDialog)
	EVT_BUTTON(91, onSubmitButtonClicked)
wxEND_EVENT_TABLE()

TransactionHistoryDialog::TransactionHistoryDialog(const std::string& email, const std::string& name,
	wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, 
	long style)
	: wxDialog( parent, id, title, pos, size, style), current_email(email), current_name(name)
{
	wxPanel* left_panel = leftPanelSetup();
	wxPanel* right_panel = rightPanelSetup();

	wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);

	sizer->Add(left_panel, 0, wxALL, 5);
	sizer->Add(right_panel, 0, wxALL, 5);

	sizer->SetSizeHints(this);
	SetSizer(sizer);
}

void TransactionHistoryDialog::onSubmitButtonClicked(wxCommandEvent& evt)
{
	std::string response;
	std::string message_status;

	{
		wxWindowDisabler disable_all;
		wxBusyInfo wait("Retrieving transactions from server...");

		std::string message = ACTION_ID;
		message += current_email + ConnectionManager::SEPARATOR;
		message += current_name + ConnectionManager::SEPARATOR;
		message += since_datepicker->GetValue().FormatISODate() + ConnectionManager::SEPARATOR;
		message += until_datepicker->GetValue().FormatISODate() + ConnectionManager::END;

		response = ConnectionManager::sendMessage(message);
		message_status = response.substr(0, 3);
		response = response.erase(0, 3);
	}

	if (message_status == ConnectionManager::ACCEPTED) {
		std::vector<transaction> ts = extractAccounts(response);
		transactions->fillTransactions(ts);
	}
	else {
		std::string message = "An error occurred during retrieval of transaction history:\n" + response;
		wxMessageDialog* dlg = new wxMessageDialog(this, message, "Error", wxICON_ERROR | wxOK);
		dlg->ShowModal();
	}
}

wxPanel* TransactionHistoryDialog::rightPanelSetup()
{
	wxPanel* right_panel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxSize(600,400));
	wxPanel* top_panel = new wxPanel(right_panel, wxID_ANY);

	wxStaticText* date_label = new wxStaticText(top_panel, wxID_ANY, "Date");
	wxStaticText* from_label = new wxStaticText(top_panel, wxID_ANY, "From");
	wxStaticText* to_label = new wxStaticText(top_panel, wxID_ANY, "To");
	wxStaticText* amount_label = new wxStaticText(top_panel, wxID_ANY, "Amount");

	wxBoxSizer* top_sizer = new wxBoxSizer(wxHORIZONTAL);

	top_sizer->Add(date_label, 1);
	top_sizer->Add(from_label, 1);
	top_sizer->Add(to_label, 1);
	top_sizer->Add(amount_label, 1);

	top_panel->SetSizer(top_sizer);

	transactions = new TransactionsPane(right_panel, wxID_ANY);

	wxBoxSizer* right_sizer = new wxBoxSizer(wxVERTICAL);

	right_sizer->Add(top_panel, 0, wxEXPAND | wxALL, 5);
	right_sizer->Add(transactions, 1,  wxEXPAND | wxRIGHT | wxBOTTOM, 5);

	right_panel->SetSizer(right_sizer);

	return right_panel;
}

std::vector<transaction> TransactionHistoryDialog::extractAccounts(std::string& response)
{
	std::vector<transaction> ts;
	std::string count_str = "";

	ConnectionManager::fillField(response, count_str, ConnectionManager::SEPARATOR);

	int count = std::stoi(count_str);
	for (int i = 0; i < count; i++) {
		std::string email_from = "";
		std::string email_to = "";
		std::string name_from = "";
		std::string name_to = "";
		std::string amount_str = "";
		std::string date = "";

		ConnectionManager::fillField(response, email_from, ConnectionManager::SEPARATOR);
		ConnectionManager::fillField(response, email_to, ConnectionManager::SEPARATOR);
		ConnectionManager::fillField(response, name_from, ConnectionManager::SEPARATOR);
		ConnectionManager::fillField(response, name_to, ConnectionManager::SEPARATOR);
		ConnectionManager::fillField(response, amount_str, ConnectionManager::SEPARATOR);
		if (i == (count - 1)) {
			ConnectionManager::fillField(response, date, ConnectionManager::END);
		}
		else {
			ConnectionManager::fillField(response, date, ConnectionManager::SEPARATOR);
		}

		double amount = std::stod(amount_str);
		transaction t{date, email_from, email_to, name_from, name_to, amount};
		ts.push_back(t);
	}

	return ts;
}

wxPanel* TransactionHistoryDialog::leftPanelSetup()
{
	wxPanel* left_panel = new wxPanel(this, wxID_ANY);

	wxStaticText* since_label = new wxStaticText(left_panel, wxID_ANY, "Since");
	since_datepicker = new wxDatePickerCtrl(left_panel, wxID_ANY);
	wxStaticText* until_label = new wxStaticText(left_panel, wxID_ANY, "Until");
	until_datepicker = new wxDatePickerCtrl(left_panel, wxID_ANY);
	submit_button = new wxButton(left_panel, 91, "Send");

	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

	sizer->Add(since_label, 0, wxALL, 5);
	sizer->Add(since_datepicker, 0, wxALL, 5);
	sizer->Add(until_label, 0, wxALL, 5);
	sizer->Add(until_datepicker, 0, wxALL, 5);
	sizer->Add(submit_button, 0, wxALL, 5);

	left_panel->SetSizer(sizer);

	return left_panel;
}

TransactionsPane::TransactionsPane(wxWindow* parent, wxWindowID id) : wxScrolledWindow(parent, id, wxDefaultPosition)
{
	sizer = new wxBoxSizer(wxVERTICAL);
	SetSizer(sizer);
	SetScrollRate(5, 5);
}

void TransactionsPane::fillTransactions(const std::vector<transaction>& transactions)
{
	sizer->Clear(true);

	for (auto&& t : transactions) {
		wxPanel* panel = createTransactionPanel(t);
		sizer->Add(panel, 0, wxEXPAND | wxALL, 3);
	}

	FitInside();
}

wxPanel* TransactionsPane::createTransactionPanel(transaction t)
{
	wxPanel* panel = new wxPanel(this, wxID_ANY);
	wxBoxSizer* sizer_panel = new wxBoxSizer(wxHORIZONTAL);

	std::string from = t.email_from + ":" + t.name_from;
	std::string to = t.email_to + ":" + t.name_to;

	std::stringstream stream;
	stream << std::fixed << std::setprecision(2) << t.amount;
	std::string amount = stream.str();

	wxTextCtrl* date_ctrl = new wxTextCtrl(panel, wxID_ANY, t.date, wxDefaultPosition, wxDefaultSize,
		wxTE_READONLY);
	wxTextCtrl* from_ctrl = new wxTextCtrl(panel, wxID_ANY, from, wxDefaultPosition, wxDefaultSize,
		wxTE_READONLY);
	wxTextCtrl* to_ctrl = new wxTextCtrl(panel, wxID_ANY, to, wxDefaultPosition, wxDefaultSize,
		wxTE_READONLY);
	wxTextCtrl* amount_ctrl = new wxTextCtrl(panel, wxID_ANY, amount, wxDefaultPosition, wxDefaultSize,
		wxTE_READONLY);

	sizer_panel->Add(date_ctrl, 1, wxALL, 5);
	sizer_panel->Add(from_ctrl, 1, wxALL, 5);
	sizer_panel->Add(to_ctrl, 1, wxALL, 5);
	sizer_panel->Add(amount_ctrl, 1, wxALL, 5);

	panel->SetSizer(sizer_panel);
	return panel;
}
