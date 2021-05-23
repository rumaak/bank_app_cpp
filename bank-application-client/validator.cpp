#include "validator.h"

wxString Validator::alnumString()
{
	wxString alnum = "abcdefghijklmnopqrstuvwxyz";
	alnum += "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	alnum += "0123456789";

    return alnum;
}

wxTextValidator Validator::email()
{
	wxTextValidator validator_email(wxFILTER_INCLUDE_CHAR_LIST);

	wxString alnum_email(alnumString());
	alnum_email += "@.-_";
	validator_email.SetCharIncludes(alnum_email);

    return validator_email;
}

wxTextValidator Validator::passwd()
{
	wxTextValidator validator_passwd(wxFILTER_INCLUDE_CHAR_LIST);

	wxString alnum_passwd(alnumString());
	alnum_passwd += " !#$%&*+,-.:<=>?@[]^_{|}~";
	validator_passwd.SetCharIncludes(alnum_passwd);

	return validator_passwd;
}

wxTextValidator Validator::accountName()
{
	wxTextValidator validator_name(wxFILTER_INCLUDE_CHAR_LIST);

	wxString alnum_name(alnumString());
	alnum_name += ".-_";
	validator_name.SetCharIncludes(alnum_name);

    return validator_name;
}

bool Validator::checkProperDouble(const std::string& str)
{
	// The stod check should be redundant, but we do it nonetheless
	bool proper_double = true;
	double amount = 0;
	try {
		amount = std::stod(str);
	}
	catch (std::exception&) {
		proper_double = false;
	}

	if (proper_double) {
		if (amount < 0) {
			proper_double = false;
		}
	}

	return proper_double;
}
