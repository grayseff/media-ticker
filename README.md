# Ticker

A small status ticker for DWM/slstatus

Reads newline delimited events from stdin:
	- `M:` updates the persistent media message
	- `N:` displays a notification twice, then returns to media
	- Other output ignored

Messages longer than scroll width scroll automatically. 

## Build

Requires a C99 compiler

make install copies a stripped, statically compiled version to .local/bin

## Usage
works best with playerctl and tiramisu, use the cat command in slstatus for best results
