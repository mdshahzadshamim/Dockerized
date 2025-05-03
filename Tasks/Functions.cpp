#include <pqxx/pqxx>
#include <ctime>
#include <chrono>
#include <sstream>
#include <iomanip>
#include "Functions.h"

void clearInputBuffer()
{
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

void space()
{
    std::cout << "                     ";
}

void heading()
{
    system("clear");
    cout << "\n\n\n                     R   E   V   I   S   I   O   N                    S   C   H   E   D   U   L   E\n\n";
}

string interval(int n)
{
    // To return interval based on the interval id
    switch (n)
    {
    case 0:
        return "New";
        break;
    case 1:
        return "24 hr";
        break;
    case 2:
        return "72 hr";
        break;
    case 3:
        return "A week";
        break;
    case 4:
        return "A month";
        break;
    case 5:
        return "3 months";
        break;
    case 6:
        return "5 months";
        break;
    case 7:
        return "7 months";
        break;

    default:
        return "Inapp. Interval";
        break;
    }
}

string get_new_study_date(std::string study_date, int days)
{
    // Convert string to date, add/subtract days
    std::tm tm = {};
    std::istringstream ss(study_date);
    ss >> std::get_time(&tm, "%Y-%m-%d");

    std::chrono::system_clock::time_point date_tp = std::chrono::system_clock::from_time_t(std::mktime(&tm));
    date_tp += std::chrono::hours(24 * days); // Add/subtract days

    std::time_t new_time = std::chrono::system_clock::to_time_t(date_tp);
    std::tm *new_tm = std::localtime(&new_time);
    std::ostringstream new_date_stream;
    new_date_stream << std::put_time(new_tm, "%Y-%m-%d");
    std::string new_study_date = new_date_stream.str();
    // Converted
    return new_study_date;
}

Functions::Functions() : conn(nullptr) {}

Functions::~Functions() {}

void Functions::connect(const std::string &conninfo)
{
    try
    {
        conn = std::make_unique<pqxx::connection>(conninfo);
        if (conn->is_open())
        {
            std::cout << "Connected to DB: " << conn->dbname() << "\n";
        }
        else
        {
            std::cerr << "Connection failed.\n";
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Connection exception: " << e.what() << "\n";
    }
}

bool Functions::isConnected() const
{
    return conn && conn->is_open();
}

void Functions ::welcome_note()
{
    dotenv::init();
    std::string user = dotenv::getenv("NAME");
    cout << "\nUsername:\t" << user << endl;

    time_t timestamp = time(NULL);
    struct tm datetime = *localtime(&timestamp);
    char output[50];
    strftime(output, 50, "%a, %b %e, %Y", &datetime);
    cout << "Date:\t" << output;

    heading();
}

void Functions ::todays_tasks()
{
    try
    {
        pqxx::work backlog_txn(*conn);

        backlog_txn.exec("UPDATE list SET study_date = CURRENT_DATE "
                         "WHERE study_date < CURRENT_DATE AND completion_status = FALSE");

        backlog_txn.commit();

        pqxx::work txn(*conn);

        std::string query =
            "SELECT id, subject_name, topic, study_type, completion_status "
            "FROM list WHERE study_date = CURRENT_DATE ORDER BY id;";

        pqxx::result result = txn.exec(query);

        txn.commit();

        space();
        std::cout << setw(10) << left << "ID"
                  << setw(20) << left << "SUBJECT"
                  << setw(30) << left << "TOPIC"
                  << setw(10) << left << "INTERVAL"
                  << setw(10) << left << "RECALLED\n"
                  << endl;

        for (const auto &row : result)
        {
            space();
            std::cout << setw(10) << left << row["id"].as<int>()
                      << setw(20) << left << row["subject_name"].as<string>()
                      << setw(30) << left << row["topic"].as<string>()
                      << setw(10) << left << interval(row["study_type"].as<int>())
                      << (row["completion_status"].as<bool>() ? "Yes" : "No") << "\n";
        }

        cout << "\n";
    }
    catch (const std::exception &e)
    {
        std::cerr << "Today's Task - Error: " << e.what() << '\n';
    }
}

void Functions ::enter_options()
{
    cout << "\n\n";
    // space();
    // cout << "O   P   T   I   O   N   S\n\n";
    space();
    cout << "O P T I O N S  ->  1 : STATUS UPDATE     2 : ADD TOPIC     3 : SHIFT SCHEDULE\n";
    // space();
    // cout << "2 : ADD TOPIC\n";
    // space();
    // cout << "3 : SHIFT SCHEDULE\n";

    cout << endl;
    space();
    cout << "Enter Option: ";
}

void Functions ::task_completion_and_rewiew()
{
    try
    {
        char repeat = 'y';
        while (repeat != 'n')
        {
            int id = 0, review = 0;
            heading();
            todays_tasks();
            space();
            cout << "R A T E       Y O U R       R E V I S I O N\n\n";

            space();
            cout << "Id: ";
            cin >> id;
            clearInputBuffer();

            space();
            cout << "How was your revision? ... ";
            cin >> review;
            clearInputBuffer();

            // Update in database
            pqxx::work txn(*conn);

            txn.exec_params(
                "UPDATE list SET completion_status = TRUE, revision_rating = "
                "$1 WHERE id = $2",
                review, id);

            txn.commit();
            // Updated

            process_the_review(id, review);

            space();
            cout << "Continue - ";
            repeat = cin.get();
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Task Completion & Review Error: " << e.what() << '\n';
    }
}

void Functions ::process_the_review(int id, int review)
{
    // Use of utility_shift_one_entry
    // send id to fetch subject_name and topic - fetch subject_name and topic
    // for each study_type with the same details, update it - filter with study_type to update
    try
    {
        pqxx::work txn(*conn);
        pqxx::result extracted_data = txn.exec_params(
            "SELECT subject_name, topic from list WHERE id = $1", (id));
        txn.commit();

        std::string subject_name = extracted_data[0][0].as<std::string>();
        std::string topic = extracted_data[0][1].as<std::string>();
        // space();
        // cout << "Subject name and topic extraction test: " << subject_name << "\t" << topic << endl;

        switch (review)
        {
        // case 1:
        // increase 1 day for study_type 2 and 3 // scrap it - no change
        // utility_shift_one_entry_review_based(subject_name, topic, 2, 1);
        // utility_shift_one_entry_review_based(subject_name, topic, 3, 1);
        // break;
        case 2:
            // decrease 1 day for study_type 2 and 3, 3 days for study_type 4, 5 days for rest
            utility_shift_one_entry_review_based(subject_name, topic, 2, -1);
            utility_shift_one_entry_review_based(subject_name, topic, 3, -1);
            utility_shift_one_entry_review_based(subject_name, topic, 4, -3);
            utility_shift_one_entry_review_based(subject_name, topic, 5, -5);
            utility_shift_one_entry_review_based(subject_name, topic, 6, -5);
            utility_shift_one_entry_review_based(subject_name, topic, 7, -5);
            break;
        case 3:
            // decrease 1 day for study type 2, 2 days for study_type 3, 5 days for study_type 4, 10 days for rest
            utility_shift_one_entry_review_based(subject_name, topic, 2, -1);
            utility_shift_one_entry_review_based(subject_name, topic, 3, -2);
            utility_shift_one_entry_review_based(subject_name, topic, 4, -5);
            utility_shift_one_entry_review_based(subject_name, topic, 5, -10);
            utility_shift_one_entry_review_based(subject_name, topic, 6, -10);
            utility_shift_one_entry_review_based(subject_name, topic, 7, -10);
            break;

        default:
            break;
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Process the Review - Error: " << e.what() << '\n';
    }
}

void Functions ::add_subject_and_topic()
{
    try
    {
        string subject_name, topic;
        char repeat = 'y';
        while (repeat != 'n')
        {
            heading();
            space();
            cout << "A D D       T O P I C\n\n";
            space();
            cout << "Subject: ";
            getline(cin, subject_name);
            space();
            cout << "Topic: ";
            getline(cin, topic);

            pqxx::work txn(*conn);

            txn.exec_params(
                "INSERT INTO list (subject_name, topic, study_type, revision_rating, completion_status) "
                "VALUES ($1, $2, 0, 0, TRUE);",
                subject_name, topic);

            txn.commit();

            generate_srs(subject_name, topic);

            cout << "\n\n";

            space();
            cout << "Continue - ";
            repeat = cin.get();
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Task Completion & Review - Error: " << e.what() << '\n';
    }
}

void Functions ::generate_srs(string subject_name, string topic)
{
    try
    {
        // Intervals
        utility_add_one_entry(subject_name, topic, 1, 1);
        utility_add_one_entry(subject_name, topic, 3, 2);
        utility_add_one_entry(subject_name, topic, 7, 3);
        utility_add_one_entry(subject_name, topic, 28, 4);
        utility_add_one_entry(subject_name, topic, 28 * 3, 5);
        utility_add_one_entry(subject_name, topic, 28 * 5, 6);
        utility_add_one_entry(subject_name, topic, 28 * 7, 7);
    }
    catch (const std::exception &e)
    {
        std::cerr << "Generate SRS - Error: " << e.what() << '\n';
    }
}

void Functions ::utility_add_one_entry(string subject_name, string topic, int n, int study_type)
{
    // Partial copy of an entry
    try
    {
        // Extracting current_date
        pqxx::work date_txn(*conn);
        std::string date_query = "SELECT CURRENT_DATE";
        pqxx::result extracted_data = date_txn.exec(date_query);
        date_txn.commit();

        std::string study_date = extracted_data[0][0].as<std::string>();
        // space();
        // cout << "Date Test: " << study_date << endl;

        std::string new_study_date = get_new_study_date(study_date, n);

        pqxx::work txn(*conn);

        txn.exec_params(
            "INSERT INTO list (subject_name, topic, study_date, study_type) "
            "VALUES ($1, $2, $3, $4)",
            subject_name, topic, new_study_date, study_type);

        txn.commit();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Add One Entry " << n << " Stage - Error: " << e.what() << '\n';
    }
}

void Functions ::shift_whole_srs()
{
    // Probabale use of utility_shift_one_entry
    try
    {
        int n = 0;
        heading();
        space();
        cout << "S H I F T       S C H E D U L E\n\n";
        space();
        cout << "How many days? ... ";
        cin >> n;
        clearInputBuffer();

        pqxx::work txn(*conn);
        pqxx::result res = txn.exec("SELECT id, study_date FROM list WHERE study_date >= CURRENT_DATE");
        txn.commit();

        for (const auto &row : res)
        {
            int id = row["id"].as<int>();
            std::string study_date = row["study_date"].as<std::string>();
            std::string new_study_date = get_new_study_date(study_date, n);
            utility_shift_one_entry(id, new_study_date);
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Shift whole SRS - Error: " << e.what() << '\n';
    }
}

void Functions ::utility_shift_one_entry(int id, string new_study_date)
{
    // Partial update on an entry - date
    try
    {
        pqxx::work txn(*conn);

        txn.exec_params("UPDATE list SET study_date = $1 WHERE id = $2", new_study_date, id);

        txn.commit();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Shift One Entry - Error: " << e.what() << '\n';
    }
}

void Functions ::utility_shift_one_entry_review_based(string subject_name, string topic, int study_type, int n)
{
    // Partial update on an entry - date
    try
    {
        // Extracting current_date
        pqxx::work ext_txn(*conn);
        pqxx::result extracted_data = ext_txn.exec_params(
            "SELECT id, study_date FROM list WHERE study_date > CURRENT_DATE AND "
            "subject_name = $1 AND topic = $2 AND study_type = $3;",
            subject_name, topic, study_type);
        ext_txn.commit();

        std::string id = extracted_data[0][0].as<std::string>();
        std::string study_date = extracted_data[0][1].as<std::string>();

        std::string new_study_date = get_new_study_date(study_date, n);

        pqxx::work txn(*conn);

        txn.exec_params(
            "UPDATE list SET study_date = $1 WHERE id = $2",
            new_study_date, id);

        txn.commit();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Review based Shift " << n << " Stage - Error: " << e.what() << '\n';
    }
}
