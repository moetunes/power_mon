##POWER_MON
### A Simple Battery Warning System


###Summary
-------

**power_mon** is a self contained application that checks the battery status
	and if charged or low will pop up a small X window. The only dependency 
	is the X11 lib.


###Usage
-----

**power_mon** doesn't continually poll the battery status. It runs once to check
	the status, if the battery is below 37% or charged it pops up a small window,
	then exits.
	I have cron run *power_mon* every five minutes which seems to work fine.

	$ crontab -e
	*/5 * * * * export DISPLAY=:0 /path/to/power_mon

###Installation
------------

Need Xlib, then:

    Edit the power_mon.c file to point to the relevant file in
    /sys/class/power_supply.

    #define SYS_FILE "/sys/class/power_supply/BAT0/uevent"

    $ make
    # make install
    $ make clean


###Bugs
----

[ * No bugs for the moment ;) (I mean, no importants bugs ;)]


###Todo
----

  * I'll think of something

