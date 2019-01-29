# Sonic-Sync
Sonic-Sync

Oneliner to build and run via VICE:

make; mv Sonic-Sync.c64 Sonic-Sync.prg; c1541 -format "sonicsync,01" d64 sonicsync.d64; c1541 sonicsync.d64 -write Sonic-Sync.prg -write c64-pot.mou; x64 sonicsync.d64

