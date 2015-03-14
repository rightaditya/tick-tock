# Basic connection information #
The Ethernet module is used via the EBI of the board; we use chip select 2.  It uses a 16-bit data bus and a 10-bit address bus, so we use D0-D15 on the EBI for data and A1-A10 for the address.  NWR0, NRD, and NCS2 are used for WR, RD, and CS.

# Register values #

DBW = 10 (EBI\_DBW\_8)

NWS = 101 (EBI\_NWS\_6)

PAGES = 00 (EBI\_PAGES\_1M)

TDF = 001 (EBI\_TDF\_1)

BAT = 0 (EBI\_BAT\_BYTE\_WRITE) - I was never sure about this, but it should be right

BA = 0x100 (Base address is 0x10000000)


# Pin diagram #
![http://tick-tock.googlecode.com/files/cmpe490_ethernet.png](http://tick-tock.googlecode.com/files/cmpe490_ethernet.png)

TODO: add diode resistor values, switch diode direction and point them to 5V instead of 3.3V, and add inverter to the reset switch that goes to ground (make sure inverter uses Vcc as input, since the reset pin won't drive anything but instead it's Vcc doing the driving)