
Endace::DAG
=================================

This plugin provides native [Endace](https://www.endace.com/) DAG card and EndaceProbe Application Dock packet capture support for Zeek.

Installation
--------------------

### Prerequisites
1. Ensure you have a recent [Zeek](https://www.zeek.org/download/) release installed. Bro 2.6 to Zeek 3.2 have been tested. To install zeek-dag you will need to have the ``zeek-devel`` or``zeek-core-dev`` package installed, or the compiled source directory for Zeek available.

    If you are using Bro, replace commands, directories and groups prefixed with ``zeek`` with ``bro`` in the following instructions.

2. Install the latest [DAG software package](https://www.endace.com/support). Follow the DAG installation instructions to get its kernel module, drivers, userspace libraries and development headers installed.

3. If your installation of Zeek runs as a non-root user, add the ``zeek`` user to the ``dag`` group to allow access to DAG cards (DAG 5.7.1 or newer):
    ````
    usermod -a -G dag zeek

4. Install prerequisites for building zeek-dag:
   * Red Hat/CentOS/Fedora:
        ````
        yum install cmake make gcc-c++
        ````
   * Debian/Ubuntu:
        ````
        apt-get install build-essential cmake
        ````

5. If you are using Zeek 3.1+ with Red Hat/CentOS 7, you will need to use a newer version of the GCC compiler using the ``devtoolset-7`` package as well as the newer ``cmake3`` package:

   * Enable the [Software Collections](https://developers.redhat.com/products/developertoolset/hello-world) and [EPEL](https://fedoraproject.org/wiki/EPEL) repositories. For CentOS this can be done using the following commands:
        ````
        yum install epel-release
        yum install centos-release-scl
        ````

   * Install the ``devtoolset-7`` and ``cmake3`` packages:
        ````
        yum install devtoolset-7 cmake3
        ````

   * Enter the devtoolset environment before installing zeek-dag. Subsequent build commands will use GCC 7. *You will need to enter this environment even if you are installing zeek-dag using the zkg package manager.*
        ````
        scl enable devtoolset-7 bash
        ````

### Installation using zkg/bro-pkg package manager
Ensure you have [zkg](https://docs.zeek.org/projects/package-manager/en/stable/quickstart.html) installed and that ``zeek-config`` is in path, then install zeek-dag:

    zkg install endace/zeek-dag

### Manual Installation
After building Zeek from the sources, run:

    git clone https://github.com/endace/zeek-dag.git
    cd zeek-dag
    ./configure --zeek-dist=<path to zeek sources>
    make && sudo make install

Usage
-----
If everything built and installed correctly, you should see this:

    zeek -N Endace::DAG
    Endace::DAG - Packet acquisition via Endace DAG capture cards (dynamic, version 0.3)

Once installed, you can use DAG card streams by prefixing them
with ``endace::`` on the command line. For example, to capture from
DAG card 1:

    zeek -i endace::dag1

To capture from DAG card 1, stream 2:

    zeek -i endace::dag1:2

This plugin does not configure hardware load balancing on the DAG card. Use the DAG
software tools to configure the card before use. For example to
configure 2-tuple (src/dst IP) load balancing for 8 worker processes:

    dagconfig -d1 hash_tuple=2 hash_bins=8

To use zeek-dag in production with multiple Zeek processes, use a configuration 
similar to this in node.cfg (e.g. /usr/local/zeek/etc/node.cfg):

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

Now start the ZeekControl shell:

    zeekctl

And start the Zeek instances:

    [ZeekControl] > deploy

Or simply:

    zeekctl deploy

Packaging
---------
### Debian/RPM packages
Basic binary-only packages can be generated as follows if you have rpmbuild and/or Debian build tools installed.
Distributing these packages outside a closed environment is not recommended, as Zeek package installation locations vary.

    ./configure --zeek-dist=<path to zeek sources after building>
    cd build/
    make package

Packages add the zeek user to the dag group if it exists.

### Tarball
From a *completely clean* (i.e. no untracked files) git checkout:

    ./configure --zeek-dist=<path to zeek sources>
    cd build/
    make package_sources
