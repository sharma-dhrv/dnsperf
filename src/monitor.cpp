/**
  * @author Dhruv Sharma (dhsharma@cs.ucsd.edu)
  */


#include <sstream>
#include <cstdlib>
#include <thread>
#include "monitor.h"
#include <ctime>
#include <ldns.h>
#include <cmath>

/**
  * Set of alpha-numeric characters
  */
static const char alphanum[] =
"0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

/**
  * Function to generate a random 8-character string
  */
std::string
gen_random_prefix() {
    std::ostringstream os;
    for (int i = 0; i < 8; i++) {
        os << alphanum[rand() % (sizeof(alphanum) - 1)];
    }

    return os.str();
}

/**
  * DNSPerfMonitor class constructor
  */
DNSPerfMonitor::DNSPerfMonitor(int interval, std::string db_name, std::string db_user, std::string db_pass, std::string db_host, std::vector<std::string> domains) {
    this->connection = mysqlpp::Connection(true);
    this->interval = interval;
    this->db_name = db_name;
    this->db_user = db_user;
    this->db_pass = db_pass;
    this->db_host = db_host;
    this->domains = domains;
    this->running = false;
}


/**
  * DNSPerfMonitor class destructor
  */
DNSPerfMonitor::~DNSPerfMonitor() {
    this->shutdown();
}


/**
  * Function to shutdown DNSPerf Monitor's operations
  */
void
DNSPerfMonitor::shutdown() {
    
    this->running = false;
    
    if (this->connection.connected()) {
        this->connection.disconnect();
    }
}


/**
  * Function to check if DNSPerf Monitor is running or not.
  */
bool
DNSPerfMonitor::is_running() {
    return this->running;
}


/**
  * Function to get the DNS query interval.
  */
int
DNSPerfMonitor::get_interval() {
    return this->interval;
}


/**
  * Function to get the domain names for which DNS query latencies are
  * being monitored.
  */
std::vector<std::string>
DNSPerfMonitor::get_domains() {
    return this->domains;
}


/**
  * Function to initialize the DNSPerf Monitor's internal state such as 
  * database connection, local data, etc.
  */
