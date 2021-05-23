#include <string>

#ifndef MAIL_CLIENT_H_
#define MAIL_CLIENT_H_

/**
 * Class encapsulating mail sending functionality.
 */
class MailClient {
public:
	/**
	 * Send an email with given contents to user.
	 * @param[in]	to		Email recipient
	 * @param[in]	message	Email contents
	 */
	void sendEmail(const std::string& to, const std::string& message);

private:
	// Credentials of email account from which client sends emails to users
	const std::string FROM = "<from@gmail.com>";
	const std::string USR = "from@gmail.com";
	const std::string PSWD = "password";
};

#endif

