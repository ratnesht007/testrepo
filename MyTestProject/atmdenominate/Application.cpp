// atmdenominate.cpp : This file contains the 'main' function. Program execution begins and ends there.
//


#include<iostream>
#include "CashOutModule.h"

int main()
{

    CashOutModule cashout;

    bool withdrawalStatus = cashout.getwidrawalStatus();

    if (withdrawalStatus) {

        std::cout << "You can do cash transactions" << "\n";
    }
    else {
        std::cout << "You can't do cash transactions" << "\n";
    }

    std::string sopPassword = "";

    int choice = 0;

    do 
    {

        std::cout << "\n";

        std::cout << "**************************MAIN MENU START****************************\n";

        std::cout << "Please select one of the following options:  \n";

        std::cout << "  1: Show Cassette Details\n";
        if (withdrawalStatus) {
            std::cout << "  2: Go to Withdrawal\n";
        }
        else {
            std::cout << "  2: Go to Withdrawal (TEMP NOT AVAILABLE)\n";
        }
        std::cout << "  3: Quit\n";

        std::cout << "**************************MAIN MENU END*****************************\n\n";

        std::cin >> choice;

        if (std::cin.fail()) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }

        switch (choice)
        {
        case 1 :
            std::cout << "Enter password (hint: 1234)\n";
            std::cin >> sopPassword;
            if (cashout.getSopPassword() == sopPassword) {
                cashout.printCuInfo();
            }
            else {
                std::cout << "Invalid password, please try again \n";
            }
            break;
        case 2:
            if (withdrawalStatus) {
                cashout.withdrawalFlow();
            }
            else {
                std::cout << " Withdrawal is temporary not available. Please choose another transaction\n";
            }
            break;
        case 3:
            std::cout << "Thank you for using our service. Good bye" << "\n";
            break;
        default:
            std::cout << "Enter valid number: " << "\n";
            break;
        }



    } while (choice != 3);


    return 0;

}