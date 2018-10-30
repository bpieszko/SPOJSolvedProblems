#include <iostream>
#include <string>
#include <vector>
#include <set>
#include <map>
#include <curl/curl.h>

static size_t WriteCallback(void * contents, size_t size, size_t nmemb, void * userp) {
	((std::string *) userp)->append((char *) contents, size * nmemb);
	return size * nmemb;
}

std::set<std::string> get_problems_solved_by_user(std::string username) {
    CURL * curl;
    CURLcode res;
    std::string read_buffer;
    curl = curl_easy_init();

    std::set<std::string> problems;

    if (curl) {
        std::string address = "https://pl.spoj.com/users/" + username + "/";

        curl_easy_setopt(curl, CURLOPT_URL, address.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &read_buffer);

        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        size_t it_beg = 0;
        size_t it_end = 0;

        while (1) {
            it_beg = read_buffer.find("<a href=\"/status/", it_beg + 1);
            it_end = read_buffer.find("</a>", it_beg + 1);

            if (it_beg >= read_buffer.size() || it_beg < 0)
                break;

            std::string result = read_buffer.substr(it_beg, it_end - it_beg);

            size_t it_problem_beg = result.find_last_of(">");
            size_t it_problem_end = result.find("\0");

            std::string problem_name = result.substr(it_problem_beg + 1, it_problem_end + 1 - it_problem_beg);
            
            if (problem_name != "Historia zgłoszeń" && problem_name != "wersja tekstowa")
                problems.insert(problem_name);
        }
    }
    return problems;
}

std::set<std::string> get_problems(std::string level) {
    std::set<std::string> problems;
    int counter = 0;

    while (counter <= 350) {
        CURL * curl;
        CURLcode res;
        std::string read_buffer;
        curl = curl_easy_init();

        if (curl) {
            std::string address = "https://pl.spoj.com/problems/" + level + "/" + "sort=0,start=" + std::to_string(counter);

            curl_easy_setopt(curl, CURLOPT_URL, address.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &read_buffer);

            res = curl_easy_perform(curl);
            curl_easy_cleanup(curl);

            size_t it_beg = 0;
            size_t it_end = 0;

            while (1) {
                it_beg = read_buffer.find("<a href=\"/submit/", it_beg + 1);
                it_end = read_buffer.find("\" title=\"", it_beg + 1);

                if (it_beg >= read_buffer.size() || it_beg < 0)
                    break;

                std::string problem_name = read_buffer.substr(it_beg + 17, it_end - 17 - it_beg);
                problems.insert(problem_name);
            }
        }
        counter += 50;
    }
    return problems;
}

int main (int argc, char * argv[]) {
    if (argc != 2) {
        std::cerr << "Usage:" << std::endl;
        std::cerr << "\t./SPOJSolvedProblems \"user_name\"" << std::endl;
        return 0;
    }
    std::string username = argv[1];

    std::set<std::string> solved_by_user = get_problems_solved_by_user(username);

    std::set<std::string> problems[4]{
        get_problems("latwe"),
        get_problems("srednie"),
        get_problems("trudne"),
        get_problems("challenge")
    };

    int solved_counter[4]{0, 0, 0, 0};

    for (int i = 0; i < 4; ++i)
        for (auto problem : solved_by_user)
            if (problems[i].find(problem) != problems[i].end())
                ++solved_counter[i];

    const std::map<int, std::string> level_name{
        {0, "Easy"},
        {1, "Medium"},
        {2, "Hard"},
        {3, "Challenge"}
    };

    for (int i = 0; i < 4; ++i)
        std::cout << level_name.find(i)->second << ": " << solved_counter[i] << std::endl;

    return 0;
}