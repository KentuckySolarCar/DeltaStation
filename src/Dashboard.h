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
        static constexpr char REFRESH_SYMBOLS[] = {'|', '/', '-', '\\'};
        std::unordered_map<std::string, double> lines{};
        int refresh_status = 0;

        friend class Dashboard;
    };

    std::unordered_map<std::string, Group> groups;
};

} // DS

#endif //DASHBOARD_H
