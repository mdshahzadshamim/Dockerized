#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <iostream>
#include <pqxx/pqxx>
#include <memory>
#include <string>
#include "dotenv.h"

using namespace std;

void clearInputBuffer();
void space();
void heading();
string interval(int n);
string get_new_study_date(std::string study_date, int days);


class Functions
{
public:
    Functions(); // Constructor
    ~Functions(); // Destructor

    void connect(const std::string &conninfo); // DBConnect
    bool isConnected() const;

    void welcome_note();
    void todays_tasks();
    void enter_options();
    void task_completion_and_rewiew();
    void process_the_review(int id, int review);
    void add_subject_and_topic();
    void generate_srs(string subject_name, string topic);
    void utility_add_one_entry(string subject_name, string topic, int n, int study_type);
    void shift_whole_srs();
    void utility_shift_one_entry(int id, string new_study_date);
    void utility_shift_one_entry_review_based(string subject_name, string topic, int study_type, int n);

private:
    std::unique_ptr<pqxx::connection> conn;
};

#endif // FUNCTIONS_H