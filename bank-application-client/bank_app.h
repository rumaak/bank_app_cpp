#include "main_frame.h"
#include "wx/wx.h"

#ifndef BANK_APP_H_ 
#define BANK_APP_H_

/**
* Entrypoint class.
*/
class BankApp : public wxApp
{
public:
	/**
	 * Entrypoint function.
	 * @returns		whether the application initialized properly
	 */
	virtual bool OnInit();
private:
	MainFrame* main_frame = nullptr;
};

#endif

