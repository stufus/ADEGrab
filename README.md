# ADEGrab
Anyone who has used the Sysinternals AD Explorer tool will know that it is very useful, especially for rapid offline analysis of an AD structure which, for example, has been acquired during a penetration test in order to gain situational awareness.

One of the unfortunate limitations of AD Explorer is that you cannot copy/paste or select the results from the Search Container. ADEGrab circumvents this problem by reaidng the contents of the ListView control and copying to clipboard or saving to a file, which should allow testers to parse this information or use it as part of other tools.

# Internals
It basically works by sending windows messages to the ListView control in order to extract the data, column by column. However, processes cannot write to other process' memory, meaning that it was not as simple as simply sending LVM_GETITEM. The way that I chose to deal with this is to read and write directly from AD Explorer's memory.

# Disclaimer
This was a tool that I put together in an evening in order to solve a problem. It has not been thoroughly tested against every possible scenario, but is a handy tool if you are in this position. Feel free to enjoy its use.