void
DNSPerfMonitor::init() {

    // seed random number generator for later use
    srand(time(0));

    // connect to database
    std::cout << "Trying connection to database '" << db_name << "' @ '" << db_host << "'... ";
    try {
        if (this->connection.connect(this->db_name.c_str(), this->db_host.c_str(), this->db_user.c_str(), this->db_pass.c_str())) {

            std::cout << "Success!" << std::endl;
            // create tables with defaults if they don't exist; sync data from pre-existing tables otherwise.
            try {
                std::cout << "Creating Table 'DomainSummary' if one does not exist... ";
                mysqlpp::Query query = this->connection.query("CREATE TABLE IF NOT EXISTS DomainSummary ("
                    "id INT AUTO_INCREMENT PRIMARY KEY,"
                    "domain_name VARCHAR(30),"
                    "record_count INT DEFAULT 0,"
                    "mean_latency FLOAT(12,3) DEFAULT 0.0,"
                    "std_dev FLOAT(12,3) DEFAULT 0.0,"
                    "first_update_time DATETIME DEFAULT NULL,"
                    "last_update_time DATETIME DEFAULT NULL"
                    ");");

                mysqlpp::SimpleResult res = query.execute();
                std::cout << "Success!" << std::endl;
            }
            catch (mysqlpp::BadQuery e) {
                std::cout << "Failure!" << std::endl;
                std::cerr << "Failed to create Table 'DomainSummary' : " << e.what() << std::endl;
                std::cout << "Terminated." << std::endl;
                exit(5);
            }

            try {
                std::cout << "Creating Table 'LatencyRecords' if one does not exist... ";
                mysqlpp::Query query = this->connection.query("CREATE TABLE IF NOT EXISTS LatencyRecords ("
                    "id INT AUTO_INCREMENT PRIMARY KEY,"
                    "domain_id INT NOT NULL,"
                    "latency FLOAT(12,3) DEFAULT 0.0,"
                    "query_time DATETIME DEFAULT CURRENT_TIMESTAMP,"
                    "FOREIGN KEY (domain_id) REFERENCES DomainSummary(id) ON DELETE RESTRICT ON UPDATE CASCADE"
                    ");");

                mysqlpp::SimpleResult res = query.execute();
                std::cout << "Success!" << std::endl;
            }
            catch(mysqlpp::BadQuery e) {
                std::cout << "Failure!" << std::endl;
                std::cerr << "Failed to create Table 'LatencyRecords' : " << e.what() << std::endl;
                std::cout << "Terminated." << std::endl;
                exit(5);
            }

            
            // sync monitoring state i.e. domain summary with the database
            std::cout << "Looking for existing domain statistics in the database... ";
            try {
                mysqlpp::Query query = this->connection.query("SELECT * FROM DomainSummary;");
                mysqlpp::StoreQueryResult res = query.store();
                if(res.num_rows() > 0) {
                    std::cout << "Found!" << std::endl;
                    std::cout << "Pulling existing domain statistics from the database... ";
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
                    
                    std::cout << "Success!" << std::endl;
                }
                else {
                    std::cout << "Not Found!" << std::endl;
                    std::cout << "Initializing with default domain statistics in the database... ";
                    try {
                        for (std::vector<std::string>::iterator it = domains.begin() ; it != this->domains.end(); ++it) {
                            std::string domain_name = *it;
                            std::stringstream query_str;
                            query_str << "INSERT INTO DomainSummary VALUES (DEFAULT, \"" << domain_name << "\", DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT);";
                            mysqlpp::Query query = this->connection.query(query_str.str());
                            mysqlpp::SimpleResult res = query.execute();
                            
                            this->record_count_map.insert(std::pair<std::string, int>(domain_name, 0));
                            this->mean_latency_map.insert(std::pair<std::string, float>(domain_name, 0.0));
                            this->std_dev_map.insert(std::pair<std::string, float>(domain_name, 0.0));
                        }
                    
                        std::cout << "Success!" << std::endl;
                    }
                    catch(mysqlpp::BadQuery e) {
                        std::cout << "Failure!" << std::endl;
                        std::cerr << "Failed to add domains to the table 'DomainSummary' (will be skipped) : " << e.what() << std::endl;
                    }
                }
            }
            catch(mysqlpp::BadQuery e) {
                std::cout << "Failure!" << std::endl;
                std::cerr << "Failed to fetch domain's DNS performance summary from table 'DomainSummary' : " << e.what() << std::endl;
                std::cout << "Terminated." << std::endl;
                exit(5);
            }
        }
        else {
            std::cout << "Failure!" << std::endl;
            std::cerr << "Failed to connect to the database '" << this->db_name << "' on host '" << this->db_host << "'." << std::endl;
            std::cout << "Terminated." << std::endl;
            exit(5);
        }
    }
    catch(mysqlpp::ConnectionFailed e) {
        std::cout << "Failure!" << std::endl;
        std::cerr << "Failed to connect to the database '" << this->db_name << "' on host '" << this->db_host << "' : " << e.what() << std::endl;
        std::cout << "Terminated." << std::endl;
        exit(5);
    }
}


/**
  * Function to update local and database records with the latest measure of
  * DNS query latency measure for a domain.
  */
void
DNSPerfMonitor::update_dns_latency_records(std::string domain_name, int latency) {
    int current_record_count;
    int new_record_count;
    float current_mean_latency;
    float new_mean_latency;
    float current_std_dev;
    float new_std_dev;

    // begin critical section; compute and update local state and update database via the shared connection.
    this->db_mutex.lock();
    
    std::map<std::string, int>::iterator count_it = this->record_count_map.find(domain_name);
    if (count_it != this->record_count_map.end()) {
        current_record_count = count_it->second;
        new_record_count = current_record_count + 1;
        this->record_count_map[domain_name] = new_record_count;
    }

    std::map<std::string, float>::iterator mean_it = this->mean_latency_map.find(domain_name);
    if (mean_it != this->mean_latency_map.end()) {
        current_mean_latency = mean_it->second;
        new_mean_latency = (current_mean_latency * current_record_count + latency) / new_record_count;
        this->mean_latency_map[domain_name] = new_mean_latency;
    }
    
    std::map<std::string, float>::iterator std_dev_it = this->std_dev_map.find(domain_name);
    if (std_dev_it != this->std_dev_map.end()) {
        current_std_dev = std_dev_it->second;
        if (current_record_count > 1) {
            new_std_dev = sqrt( ( ((current_record_count - 1) * current_std_dev * current_std_dev) + ((latency - current_mean_latency) * (latency - new_mean_latency)) ) / (new_record_count - 1) );
        }
        else {
            new_std_dev = 0.0;
        }
        this->std_dev_map[domain_name] = new_std_dev;
    }

    // update database
    try {
		std::stringstream query_str;
		query_str << "UPDATE DomainSummary " <<
            "SET record_count = " << new_record_count <<
            ", mean_latency = " << new_mean_latency <<
            ", std_dev = " << new_std_dev;
        if (new_record_count == 1) {
            query_str << ", first_update_time = CURRENT_TIMESTAMP";
        }
        query_str << ", last_update_time = CURRENT_TIMESTAMP " <<
            "WHERE domain_name=\"" << domain_name << "\";";

        mysqlpp::Query query = this->connection.query(query_str.str());
		mysqlpp::SimpleResult res = query.execute();
	}
	catch(mysqlpp::BadQuery e) {
		std::cerr << "Failed to update summary for domain '" << domain_name << "' in the table 'DomainSummary' : " << e.what() << std::endl;
        std::cerr << "Skipping this round of update for domain '" << domain_name << "'." << std::endl;
	}

    try {
		std::stringstream query_str;
		query_str << "INSERT INTO LatencyRecords VALUES(" <<
            "DEFAULT, " <<
            "(SELECT id FROM DomainSummary WHERE domain_name=\"" << domain_name << "\"), " <<
            latency <<
            ", DEFAULT);";

        mysqlpp::Query query = this->connection.query(query_str.str());
		mysqlpp::SimpleResult res = query.execute();
	}
	catch(mysqlpp::BadQuery e) {
		std::cerr << "Failed to add latency record for '" << domain_name << "' in the table 'LatencyRecords' : " << e.what() << std::endl;
        std::cerr << "Skipping this round of latency record for domain '" << domain_name << "'." << std::endl;
	}

    // end of critical section
    this->db_mutex.unlock();

}


