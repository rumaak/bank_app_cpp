#include "bank_app.h"

wxIMPLEMENT_APP(BankApp);

bool BankApp::OnInit()
{
    main_frame = new MainFrame();

    if (!main_frame->init_success) {
        return false;
    }

    main_frame->Show();

    return true;
}
