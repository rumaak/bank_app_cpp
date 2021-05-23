#include "main_frame.h"
#include <iomanip>
#include <sstream>
#include "wx/busyinfo.h"
#include "account_frame.h"

// Map buttons to handlers
wxBEGIN_EVENT_TABLE(MainFrame, wxFrame)
	EVT_BUTTON(61, onNewAccountButtonClicked)
	EVT_CLOSE(onClose)
wxEND_EVENT_TABLE()

MainFrame::MainFrame() : wxFrame(nullptr, wxID_ANY, "Bank application", wxDefaultPosition, wxSize(500,600)),
	accounts(), email("")
{
	// White background
	SetBackgroundColour(wxColor(255,255,255));

	// Place controls
	wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
	new_account_button = new wxButton(this, 61, "Add account");
	pane = new AccountsPane(this, wxID_ANY);
	sizer->Add(new_account_button, 1, wxALL, 5);
	sizer->Add(pane, 4, wxEXPAND | wxALL, 5);
	SetSizer(sizer);

	// Initial login / register dialog
	loginRegister();
}

void MainFrame::onNewAccountButtonClicked(wxCommandEvent& evt)
{
	AddAccountDialog* dlg = new AddAccountDialog(email, this, wxID_ANY, "Add account");
	if (dlg->ShowModal() == wxID_OK) {
		init_success = true;
		loadResponseData(dlg);
	}

    dlg->Destroy();
}

void MainFrame::onOpenButtonClick(wxCommandEvent& evt)
{
	int id = evt.GetId();
	std::string name = pane->button_id_to_name[id];
	account selected_acc;

	for (int i = 0; i < (int) accounts.size(); i++) {
		account acc = accounts[i];
		if (acc.name == name) {
			selected_acc = acc;
			break;
		}
	}

	if (open_window == nullptr) {
		AccountFrame* account_frame = new AccountFrame(selected_acc, this);
		open_window = account_frame;
		account_frame->Show();
	}
	else {
		std::string message = "Cannot open multiple accounts at once";
		wxMessageDialog* dlg = new wxMessageDialog(this, message, "Error", wxICON_ERROR | wxOK);
		dlg->ShowModal();
	}

}

void MainFrame::onClose(wxCloseEvent& evt) {
	if (open_window != nullptr) {
		open_window->main_frame = nullptr;
	}
	Destroy();
}

void MainFrame::loginRegister()
{
	LoginRegisterDialog* dlg = new LoginRegisterDialog(this, wxID_ANY, "Login / Registration");
	if (dlg->ShowModal() == wxID_OK) {
		init_success = true;
		loadResponseData(dlg);
	}

    dlg->Destroy();
}

void MainFrame::loadResponseData(AccountListDialog* dlg)
{
	email = dlg->email;
	accounts = dlg->accounts;

	pane->updateAccounts(this);
}

void MainFrame::updateData()
{
	std::string response;
	std::string message_status;

	// request list of accounts from server
	{
		wxWindowDisabler disable_all;
		wxBusyInfo wait("Updating account info ...");

		std::string message = LIST_ACCOUNTS_ID;
		message += email + ConnectionManager::END;

		response = ConnectionManager::sendMessage(message);
		message_status = response.substr(0, 3);
		response = response.erase(0, 3);
	}

	// parse response, update accounts
	if (message_status == ConnectionManager::ACCEPTED) {
		accounts.clear();
		std::string count_str = "";
		std::string email_response = "";

		ConnectionManager::fillField(response, email_response, ConnectionManager::SEPARATOR);
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

		pane->updateAccounts(this);
	}
	else {
		std::string message = "An error occurred during the retrieval of accounts:\n" + response;
		wxMessageDialog* dlg = new wxMessageDialog(this, message, "Error", wxICON_ERROR | wxOK);
		dlg->ShowModal();
	}
}

AccountsPane::AccountsPane(wxWindow* parent, wxWindowID id) : wxScrolledWindow(parent, id), button_id_to_name()
{
	sizer = new wxBoxSizer(wxVERTICAL);
	this->SetSizer(sizer);
	this->SetScrollRate(5, 5);
}

void AccountsPane::updateAccounts(MainFrame* main_frame)
{
	button_id_to_name.clear();
	sizer->Clear(true);

	for (auto&& acc : main_frame->accounts) {
		wxPanel* panel = createAccountPanel(acc.name, acc.amount, acc.state, main_frame);
		sizer->Add(panel, 0, wxALL, 3);
	}

	this->FitInside();
}

wxPanel* AccountsPane::createAccountPanel(const std::string& name, double amount, const std::string& state,
	MainFrame* main_frame)
{
	wxPanel* panel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSIMPLE_BORDER);
	wxBoxSizer* sizer_panel = new wxBoxSizer(wxVERTICAL);

	addAccountPanelRow(panel, sizer_panel, "Account", name);
	addAccountPanelRow(panel, sizer_panel, "Balance", format_amount(amount));
	addAccountPanelRow(panel, sizer_panel, "State", state);

	wxButton* open_button = new wxButton(panel, wxID_ANY, "Open");
	Bind(wxEVT_BUTTON, &MainFrame::onOpenButtonClick, main_frame);

	button_id_to_name.insert({ open_button->GetId(), name });

	sizer_panel->Add(open_button, 0, wxALL, 5);
	panel->SetSizer(sizer_panel);

	return panel;
}

void AccountsPane::addAccountPanelRow(wxPanel* parent, wxBoxSizer* parent_sizer, const std::string& name, const std::string& value)
{
	wxPanel* row = new wxPanel(parent, wxID_ANY);

	wxStaticText* state_label = new wxStaticText(row, wxID_ANY, name, wxPoint(0, 0));
	wxTextCtrl* state_control = new wxTextCtrl(row, wxID_ANY, value, wxPoint(OFFSET, 0), wxDefaultSize, wxTE_READONLY);

	parent_sizer->Add(row, 0, wxALL, 5);
}

std::string AccountsPane::format_amount(double amount)
{
	std::stringstream stream;
	stream << std::fixed << std::setprecision(2) << amount;
	std::string result = stream.str();

	return result;
}
