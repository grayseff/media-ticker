# Ticker

A small status ticker for dwm/slstatus.

## Description

Ticker reads newline-delimited events from standard input and writes a scrolling status message to an output file.

Two event types are supported:

- `M:` — Updates the persistent message, displayed whenever no notification is active.
- `N:` — Queues a temporary notification, displayed twice before returning to the persistent message.

Unrecognised input is ignored.

Messages exceeding the configured display width scroll automatically.

## Build

Requires a C99 compiler.

`make install` builds a stripped, statically linked executable and installs it to `~/.local/bin`.

## Usage

Ticker can be run directly from the command line, supplied with piped input, or fed through a named pipe (FIFO).

Input must consist of newline-delimited messages prefixed with `M:` or `N:`.

Ticker writes its output to a file, which can be read by a status bar or other program.

For use with dwm/slstatus, `playerctl` and `tiramisu` provide convenient sources of media information and desktop notifications. The `cat` function in slstatus can read the output file.

## Files

`/tmp/current-ticker` — Output file consumed by slstatus.
