# dnsperf

A prototype service to track DNS query latencies for top 10 Alexa sites.

## Dependencies

* MySQL Server : On Debian/Ubuntu systems, install using `apt-get install mysqlserver`.

* MySQL++ : A C++ wrapper library over MySQL C connector for easy of development and rich functionality for MySQL-related operations. On Debian/Ubuntu distributions it can be installed using `apt-get install libmysql++-dev`. Alternatively, on other systems if the distribution binaries are not available, a source tarball can be found in the project directory or the latest tarball can be downloaded from the [MySQL++](http://tangentsoft.net/mysql++/) website.

* ldns : A library with rich functionality for DNS-related operations. On Debian/Ubuntu distributions it can be installed using `apt-get install libldns-dev`. Alternatively, on other systems if the distribution binaries are not available, a source tarball can be found in the project directory or the latest tarball can be downloaded from the [ldns](https://www.nlnetlabs.nl/projects/ldns/) website.

## Defaults

* MySQL user: `dnsperf-admin`
* MySQL password: `dnspass`
* MySQL database: `dnsperfdb`

