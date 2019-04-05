
Endace::DAG
=================================

This plugin provides native [Endace DAG](https://www.endace.com/dag) packet capture card support for Bro.

Installation
--------------------

Ensure you have the latest [Bro](https://www.zeek.org/download/) release installed.

Install the latest [DAG software package](https://www.endace.com/support). Follow the DAG installation instructions to get its kernel module, drivers, userspace libraries and development headers installed, then use the following commands to configure and build the plugin.

Install pre-requisites for building bro-dag:

* Red Hat/CentOS/Fedora:
    ````
    yum install cmake make gcc-c++
    ````
* Debian/Ubuntu:
    ````
    apt-get install build-essential cmake
    ````
If your installation of bro runs as a non-root user, add the bro user to the dag group to allow access to DAG cards (DAG 5.7.1 or newer):

    usermod -a -G dag bro

### Installation using bro-pkg
Ensure you have [bro-pkg]((https://bro-package-manager.readthedocs.io/en/stable/quickstart.html)) installed and that bro-config is in path, then install bro-dag:

    bro-pkg install endace/bro-dag

### Manual Installation
After building bro from the sources, run:

    git clone https://github.com/endace/bro-dag.git
    cd bro-dag
    ./configure --bro-dist=<path to bro sources>
    make && sudo make install

Usage
-----
If everything built and installed correctly, you should see this:

    bro -N Endace::DAG
    Endace::DAG - Packet acquisition via Endace DAG capture cards (dynamic, version 0.3)

Once installed, you can use DAG card streams by prefixing them
with ``endace::`` on the command line. For example, to capture from
DAG card 1:

    bro -i endace::dag1

To capture from DAG card 1, stream 2:

    bro -i endace::dag1:2

This plugin does not configure hardware load balancing on the DAG card. Use the DAG
software tools to configure the card before use. For example to
configure 2-tuple (src/dst IP) load balancing for 8 worker processes:

    dagconfig -d1 hash_tuple=2 hash_bins=8

To use bro-dag in production with multiple Bro processes, use a configuration 
similar to this in node.cfg (e.g. /usr/local/bro/etc/node.cfg):

    [worker-1]
    type=worker
    host=localhost
    interface=endace::dag1
    lb_method=custom
    lb_procs=8
    ## Optionally pin worker threads
    #pin_cpus=0,1,2,3,4,5,6,7

Where lb_procs is the number of processes for load balancing. Current DAG card models support up to 32 streams/procs for load balancing in hardware, as well as hardware packet filtering and flexible steering of up to 4 capture ports/interfaces to streams (see [DAG documentation](https://www.endace.com/support)).

To use with multiple DAG cards, add multiple worker stanzas as above.

Now start the BroControl shell:

    broctl

And start the Bro instances:

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
    make package

Packages add the bro user to the dag group if it exists.

### Tarball
From a *completely clean* (i.e. no untracked files) git checkout:

    ./configure --bro-dist=<path to bro sources>
    cd build/
    make package_sources
