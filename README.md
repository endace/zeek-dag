
Endace::DAG
=================================

This plugin provides native [Endace DAG](https://www.endace.com) support for Bro.

Bro-pkg Installation
--------------------

Make sure you have the [DAG software package](https://www.endace.com/support) installed and then run:

    bro-pkg install endace/bro-dag

Manual Installation
-------------------

Follow the DAG installation instructions to get its kernel module, drivers and userspace libraries
installed, then use the following commands to configure and build the plugin.

After building bro from the sources, change to the "bro-dag" directory and run:

    ./configure --bro-dist=<path to bro sources>
    make && sudo make install

If everything built and installed correctly, you should see this:

    bro -N Endace::DAG
    Endace::DAG - Packet acquisition via Endace DAG capture cards (dynamic, version 0.1)

Optionally, add the bro user to the dag group (DAG 5.7.1 or newer):

    usermod -a -G dag bro

Usage
-----

Once installed, you can use DAG card streams by prefixing them
with ``endace::`` on the command line. For example, to capture from
DAG card 1:

    bro -i endace::dag1

To capture from DAG card 1, stream 2:

    bro -i endace::dag1:2

This plugin does not configure hardware load balancing on the DAG card. Use the DAG
software tools to configure the card before use. For example to
configure 2-tuple (src/dst IP) load balancing for 8 processes:

    dagconfig -d1 hash_tuple=2 hash_bins=8

To use it in production with multiple Bro processes, use a configuration 
similar to this in node.cfg (e.g. /usr/local/bro/etc/node.cfg):

    [worker-1]
    type=worker
    host=localhost
    interface=endace::dag1
    lb_method=custom
    lb_procs=8
    pin_cpus=0,1,2,3,4,5,6,7

Where lb_procs is the number of processes for load balancing.

To use with multiple DAG cards (each using a single stream), use a configuration 
similar to this in node.cfg (e.g. /usr/local/bro/etc/node.cfg):

    [worker-1]
    type=worker
    host=localhost
    lb_method=interfaces
    lb_interfaces=endace::dag0:0,endace::dag1:0
    lb_procs=2
    pin_cpus=0,1

Now start the BroControl shell like:

    broctl

Then start the Bro instances:

    [BroControl] > deploy

Or simply:

    broctl deploy

Packaging
---------
### Debian/RPM packages
Basic binary-only packages can be generated as follows if you have rpmbuild and/or Debian build tools installed.
Distributing these packages outside a closed environment is not recommended, as bro package installation locations vary.

    ./configure --bro-dist=<path to bro sources after building>
    cd build/
    make packages

Packages add the bro user to the dag group if it exists.

### Tarball
From a *completely clean* (i.e. no untracked files) git checkout:

    ./configure --bro-dist=<path to bro sources>
    cd build/
    make package_sources