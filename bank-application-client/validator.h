#include <wx/wx.h>

#ifndef VALIDATOR_H_
#define VALIDATOR_H_

/**
* A "static" class encapsulating functionality for user input validation.
*/
class Validator
{
public:
	/**
	 * Get wxWidgets string with alpha-numerical characters.
	 * @returns		Alpha-numerical characters
	 */
	static wxString alnumString();

	/**
	 * Get validator for email fields.
	 * @returns		Email wxTextValidator
	 */
	static wxTextValidator email();

	/**
	 * Get validator for password field.
	 * @returns		Password wxTextValidator
	 */
	static wxTextValidator passwd();

	/**
	 * Get validator for account name fields.
	 * @returns		Account name wxTextValidator
	 */
	static wxTextValidator accountName();

	/**
	 * Check validity and non-negativity of string representations of double value.
	 * @param[in]	str		String representation of double
	 * @returns				True if str represents correct double
	 */
	static bool checkProperDouble(const std::string& str);
};

#endif
