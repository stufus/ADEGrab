# ADEGrab
Anyone who has used the Sysinternals AD Explorer tool will know that it is very useful, especially for rapid offline analysis of an AD structure which, for example, has been acquired during a penetration test in order to gain situational awareness.

One of the unfortunate limitations of AD Explorer is that you cannot copy/paste or select the results from the Search Container. ADEGrab circumvents this problem by reading the contents of the ListView control and copying to clipboard or saving to a file, which should allow testers to parse this information or use it as part of other tools.

# Internals
Initially, I wrote a DLL to hook the windows messages to AD Explorer's ListView control; my plan was simply to intercept the LVM_INSERTITEM message and read the lParam->pszText item in the LVITEM struct. However, it seems that they chose to set pszText to LPSTR_TEXTCALLBACK and use notify messages to actually set the text. It would be more complicated to hook those messages because it would involve intercepting WM_NOTIFY messages, identifying LVN_GETDISPINFO requests and intercepting the responses. 

A simpler method is to send windows messages such as LVM_GETITEM to the ListView control in order to extract the data, column by column. However, processes cannot write to other process' memory, meaning that it was not as simple as simply sending LVM_GETITEM. The way that I chose to deal with this is to read and write directly from AD Explorer's memory.

When attempting to capture the listbox contents, ADEGrab:

1. Attempts to retrieve a handle to the Search Container SysListView32 control (i.e. the results list).
2. Retrieve a couple of handles (thread and process) of the instance of AD Explorer that owns the control above and gain read/write access to the process.
3. Sends the LVM_GETITEMCOUNT message to the ListView control to retrieve the number of items in the list.
4. Calculate the amount of buffer space needed to store everything. On the assumption that each 'cell' will be no more than 250 characters in length, calculate 2*(number+2). The +2 is for the \r\n linefeed characters, and the multiplication by 2 is because we are retrieving Unicode characters.
5. Working on the same assumption that we are going to retrieve a maximum of 250 characters on each cell, allocate a block of memory to store a LVITEM struct with a (250*2)+1 byte buffer below it. 250*2 because it is unicode, and the +1 for the null terminator. In order to keep this simple, I created a struct called ADMemInject that has two items; an LVITEM and a suitably sized TCHAR buffer.
 
 The reason for this is the way that interacting with ListView controls actually works. In order to retrieve an item from a ListView control, you need to send it an LVM_GETITEM message, with lParam pointing to an LVITEM struct that contains details on which item to receive. The key component here is the iItem variable, which is the zero-indexed item position to receive. When the ListView receives that message, it will go to the cell at the iItem'th position and, providing the flags such as mask are set correctly, it will copy the text into the buffer that pszText points to. 
 However, processes cannot usually access memory belonging to other processes, so you cannot simply create a buffer using malloc/calloc or similar, because that would be created inside ADEGrab's process. I therefore used VirtualAlloc to create a block of memory inside AD Explorer's process (that's why #2 was necessary), copied the ADMemInject struct into it and made sure that the pszText parameter pointed to the block of memory immediately after the struct. The idea is that the ListView control will read the struct and copy the item's text to the memory directly 'after' the LVITEM struct itself. 

6. Copy the local variable to AD Explorer's buffer and send the LVM_GETITEM message. This will cause AD Explorer's ListView to write the cell contents to its own buffer, which is allowed. 
7. Read that block of memory back into ADEGrab's process. This will always be safe because we allocated the memory block; it gave us a virtual memory address inside AD Explorer's process, so we can safely interact with it as much as we like until we release the handle or AD Explorer closes.
8. Append the text to the complete buffer, with the terminating \r\n.

Once each item has been retrieved from the ListView control, the handles are closed and the relevant memory freed. We now have a large Unicode buffer with a newline-separated list of every item that has been retrieved from the ListView control. ADEGrab now performs two optional activities.

1. If the Clipboard menu option is ticked, ADEGrab converts the buffer to ANSI and copies it to clipboard.
2. If the Log File menu option is ticked, ADEGrab opens "adegrab.log" and appends the ANSI buffer to it.

It then displays a balloon tip (in the system tray) to the user, informing them of the captured data.

# Usage
- Load AD Explorer and connect to the domain that you wish to browse.
- Load the 'Search Container' (i.e. Search for something).
- Execute ADEGrab. If you minimise it, it will go to the tray.
- Double-click the tray icon, right-click the tray icon and select 'Capture' or press the 'Capture' button on the GUI. ADEGrab will retrieve the first column of the Search results and display it on the main GUI. If 'Capture to Clipboard' is selected, ADEGrab will convert the results from Unicode to ANSI and save them to the clipboard.
- If 'Capture to Log File' is selected, ADEGrab will convert the results from Unicode to ANSI and append them to 'adegrab.log'.

Primarily, I use it by minimising ADEGrab to the tray, double-clicking it whenever I want to save the results and then pasting into whatever file I want. This works across VMs (tested on VirtualBox) as long as VirtualBox tools are installed and Clipboard is set to bidirectional.

Note that ADEGrab opens and closes all handles on each capture attempt. This means that it does not matter whether you load ADEGrab first, or AD Explorer first, or whether you close or open them.

# Downloads

AD Explorer v1.44 can be downloaded at https://download.sysinternals.com/files/AdExplorer.zip
Homepage: https://technet.microsoft.com/en-GB/sysinternals/bb963907.aspx
 
The ADEGrab binary can be downloaded at https://github.com/stufus/ADEGrab/releases/download/v1.00/ADEGrab.zip. Grab the source, check it and recompile it if you are worried about executing an untrusted binary.
SHA256 (ADEGrab.zip) = 6860b91288eb796a2721b66158cfa668f4f108cac0faa3e5e00a35012a05f3fb                
SHA512 (ADEGrab.zip) = 5636d84e608e9e90986b43d740fe043bea6223cce02aedd1927353f946e9600419e3d42f3de5bc8673618ec60038e7d

# Building

If you want to compile and run it yourself, just open ADEGrab.sln in Visual Studio 2013 (I used VS2013 Community Edition). You will need to ensure that it is compiled using Unicode rather than ANSI (it will compile but not function correctly in ANSI mode because AD Explorer is Unicode). For those old-timers out there, this is oldschool C (ish) code; no .NET in sight. I've nothing against C# by the way; I'm just not a C# coder and there is something I like about C and Win32 APIs.

There are a whole bunch of compiler warnings, mainly because I have not casted variables properly. This is because this started as a quick and dirty project.

# Further Development

Additional functionality that I will stabilise and release will include:
- Optionally grab all columns from the Search Container.
- Optionally choose the output file.
- Fix the compiler warnings.

# Disclaimer

This was a tool that I put together in an evening in order to solve a problem. It has not been thoroughly tested against every possible scenario, but is a handy tool if you are in this position. 

# Further Reading

https://labs.mwrinfosecurity.com/blog/memory-allocation-how-injecting-into-your-own-tools-might-help-you-compromise-a-windows-domain/
