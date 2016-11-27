# DNSPerf

A prototype service to track DNS query latencies for top 10 Alexa domains.

## Dependencies

* MySQL Server : On Debian/Ubuntu systems, install using `apt-get install mysqlserver`.

* MySQL++ : A C++ wrapper library over MySQL C connector for easy of development with rich functionality for MySQL-related operations. On Debian/Ubuntu distributions it can be installed using `apt-get install libmysql++-dev`. Alternatively, on other systems if the distribution binaries are not available, a source tarball can be found in the project directory or the latest tarball can be downloaded from the [MySQL++](http://tangentsoft.net/mysql++/) website.

* ldns : A library with rich functionality for DNS-related operations. On Debian/Ubuntu distributions it can be installed using `apt-get install libldns-dev`. Alternatively, on other systems if the distribution binaries are not available, a source tarball can be found in the project directory or the latest tarball can be downloaded from the [ldns](https://www.nlnetlabs.nl/projects/ldns/) website.

NOTE: If any of these dependencies are installed on custom locations, you might have to change these paths in `Makefile` and/or `driver.sh` files in the project's root(`.`) directory and `src` directory accordingly.

## Defaults

* MySQL User: `dnsperf-admin`
* MySQL Password: `dnspass`
* MySQL Database: `dnsperfdb`
* MySQL Host: `127.0.0.1`
* Database-related Environment Variables File: dnsperf.env
* Domains File: domains.lst

NOTE: MySQL User, MySQL Password, MySQL Database and MySQL Host (above) are sourced as environment variables from `dnsperf.env` by `driver.sh`. Therefore, these must be set once in the environment variables file. It is highly recommended that password field should not be written to file or sourced as environment variable in unencrypted form. Ideally, there are secure ways to access it but for the current scope of the project, it is deferred as future work.

## How To Run

An ideal sequence of steps for using `DNSPerf` are shown using a test run as follows:-

* Understand the usage description, create a database and start `DNSPerf`.
"
./driver.sh
./driver.sh create-db
./driver.sh run 10
"

* Press <Ctrl+C> (<Command+C>) after `DNSPerf` has run for some intervals. However, instead of terminating, you could run the following `./driver.sh show-*` commands simultaneously to view the progress while `DNSPerf` is running.
"
./driver.sh show-schema
./driver.sh show-summary
./driver.sh show-details
"

* Finally, once DNSPerf has terminated, clean-up the database and binaries.
"
./driver.sh remove-db
./driver.sh clean
"

NOTE: Display headers of `show-summary` and `show-details` are modified for readability using field aliasing and table joins.

## Test Platform

The project has been tested successfully on a system with following configuration:-

* Operating System: Ubuntu 16.04.1 LTS (x86-64 GNU Linux 4.4.0-47-generic)
* MySQL: MySQL Server and Client v5.6.17
* MySQL++ library: MySQL++ v3.2.2
* LDNS library: ldns v1.6.17
* Shell: GNU bash v4.3.46(1)-release
* C++ Compiler: GNU C++ (g++) v5.4.0
