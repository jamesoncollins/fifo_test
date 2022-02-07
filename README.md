# fifo_test

Demonstrates an issue with SSH logins when running a program that uses many
SCHED_FIFO threads and gtk3 file choosers.

* Pass the option 'fifo' to start a SCHED_FIFO thread
* Pass 'gtk' to have gtk be initialized.
* Pass 'fc' to initialize gtk and a file chooser dialog
* Pass 'sock' to simulate the netlink socket call that fails inside SSH

Note that the application will not actually display a window

## Building

* simply call `make`
* After calling make you need to allow this application to use the real-time
 scheduler: `sudo setcap cap_sys_nice+ep ./fifo_make`


## Running

Run `./fifo_make fifo`, `./fifo_make fc`, and then `./fifo_make sock`

After about 30 seconds the sock thread will hang.

You could also test this by trying to login to SSH, which will also hang if 
you server doesn't specify an IPv4 type.

 