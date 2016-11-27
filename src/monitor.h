#include <mysql++.h>
#include <thread>
#include <mutex>

#ifndef DNS_PERF_MONITOR_H
#define DNS_PERF_MONITOR_H 1

class DNSPerfMonitor {

    private:
        bool running;

        std::mutex db_mutex;
        mysqlpp::Connection connection;

        int interval;
        std::vector<std::string> domains;
        std::string db_name;
        std::string db_user;
        std::string db_pass;
        std::string db_host;
        std::map<std::string, int> record_count_map;
        std::map<std::string, float> mean_latency_map;
        std::map<std::string, float> std_dev_map;

    public:
        DNSPerfMonitor(int, std::string, std::string, std::string, std::string, std::vector<std::string>);

        ~DNSPerfMonitor();

        void init();

        void shutdown();

        void run();

        bool is_running();

        int get_interval();
        
        std::vector<std::string> get_domains();

        void update_dns_latency_records(std::string, int);
};

#endif
