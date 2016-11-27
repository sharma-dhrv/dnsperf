#!/bin/bash

#####################################
#           DRIVER FILE             #
#####################################
#DB_NAME="dnsperfdb"
#DB_USER="dnsperf-admin"
#DB_PASSWORD="dnspass"
#DB_HOST="127.0.0.1"
ENV_FILE="dnsperf.env"
EXEC_FILE="dnsperf"
DEFAULT_DOMAINS_FILE="domains.lst"

if [ ! -e "$ENV_FILE" ]; then
    echo "DNSPerf environment variable file '$ENV_FILE' does not exist."
    exit 5
fi    

source "$ENV_FILE"

script_name=$0
action=$1

case "$action" in
    
    "run")
        shift 1
        query_interval=10
        domains_file=$DEFAULT_DOMAINS_FILE

        echo "Building executable '$EXEC_FILE'..."
        make

        if [ ! -e "$EXEC_FILE" ]; then
            echo "The executable '$EXEC_FILE' does not exist!"
            echo "Build failed."
            exit 6
        else
            echo "Build successful."
            echo ""
        fi

        if [ $# -lt 1 ]; then
            echo "Missing parameters for action 'run'"
            echo "Usage: $script_name run <Query Interval (secs)> <Domain Names File (default: $DEFAULT_DOMAINS_FILE)>"
            exit 7
        elif [ $# -eq 1 ]; then
            query_interval=$1
        else
            query_interval=$1
            domains_file=$2
        fi

        if [ $query_interval -le 0 ]; then
            echo "DNS Query Interval '$query_interval sec(s)' is invalid."
            exit 8
        fi

        if [ ! -e "$domains_file" ]; then
            echo "Domain Names File '$domains_file' doesn't exist."
            exit 9
        fi

        echo "Starting DNSPerf..."
        DNSPERF_DB_NAME=$DB_NAME DNSPERF_DB_USER=$DB_USER DNSPERF_DB_PASS=$DB_PASSWORD DNSPERF_DB_HOST=$DB_HOST "./$EXEC_FILE" $query_interval "$domains_file"

        exit $?
        ;;

    "show-schema")
        echo "'DomainSummary' Table Schema:-"
        mysql -u $DB_USER -p$DB_PASSWORD -h $DB_HOST -e "DESCRIBE DomainSummary;" -D $DB_NAME
        echo ""
        echo "'LatencyRecords' Table Schema:-"
        mysql -u $DB_USER -p$DB_PASSWORD -h $DB_HOST -e "DESCRIBE LatencyRecords;" -D $DB_NAME
        exit $?
        ;;

    "show-summary")
        echo "'DomainSummary' Table Entries:-"
        mysql -u $DB_USER -p$DB_PASSWORD -h $DB_HOST -e "SELECT domain_name AS 'Domain Name', record_count AS 'Total Records', mean_latency AS 'Mean Latency (usecs)', std_dev AS 'Latency Standard Deviation (usecs)', first_update_time AS 'First Update Time', last_update_time AS 'Last Update Time' FROM DomainSummary;" -D $DB_NAME
        exit $?
        ;;

    "show-details")
        echo "'DomainSummary' Table Entries:-"
        mysql -u $DB_USER -p$DB_PASSWORD -h $DB_HOST -e "SELECT D.domain_name AS 'Domain Name', L.latency AS 'Latency (usecs)', L.query_time AS 'DNS Query Time' FROM LatencyRecords L, DomainSummary D WHERE D.id = L.domain_id;" -D $DB_NAME
        exit $?
        ;;

    "create-db")
        mysql -u $DB_USER -p$DB_PASSWORD -h $DB_HOST -e "CREATE DATABASE IF NOT EXISTS $DB_NAME;"
        exit $?
        ;;

    "remove-db")
        mysql -u $DB_USER -p$DB_PASSWORD -h $DB_HOST -e "DROP TABLE IF EXISTS LatencyRecords;" -D $DB_NAME
        mysql -u $DB_USER -p$DB_PASSWORD -h $DB_HOST -e "DROP TABLE IF EXISTS DomainSummary;" -D $DB_NAME
        mysql -u $DB_USER -p$DB_PASSWORD -h $DB_HOST -e "DROP DATABASE IF EXISTS $DB_NAME;"
        exit $?
        ;;

    "clean")
        make clean
        ;;

    *)
        echo "A DNS query latency monitoring tool for given set of domains (Eg: Top 10 Alexa Domains)"
        echo "Usage: $script_name [ run <Query Interval (secs)> <Domain Names File (default: domains.lst)> | show-schema | show-summary | show-details | create-db | remove-db | clean]"
        ;;

esac

exit 0
