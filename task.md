## Purpose

Functionality:
+ The client and the server should be two separate processes running in two separate Docker containers.
+ The client and the server should periodically (e.g. 0.01Hz) exchange some arbitrary data.
+ The client and the server should be both able to detect communication failures: repeated data, lost data, data out of order.
+ The project should compile either in a container of your choosing and on a standard base Debian system.
