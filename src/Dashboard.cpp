/* date = April 04, 2025 11:38 AM */


#include "Dashboard.h"

namespace DS {
    Dashboard::Dashboard() {
        auto mtA = Group();
        groups.insert(std::make_pair("mtA", mtA));

        groups.insert(std::make_pair("mtB", Group()));
        groups.insert(std::make_pair("gps", Group()));
        groups.insert(std::make_pair("arr", Group()));
        groups.insert(std::make_pair("bat", Group()));
        groups.insert(std::make_pair("drv", Group()));
        groups.insert(std::make_pair("sta", Group()));
        groups.insert(std::make_pair("imu", Group()));
    }
} // DS