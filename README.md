
Endace::DAG
=================================

This plugin provides native [Endace](https://www.endace.com/) DAG card and
EndaceProbe Application Dock packet capture support for Zeek.

Installation
--------------------

### Prerequisites
1. Ensure you have a recent [Zeek](https://www.zeek.org/download/) release
   installed. Bro 2.6 to Zeek 4.0 have been tested. To install zeek-dag you will
   need to have the ``zeek-devel`` or``zeek-core-dev`` package installed, or the
   compiled source directory for Zeek available.

   If you are using Bro, replace commands, directories and groups prefixed with
   ``zeek`` with ``bro`` in the following instructions.

2. Install the latest [DAG software package](https://www.endace.com/support).
   Follow the DAG installation instructions to get its kernel module, drivers,
   userspace libraries and development headers installed.

3. If your installation of Zeek runs as a non-root user, add the ``zeek`` user
   to the ``dag`` group to allow access to DAG cards (DAG 5.7.1 or newer):
    ````
    usermod -a -G dag zeek
    ````

4. Install prerequisites for building zeek-dag:
   * Red Hat/CentOS/Fedora:
        ````
        yum install cmake make gcc-c++
        ````
   * Debian/Ubuntu:
        ````
        apt-get install build-essential cmake
        ````

5. If you are using Zeek 3.1 or later with Red Hat/CentOS 7, you will need a
   newer version of the GCC compiler from the ``devtoolset-7`` package as well
   as the newer ``cmake3`` package:

   * Enable the
     [Software Collections](https://developers.redhat.com/products/developertoolset/hello-world) and
     [EPEL](https://fedoraproject.org/wiki/EPEL) repositories. For CentOS
     this can be done using the following commands:
        ````
        yum install epel-release
        yum install centos-release-scl
        ````

   * Install the ``devtoolset-7-toolchain`` and ``cmake3`` packages:
        ````
        yum install devtoolset-7-toolchain cmake3
        ````

   * Enter the devtoolset environment before installing zeek-dag. Subsequent
     build commands will use GCC 7. *You will need to enter this environment
     even if you are installing zeek-dag using the zkg package manager.*
        ````
        scl enable devtoolset-7 bash
        ````

### Installation using zkg/bro-pkg package manager
Ensure you have
[zkg](https://docs.zeek.org/projects/package-manager/en/stable/quickstart.html)
installed and configured.

If you have previously installed zeek-dag under the name ``bro-dag`` you will
need to remove it first using ``zkg remove bro-dag``.

Install zeek-dag using zkg:

    zkg install endace/zeek-dag

To uninstall use:

    zkg remove zeek-dag

### Manual Installation
After building Zeek from the sources, run:

    git clone https://github.com/endace/zeek-dag.git
    cd zeek-dag
    ./configure --zeek-dist=<path to zeek sources>
    make && sudo make install

If you encounter build errors, run ``make distclean`` and ensure you have an
appropriate build environment before running configure and make again.

To uninstall use:

    make uninstall

### Test
Check the zeek-dag packet source can be loaded successfully. If everything built
and installed correctly, you should see this:

    $ zeek -N Endace::DAG
    Endace::DAG - Packet acquisition via Endace DAG capture cards (dynamic, version 0.4)

Once installed, you can use DAG card streams by prefixing them with ``endace::``
on the command line. Note that Zeek will write output files to the current
directory by default.

For example, to capture from
DAG card 0:

    zeek -i endace::dag0

To capture from DAG card 1, stream 2:

    zeek -i endace::dag1:2

Usage with ``zeekctl``
----------------------

> *Note:* This plugin does not configure hardware load balancing on the DAG
> card. Use the DAG software tools to configure the card before use.
>
> For example to configure 2-tuple (src/dst IP) load balancing for 8 worker
> processes on physical DAG card dag1:
>
> ````
> dagconfig -d1 hash_tuple=2 hash_bins=8
> ````
>
> For information on configuring hash load balancing to multiple vDAGs in
> EndaceProbe Application Dock, please refer to the
> [EndaceProbe User Guide](https://www.endace.com/support).

To use zeek-dag in production with multiple Zeek processes, use a worker
configuration similar to this in node.cfg (e.g. /opt/zeek/etc/node.cfg):

```` ini
[worker-1]
type=worker
host=localhost
lb_method=custom
lb_procs=8
dag_lb_method=stream
interface=endace::dag0
## Optionally pin worker threads
#pin_cpus=0,1,2,3,4,5,6,7
````

Where ``lb_procs`` is the number of processes for load balancing. DAG cards
support up to 32 streams/procs for load balancing in hardware, as well as
hardware packet filtering and flexible steering of up to 4 capture
ports/interfaces to streams
(see [DAG documentation](https://www.endace.com/support)).

The ``dag_lb_method`` node.cfg option can be used to specify the DAG load
balancing method. This is set to ``stream`` by default if not specified, and
needs to be set for each worker section. Load balancing stream numbering is
reset between worker sections unless they have identical interface configuration
and the same host.

   * ``stream`` (default): Load balance receive (even numbered) streams on a
     single DAG card. The load balancing will start at 0 or the specified stream
     number.
   * ``card``: Load balance multiple DAG cards, each with a single stream. This
     configuration is commonly used for vDAGs inside Application Dock. The load
     balancing will start at the specified DAG card number, capturing on stream
     0 or the specified stream on each DAG card.

### Load Balancing by Card

To use multiple DAG cards each with one stream, as commonly used for vDAGs
inside Application Dock, use ``dag_lb_method=card``. For example, to load
balance across 8 DAGs dag0-dag7 use:

```` ini
[worker-1]
type=worker
host=localhost
lb_method=custom
lb_procs=8
dag_lb_method=card
interface=endace::dag0
## Optionally pin worker threads
#pin_cpus=0,1,2,3,4,5,6,7
````

### Deploy Configuration
Now start the ZeekControl shell:

    zeekctl

And start the Zeek instances:

    [ZeekControl] > deploy

Or simply:

    zeekctl deploy

Packaging
---------
### Debian/RPM packages
Basic binary-only packages can be generated as follows if you have rpmbuild
and/or Debian build tools installed. Distributing these packages outside a
closed environment is not recommended, as Zeek package installation locations
vary.

    ./configure --zeek-dist=<path to zeek sources after building>
    cd build/
    make package

Packages add the zeek user to the dag group if it exists.

If Zeek packages are installed as ``zeek-lts`` add the configure option ``--zeek-package-name=zeek-lts``.

### Tarball
From a *completely clean* (i.e. no untracked files) git checkout:

    ./configure --zeek-dist=<path to zeek sources>
    cd build/
    make package_sources
