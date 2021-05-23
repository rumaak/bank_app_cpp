# Draft of the semestral work

### Bank client
- client part
    - GUI (form)
    - first screen (login)
        - log in / create account
    - second screen (info)
        - info - account name, amount, current state (blocked,...)
        - regular options - transfer, direct debit, standing order
        - extra option - add money (analog to real world deposition of funds through ATM)
        - (message box confirming successful action)
- server part
    - authorize user / create new account / remove account
    - send info to client
    - execute client actions (+ send confirmation and new info)
    - at given intervals execute standing orders
    - communication with database
    - sends email when user becomes "blocked" (= has too much debt)
- database part
    - list of accounts with needed info
    - info - ID, (name?), email, current amount, direct debit amount, direct debit time, direct debit currently used, pswd hash, standing order info, state (ok, blocked),..
    - possibly other server options

##### Update
During the course of writing the application an extension of the functionality it should provide was requested. The changes to specification are provided below.

- each user can have multiple accounts
- after login, the account window itself does not appear; rather, a list of accounts of given user (and button enabling him / her to add a new one) is shown
- after opening one of the accounts new window appears - aside from the actions specified above a transaction history can be viewed for given account
- when issuing a transaction directed towards another user suggestions based on previous use are offered

##### Libraries / frameworks
- GUI - wxwidgets
- client / server communication - Boost.Asio
- email - libcurl
- database - SQLite
