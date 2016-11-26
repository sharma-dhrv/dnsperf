#include <cstdlib>
#include <thread>
#include "monitor.h"

DNSQueryMonitor::DNSQueryMonitor() {
}

DNSQueryMonitor::DNSQueryMonitor(int interval, std::string db_name, std::string db_user, std::string db_pass, std::string db_host, std::vector<std::string> domains) {
    this->connection = mysqlpp::Connection(true);
    this->interval = interval;
    this->db_name = db_name;
    this->db_user = db_user;
    this->db_pass = db_pass;
    this->db_host = db_host;
    this->domains = domains;
    this->running = false;
}

DNSQueryMonitor::~DNSQueryMonitor() {
    this->shutdown();
}

void DNSQueryMonitor::shutdown() {
    if (this->connection.connected()) {
        this->connection.disconnect();
    }
    this->running = false;
}


bool DNSQueryMonitor::is_running() {
    return this->running;
}

int DNSQueryMonitor::get_interval() {
    return this->interval;
}


std::vector<std::string> DNSQueryMonitor::get_domains() {
    return this->domains;
}


void DNSQueryMonitor::init() {
    if (this->connection.connect(this->db_name.c_str(), this->db_host.c_str(), this->db_user.c_str(), this->db_pass.c_str())) {

        try {
            std::cout << "Creating Table 'DomainSummary' if one does not exist..." << std::endl;
            mysqlpp::Query query = this->connection.query("CREATE TABLE IF NOT EXISTS DomainSummary (id INT AUTO_INCREMENT PRIMARY KEY, domain_name VARCHAR(30), record_count INT DEFAULT 0, mean_latency FLOAT DEFAULT 0.0, std_dev FLOAT DEFAULT 0.0 );");
            mysqlpp::SimpleResult res = query.execute();
            std::cout << "Success : " << res.info() << std::endl;
        }
        catch (mysqlpp::BadQuery e) {
            std::cerr << "Failed to create Table 'DomainSummary' : " << e.what() << std::endl;
            return;
        }

        try {
            std::cout << "Creating Table 'LatencyRecords' if one does not exist..." << std::endl;
            mysqlpp::Query query = this->connection.query("CREATE TABLE IF NOT EXISTS LatencyRecords (id INT AUTO_INCREMENT PRIMARY KEY, domain_id INT NOT NULL, latency FLOAT DEFAULT 0.0);");
            mysqlpp::SimpleResult res = query.execute();
            std::cout << "Success : " << res.info() << std::endl;
        }
        catch(mysqlpp::BadQuery e) { 
            std::cerr << "Failed to create Table 'LatencyRecords' : " << e.what() << std::endl;
            return;
        }

        try {
            mysqlpp::Query query = this->connection.query("SELECT * FROM DomainSummary;");
        	mysqlpp::StoreQueryResult res = query.store();
            if(res.num_rows() > 0) {
                std::cout << "Syncing domain statistics from the database..." << std::endl;
            	mysqlpp::StoreQueryResult::const_iterator it;
            	for (it = res.begin(); it != res.end(); ++it) {
                	mysqlpp::Row row = *it;
                	std::string domain_name = std::string (row[1]);
                	
                    int record_count = std::stoi(std::string(row[2]));
                    this->record_count_map.insert(std::pair<std::string, int>(domain_name, record_count));
                	float mean_latency = std::stof(std::string(row[3]));
                    this->mean_latency_map.insert(std::pair<std::string, float>(domain_name, mean_latency));
                	float std_dev = std::stof(std::string(row[4]));
                    this->std_dev_map.insert(std::pair<std::string, float>(domain_name, std_dev));
            	}
        	}
        	else {
                std::cout << "Initializing default domain statistics in the database..." << std::endl;
                for (std::vector<std::string>::iterator it = domains.begin() ; it != this->domains.end(); ++it) {
                    std::string domain_name = *it;
                    try {
                        std::stringstream query_str;
                        query_str << "INSERT INTO DomainSummary VALUES (DEFAULT, \"" << domain_name << "\", DEFAULT, DEFAULT, DEFAULT);";
                        mysqlpp::Query query = this->connection.query(query_str.str());
                        mysqlpp::SimpleResult res = query.execute();
                        
                        this->record_count_map.insert(std::pair<std::string, int>(domain_name, 0));
                        this->mean_latency_map.insert(std::pair<std::string, float>(domain_name, 0.0));
                        this->std_dev_map.insert(std::pair<std::string, float>(domain_name, 0.0));
                    }
                    catch(mysqlpp::BadQuery e) {
                        std::cerr << "Failed to add domain '" << domain_name << "' to the table 'DomainSummary' (will be skipped) : " << query.error() << std::endl;
                    }
                }
        	}
        }
        catch(mysqlpp::BadQuery e) {
            std::cerr << "Failed to fetch domain's DNS performance summary from table 'DomainSummary' : " << e.what() << std::endl;
            return;
        }
    }
    else {
        std::cerr << "Failed to connect to the database '" << this->db_name << "' on host '" << this->db_host << "'." << std::endl;
        return;
    }
}


void send_dns_query(std::string domain, DNSQueryMonitor *monitor_ptr) {
// TODO: make actual dns query and report value to monitor function
}


void run_periodic_dns_queries(DNSQueryMonitor *monitor_ptr) {

    while(monitor_ptr->is_running()) {

        std::vector<std::thread> dns_queries;

        std::vector<std::string> domains = monitor_ptr->get_domains();
        for(std::vector<std::string>::iterator it = domains.begin(); it != domains.end(), ++it) {
            
            std::thread dns_query_thread = std::thread(run_periodic_dns_queries, this);
            dns_queries.push_back(dns_query_thread);
        }

        for(std::vector<std::thread>::iterator it = dns_queries.begin(); it != dns_queries.end(); ++it) {
            std::thread dns_query = *it;
            dns_query.join();
        }

        std::chrono::seconds refresh_interval(monitor_ptr->get_interval());
        std::this_thread::sleep_for(refresh_interval);
    }

}


void DNSQueryMonitor::run() {

    this->running = true;
    std::thread monitoring_thread = std::thread(run_periodic_dns_queries, this);

    monitoring_thread.join();

    this->shutdown();
}
