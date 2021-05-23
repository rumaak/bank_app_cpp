#include <SDKDDKVer.h>

#include <asio.hpp>
#include "account_frame.h"
#include <wx/wizard.h>
#include "wx/busyinfo.h"
#include <iomanip>

using tcp = asio::ip::tcp;

// Map buttons to handlers
wxBEGIN_EVENT_TABLE(AccountFrame, wxFrame)
	EVT_BUTTON(21, onTransferButtonClicked)
	EVT_BUTTON(22, onDirectDebitButtonClicked)
	EVT_BUTTON(23, onStandingOrderButtonClicked)
	EVT_BUTTON(24, onAddMoneyButtonClicked)
	EVT_BUTTON(25, onHistoryButtonClicked)
	EVT_CLOSE(onClose)
wxEND_EVENT_TABLE()

// Don't use main_frame for anything else than close event; if you can't help yourself, at least check whether it's not nullptr first
AccountFrame::AccountFrame(account acc, MainFrame* mf) : wxFrame(nullptr, wxID_ANY, "Bank application"),
	main_frame(mf), current_email(mf->email)
{
	// White background
	SetBackgroundColour(wxColor(255,255,255));

	// Setup and fill sizer with info, actions and extra sections
	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
	setupInfo(sizer);
	setupActions(sizer);
	setupExtra(sizer);
	SetSizer(sizer);
	sizer->SetSizeHints(this);

	loadMainFrameData(acc);
}

void AccountFrame::onTransferButtonClicked(wxCommandEvent& evt)
{
	std::string current_name = name_control->GetValue().ToStdString();
	TransferDialog* dlg = new TransferDialog(current_email, current_name, this, wxID_ANY, "Transfer");

	if (dlg->ShowModal() == wxID_OK) {
		loadResponseData(dlg);
	}

    dlg->Destroy();
}

void AccountFrame::onDirectDebitButtonClicked(wxCommandEvent& evt)
{
	std::string current_name = name_control->GetValue().ToStdString();
	RecurringPaymentDialog* dlg = new RecurringPaymentDialog(current_email, current_name, "direct_debit", this, 
		wxID_ANY, "Direct debit");
	dlg->ShowModal();
    dlg->Destroy();
}

void AccountFrame::onStandingOrderButtonClicked(wxCommandEvent& evt)
{
	std::string current_name = name_control->GetValue().ToStdString();
	RecurringPaymentDialog* dlg = new RecurringPaymentDialog(current_email, current_name, "standing_order", this,
		wxID_ANY, "Standing order");
	dlg->ShowModal();
    dlg->Destroy();
}

void AccountFrame::onAddMoneyButtonClicked(wxCommandEvent& evt)
{
	std::string current_name = name_control->GetValue().ToStdString();
	AddMoneyDialog* dlg = new AddMoneyDialog(current_email, current_name, this, wxID_ANY, "Add money");

	if (dlg->ShowModal() == wxID_OK) {
		loadResponseData(dlg);
	}

    dlg->Destroy();
}

void AccountFrame::onHistoryButtonClicked(wxCommandEvent& evt)
{
	std::string current_name = name_control->GetValue().ToStdString();
	TransactionHistoryDialog* dlg = new TransactionHistoryDialog(current_email, current_name, this, 
		wxID_ANY, "Transaction history");

	dlg->ShowModal();
    dlg->Destroy();
}

void AccountFrame::onClose(wxCloseEvent& evt)
{
	if (main_frame != nullptr) {
		main_frame->updateData();
		main_frame->open_window = nullptr;
	}
	Destroy();
}

void AccountFrame::setupInfo(wxBoxSizer* sizer)
{
	wxStaticText* info_label = new wxStaticText(this, wxID_ANY, "Info");
	makeBold(info_label);

	wxPanel* name_panel = createInfoPanel("Name", name_control);
	wxPanel* balance_panel = createInfoPanel("Balance", balance_control);
	wxPanel* state_panel = createInfoPanel("State", state_control);
	wxPanel* history_panel = createHistoryPanel();

	sizer->Add(info_label, 0, wxALL, 5);
	sizer->Add(name_panel, 0, wxALL, 5);
	sizer->Add(balance_panel, 0, wxALL, 5);
	sizer->Add(state_panel, 0, wxALL, 5);
	sizer->Add(history_panel, 0, wxALL, 5);
}

void AccountFrame::setupActions(wxBoxSizer* sizer)
{
	wxStaticText* actions_label = new wxStaticText(this, wxID_ANY, "Actions");
	makeBold(actions_label);

	wxPanel* actions_panel = new wxPanel(this, wxID_ANY);
	transfer_button = new wxButton(actions_panel, 21, "Transfer", wxPoint(0,0));
	direct_debit_button = new wxButton(actions_panel, 22, "Direct debit", wxPoint(110,0));
	standing_order_button = new wxButton(actions_panel, 23, "Standing order", wxPoint(220,0));

	sizer->Add(actions_label, 0, wxALL, 5);
	sizer->Add(actions_panel, 0, wxALL, 5);
}

void AccountFrame::setupExtra(wxBoxSizer* sizer)
{
	wxStaticText* extra_label = new wxStaticText(this, wxID_ANY, "Extra");
	makeBold(extra_label);
	add_money_button = new wxButton(this, 24, "Add money");

	sizer->Add(extra_label, 0, wxALL, 5);
	sizer->Add(add_money_button, 0, wxALL, 5);
}

void AccountFrame::loadResponseData(AccountInfoDialog* dlg)
{
	balance_control->SetValue(format_amount(dlg->balance));
	name_control->SetValue(dlg->name);
	state_control->SetValue(dlg->status);
}

void AccountFrame::loadMainFrameData(account acc)
{
	balance_control->SetValue(format_amount(acc.amount));
	name_control->SetValue(acc.name);
	state_control->SetValue(acc.state);
}

void AccountFrame::makeBold(wxStaticText* text)
{
	wxFont font = text->GetFont();
	font.SetWeight( wxFONTWEIGHT_BOLD );
	text->SetFont( font );
}

wxPanel* AccountFrame::createInfoPanel(std::string label_name, wxTextCtrl*& control)
{
	wxPanel* panel = new wxPanel(this, wxID_ANY);
	wxStaticText* name_label = new wxStaticText(panel, wxID_ANY, label_name, wxPoint(0, 0));

	// Make sure all the controls on the right side are of the same size
	int height = wxDefaultSize.GetHeight();
	wxSize size{RIGHT_CTRL_WIDTH, height};

	control = new wxTextCtrl(panel, wxID_ANY, "value", wxPoint(OFFSET, 0), size, wxTE_READONLY);
	return panel;
}

wxPanel* AccountFrame::createHistoryPanel()
{
	wxPanel* panel = new wxPanel(this, wxID_ANY);

	// Make sure all the controls on the right side are of the same size
	int height = wxDefaultSize.GetHeight();
	wxSize size{RIGHT_CTRL_WIDTH, height};

	wxStaticText* name_label = new wxStaticText(panel, wxID_ANY, "Transaction history", wxPoint(0, 0));
	history_button = new wxButton(panel, 25, "Transaction history", wxPoint(OFFSET, 0), size);
	return panel;
}

std::string AccountFrame::format_amount(double amount)
{
	std::stringstream stream;
	stream << std::fixed << std::setprecision(2) << amount;
	std::string result = stream.str();

	return result;
}

