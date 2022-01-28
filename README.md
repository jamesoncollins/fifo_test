# fifo_test




Demonstrates an issue with SSH logins when running a program that uses many
SCHED_FIFO threads and gtk3 file choosers.

This program will create NUM_CPU-2 threads, locked to cores, that will
utilize 100% of those cores.

* Pass the option 'fifo' to use SCHED_FIFO for each thread.
* Pass 'gtk' to have gtk be intialized.
* Pass 'fc' to initialize gtk and a file chooser dialog

Note that the application will not actually display a window

## Building

* simply call `make`
* After calling make you need to allow this application to use the real-time
 scheduler: `sudo setcap cap_sys_nice+ep ./fifo_make`


## Running
Run this application as `./fifo_make fifo` and attempted to login via ssh.
It doesn't matter whether the login is succesful (you can cancel out of the
password prompt).  Do this ~100 times.

Run the application again as `./fifo_make fifo fc` and try to login again
via ssh.  You should see that the login hangs after between 30 and 100
attempts.
 