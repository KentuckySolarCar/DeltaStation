/* date = April 04, 2025 11:38 AM */


#ifndef DASHBOARD_H
#define DASHBOARD_H
#include <string>
#include <unordered_map>

namespace DS {

class Dashboard {
public:
    Dashboard();

private:
    class Group {
        std::unordered_map<std::string, double> lines{};
        char refresh_status = '|';
    };

    std::unordered_map<std::string, Group> groups;
};

} // DS

#endif //DASHBOARD_H
