//#include "cmdline.h"
//#include "printdata.h"

#include <mysql++.h>

#include <iostream>
#include <iomanip>

using namespace std;

int
main(int argc, char *argv[])
{
    // Get database access parameters from command line
    //mysqlpp::examples::CommandLine cmdline(argc, argv);
    //if (!cmdline) {
    //    return 1;
    //}

    // Connect to the sample database.
    mysqlpp::Connection conn(false);
    if (conn.connect("dnsperfdb", "127.0.0.1",
            "dnsperf-admin", "dnspass")) {
        // Retrieve a subset of the sample stock table set up by resetdb
        // and display it.
        mysqlpp::Query query = conn.query("select * from test;");
        if (mysqlpp::StoreQueryResult res = query.store()) {
            cout << "We have:" << endl;
            mysqlpp::StoreQueryResult::const_iterator it;
            for (it = res.begin(); it != res.end(); ++it) {
                mysqlpp::Row row = *it;
                cout << row[0] << row[1] << endl;
            }
        }
        else {
            cerr << "Failed to get item list: " << query.error() << endl;
            return 1;
        }

        return 0;
    }
    else {
        cerr << "DB connection failed: " << conn.error() << endl;
        return 1;
    }
}
