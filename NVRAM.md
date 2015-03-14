# Basic connection information #
The NVRAM is used via the EBI of the board; we use chip select 3.  It uses an eight-bit data bus and a 21-bit address bus, so we use D0-D7 on the EBI for data and A0-A20 for the address.  NWR0, NRD, and NCS3 are used for WR, RD, and CS.

# Register values #

DBW = 10 (EBI\_DBW\_8)

NWS = 100 (EBI\_NWS\_5) - testing will show if we need to increase this; the W3100A gives max tCO and tOE of 73ns for direct mode and 81 ns for indirect mode); furthermore, the internal clocked mode of the Ethernet module is for access times >= 100ns

PAGES = 01 (EBI\_PAGES\_4M)

TDF = 001 (EBI\_TDF\_1)

BAT = 0 (EBI\_BAT\_BYTE\_WRITE) - I was never sure about this, but it should be right

BA = 0x200 (Base address is 0x20000000)

# Pin diagram #
TODO if we feel like it

# Testing #
Seems to be working correctly.  Had to increase NWS from 3 to 5.