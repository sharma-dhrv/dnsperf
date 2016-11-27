#include <iostream>
#include <fstream>
#include <cstdlib>
#include <signal.h>
#include "monitor.h"
#include <chrono>
#include <thread>
#include <ldns.h>

DNSPerfMonitor *monitor_ptr;

void sig_handler(int signal) {
    monitor_ptr->shutdown();
    std::cout << "Initiating shutdown..." << std::endl;
}

std::vector<std::string> parse_domains(std::string filename) {
    std::ifstream fin(filename.c_str());
    std::vector<std::string> domains;
    std::string domain;
    while(getline(fin, domain)) {
        domains.push_back(domain);
    }

    return domains;
}

int main(int argc, char **argv) {

    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <Refresh Interval (msecs)> <Domain Names (optional)>" << std::endl;
    }

    int refresh_interval;
    std::string arg_str (argv[1]);
    refresh_interval = std::stoi(arg_str);

    std::string domains_filename("domains.lst");
    if(argc >= 3) {
        domains_filename = std::string(argv[2]);
    }
    std::vector<std::string> domains = parse_domains(domains_filename);

    std::string db_name;
    if(const char* env_db_name = std::getenv("DNSPERF_DB_NAME")) {
        db_name = std::string (env_db_name);
    }
    
    std::string db_user;
    if(const char* env_db_user = std::getenv("DNSPERF_DB_USER")) {
        db_user = std::string (env_db_user);
    }
    
    std::string db_pass;
    if(const char* env_db_pass = std::getenv("DNSPERF_DB_PASS")) {
        db_pass = std::string (env_db_pass);
    }
    
    std::string db_host;
    if(const char* env_db_host = std::getenv("DNSPERF_DB_HOST")) {
        db_host = std::string (env_db_host);
    }

    DNSPerfMonitor monitor(refresh_interval, db_name, db_user, db_pass, db_host, domains);
    monitor_ptr = &monitor;

	struct sigaction sigIntHandler;

	sigIntHandler.sa_handler = sig_handler;
	sigemptyset(&sigIntHandler.sa_mask);
	sigIntHandler.sa_flags = 0;
	sigaction(SIGINT, &sigIntHandler, NULL);

    monitor.init();

	monitor.run();

    std::cout << "DNSPerf has shutdown. Bye!" << std::endl;
    
    return 0;
}
