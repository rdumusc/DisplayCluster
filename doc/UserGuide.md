User Guide {#userguide}
============

This document is a short how-to to start running DisplayCluster.

## Running the application

Run 'startdisplaycluster' from the install/bin folder to start the application with an example configuration.

Use 'startdisplaycluster \-\-help' to see all the available options.

### OSX notes

The following steps might be required to run the application on OSX.

* Allow 'Remote Login' in 'System Preferences' -> Sharing
* Allow passwordless login on the same machine: cat ~/.ssh/id_rsa.pub >> ~/.ssh/authorized_keys
* Reference your hostname explicitly in /etc/hosts to point to 127.0.0.1.
  Example: hostname>  bluebrain077.epfl.ch -> Add to /etc/hosts: 127.0.0.1    bluebrain077.epfl.ch
