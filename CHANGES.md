# Release Notes
## 0.4.0 - September 18 2020
- Update for compatibility with Zeek 3.2. Further changes will likely be needed for Zeek 4.1.
- Avoid runtime deprecation warning in zeekctl load balancing plugin.
- Update installation instructions to Zeek requirements.
- Rename package to zeek-dag.

## 0.3.1 - April 5 2019
- Update bro-pkg metadata to support building with bro 2.6 devel packages.
- bro-config must now be in path for bro-pkg installation to work.
- Improve installation instructions.

## 0.3.0 - December 5 2018
- Bugfix for Bro 2.6 release compatibility
- Update plugin configure template for Bro 2.6 to support building with minimal bro devel packages

## 0.2.0 - July 18 2018
- Improved accuracy of packets dropped statistics on physical DAG cards.
- Fix CMake generated RPM package (make package).
- Fix error message during capture not printing.

## 0.1.5 - May 1 2018
- Fix only successfully capturing on stream 0. Error message does not print due to bro bug.

## 0.1.4 - Apr 10 2018
- Continue if ERF type unknown. Fixes vDAGs failing if they have seen no packets yet.

## 0.1.3 - Apr 5 2018
- Fix caplen including record alignment padding. Bro seems to directly pass
  this through to protocol analyzers.

## 0.1.2 - Apr 3 2018
- Fix broctl plugin breaking lb_interfaces. Only operate when lb_method="custom".

## 0.1.1 - Mar 29 2018
- Add large stream support.
- Fix bytes_received always being reported as 0.
- Basic CPack Debian/RPM packaging by running make packages from build directory. RPM packaging is not recommended.
- Packages add the bro user to the dag group if there is one.

## 0.1 (unreleased)
- First cut version of bro-dag native Endace DAG plugin.
- Port old bro DAG IOSource to modern bro.