/**
  * Function (run as thread) to make actual DNS query for a given domain name
  * and measure the latency.
  */
void
send_dns_query(std::string domain_name, DNSPerfMonitor *monitor_ptr) {
    ldns_resolver *res;
    ldns_rdf *domain;
    ldns_pkt *p;
    ldns_rr_list *a_recs;
    ldns_status s;

    p = NULL;
    a_recs = NULL;
    domain = NULL;
    res = NULL;

    // create pseudo sub-domain using random prefix to avoid DNS caching
    std::ostringstream os;
    os << gen_random_prefix() << "." << domain_name;
    std::string prefixed_domain_name = os.str();

    // create a rdf from the domain name provided
    domain = ldns_dname_new_frm_str(prefixed_domain_name.c_str());

    // create a new resolver from /etc/resolv.conf
    s = ldns_resolver_new_frm_file(&res, NULL);

    if (s != LDNS_STATUS_OK) {
	    std::cerr << "Failed to create DNS resolver." << std::endl;
        return;
    }

    std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();

    // use the resolver to send a query for the DNS Type A records of the domain
    p = ldns_resolver_query(res,
                            domain,
                            LDNS_RR_TYPE_A,
                            LDNS_RR_CLASS_IN,
                            LDNS_RD);


    std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
    
    int latency = std::chrono::duration_cast<std::chrono::microseconds>(end-start).count();

    ldns_rdf_deep_free(domain);

	if (!p)  {
	    std::cerr << "Failed to receive a DNS reply for domain " << domain_name << std::endl;
	}
	else {
         // retrieve the DNS Type A records from the answer section of that packet
        a_recs = ldns_pkt_rr_list_by_type(p,
                                      LDNS_RR_TYPE_A,
                                      LDNS_SECTION_ANSWER);
	}

	if(!p) {
        ldns_pkt_free(p);
    }
    if(!a_recs) {
        ldns_rr_list_deep_free(a_recs);
    }
	ldns_resolver_deep_free(res);

    monitor_ptr->update_dns_latency_records(domain_name, latency);

}


/**
  * Function (run as thread) to issue periodic DNS queries for given domains.
  */
void
run_periodic_dns_queries(DNSPerfMonitor *monitor_ptr) {

    std::cout << "DNSPerf is running." << std::endl;

    while(monitor_ptr->is_running()) {

        std::vector<std::thread> dns_queries;

        std::vector<std::string> domains = monitor_ptr->get_domains();
        for(std::vector<std::string>::iterator it = domains.begin(); it != domains.end(); ++it) {
            std::string domain_name = *it;
            dns_queries.push_back(std::thread(send_dns_query, domain_name, monitor_ptr));
        }

        for(std::thread& dns_query : dns_queries) {
            dns_query.join();
        }

        std::chrono::seconds refresh_interval(monitor_ptr->get_interval());
        std::this_thread::sleep_for(refresh_interval);
    }

}


/**
  * Function to start the primary monitoring thread.
  */
void
DNSPerfMonitor::run() {

    this->running = true;
    std::thread monitoring_thread = std::thread(run_periodic_dns_queries, this);
    monitoring_thread.join();
}
