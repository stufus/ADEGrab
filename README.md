# ADEGrab
Anyone who has used the Sysinternals AD Explorer tool will know that it is very useful, especially for rapid offline analysis of an AD structure which, for example, has been acquired during a penetration test in order to gain situational awareness.

One of the unfortunate limitations of AD Explorer is that you cannot copy/paste or select the results from the Search Container. ADEGrab circumvents this problem by reaidng the contents of the ListView control and copying to clipboard or saving to a file, which should allow testers to parse this information or use it as part of other tools.

# Internals
Initially, I wrote a DLL to hook the windows messages to AD Explorer's ListView control; my plan was simply to intercept the LVM_INSERTITEM message and read the lParam->pszText item in the LVITEM struct. However, it seems that they chose to set pszText to LPSTR_TEXTCALLBACK and use notify messages to actually set the text. It would be more complicated to hook those messages because it would involve intercepting WM_NOTIFY messages, identifying LVN_GETDISPINFO requests and intercepting the responses. 

A simpler method is to send windows messages such as LVM_GETITEM to the ListView control in order to extract the data, column by column. However, processes cannot write to other process' memory, meaning that it was not as simple as simply sending LVM_GETITEM. The way that I chose to deal with this is to read and write directly from AD Explorer's memory.

# Usage
- Load AD Explorer and connect to the domain that you wish to browse.
- Load the 'Search Container' (i.e. Search for something).
- Execute ADEGrab. If you minimise it, it will go to the tray.
- Double-click the tray icon, right-click the tray icon and select 'Capture' or press the 'Capture' button on the GUI. ADEGrab will retrieve the first column of the Search results and display it on the main GUI. If 'Capture to Clipboard' is selected, ADEGrab will convert the results from Unicode to ANSI and save them to the clipboard.
- If 'Capture to Log File' is selected, ADEGrab will convert the results from Unicode to ANSI and append them to 'adegrab.log'.

Primarily, I use it by minimising ADEGrab to the tray, double-clicking it whenever I want to save the results and then pasting into whatever file I want. This works across VMs (tested on VirtualBox) as long as VirtualBox tools are installed and Clipboard is set to bidirectional.

Note that ADEGrab opens and closes all handles on each capture attempt. This means that it does not matter whether you load ADEGrab first, or AD Explorer first, or whether you close or open them.

# Further Development

Additional functionality that I will stabilise and release will include:
- Optionally grab all columns from the Search Container.
- Optionally choose the output file.

# Downloads

AD Explorer v1.44 can be downloaded at https://download.sysinternals.com/files/AdExplorer.zip (Homepage: https://technet.microsoft.com/en-GB/sysinternals/bb963907.aspx)

# Disclaimer
This was a tool that I put together in an evening in order to solve a problem. It has not been thoroughly tested against every possible scenario, but is a handy tool if you are in this position. 
