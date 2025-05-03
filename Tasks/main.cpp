#include <vector>
#include <algorithm>
#include <string>
#include <cstdlib>
#include <iostream>
#include <pqxx/pqxx>

#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

#include "Functions.h"
#include "dotenv.h"
using namespace std;

int main()
{
    try
    {
        dotenv::init();
        Functions f;

        std::string conn_str = "host=" + std::string(std::getenv("DB_HOST")) +
                               " dbname=" + std::string(std::getenv("DB_NAME")) +
                               " user=" + std::string(std::getenv("DB_USER")) +
                               " password=" + std::string(std::getenv("DB_PASS")) +
                               " port=" + std::getenv("DB_PORT");

        f.connect(conn_str);

        if (f.isConnected())
        {
            char again = 'y';
            while (again != 'n')
            {
                f.welcome_note();
                f.todays_tasks();
                f.enter_options();

                int option = 0;
                cin >> option;
                clearInputBuffer();

                switch (option)
                {
                case 1:
                    f.task_completion_and_rewiew();
                    break;
                case 2:
                    f.add_subject_and_topic();
                    break;
                case 3:
                    f.shift_whole_srs();
                    break;
                default:
                    break;
                }

                cout << "\n\n";
                space();
                std::cout << "Repeat SRS(n)   ";
                again = std::cin.get();
                system("cls");
            }
        }
    }
    catch (const exception &e)
    {
        std::cerr << "Error: " << e.what() << endl;
        return 1;
    }
    cin.get();
    return 0;
}
