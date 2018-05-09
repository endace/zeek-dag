# Release Notes
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