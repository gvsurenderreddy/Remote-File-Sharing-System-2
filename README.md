Remote-File-Sharing-System
==========================

Functionality of your program

When launched, your process should work like a UNIX shell. It should accept incoming connections
and at the same time provide a user interface that will offer the following command options: (Note that
specific examples are used for clarity.)


1. HELP Display information about the available user interface options.


2. MYIP Display the IP address of this process.
   Note: The IP should not be your "lo" address (127.0.0.1). It should be the actual IP of the server.


3. MYPORT Display the port on which this process is listening for incoming connections.


4. REGISTER <server IP> <port_no>: This command is used by the client to register itself with the
server and to get the IP and listening port numbers of all the peers currently registered with the server.
    The first task of every client is to register itself with the server by sending the server a TCP
message containing its listening port number. The server should maintain a list of the IP address and
the listening ports of all the registered clients. Let's call this list as "Server-IP-List". Whenever a new
client registers or a registered client exits, the server should update its Server-IP-List appropriately and
then send this updated list to all the registered clients. Client should always listen to such updates from
the server and update their own local copy of the available peers. Any such update which is received
by the client should be displayed by the client. The REGISTER command takes 2 arguments. The
first argument is the IP address of the server and the second argument is the listening port of the

server.


Note: The REGISTER command works only on the client and should not work on the server.


5. CONNECT <destination> <port no>: This command establishes a new TCP connection to the
specified <destination> at the specified < port no>. The <destination> can either be an IP address or a
hostname. (e.g., CONNECT euston.cse.buffalo.edu 3456 or CONNECT 192.168.45.55 3456). The
specified IP address should be a valid IP address and listed in the Server-IP-List sent to the client by
the server. Any attempt to connect to an invalid IP/hostname or an IP/hostname not listed by the server
in its Server-IP-List should be rejected and suitable error message should be displayed. Success or
failure in connections between two peers should be indicated by both the peers using suitable
messages. Self-connections and duplicate connections should be flagged with suitable error messages.
Every client can maintain up-to 3 connections with its peers. Any request for more than 3 connections
should be rejected. However, every client should always maintain a constant connection with the
server.


6. LIST Display a numbered list of all the connections this process is part of. This numbered list will
include connections initiated by this process and connections initiated by other processes. The output
should display the hostname, IP address and the listening port of all the peers the process is connected
to. Also, this should include the server details.
E.g.,  id:     Hostname                                   IP address                    Port No.
        1: timberalake.cse.buffalo.edu                192.168.21.20                      4545
        2: embankment.cse.buffalo.edu                 192.168.21.21                      5454
        3: highgate.cse.buffalo.edu                    192.168.21.22                     5000
        4: euston.cse.buffalo.edu                     192.168.21.22                     5000



Note: The connection id 1 should always be your server running on timberlake.cse.buffalo.edu. The
remaining connections should be the peers who you are connected to.


7. TERMINATE <connection id.> This command will terminate the connection listed under the
specified number when LIST is used to display all connections. E.g., TERMINATE 2. In this example,
the connection with embankment should end. An error message is displayed if a valid connection does
not exist as number 2. If a remote machine terminates one of your connections, you should also
display a message.


8. EXIT Close all connections and terminate this process. When a client exits, the server de-registers
the client and sends the updated "Server-IP-List" to all the clients. Clients on receiving the updated list
from the server should display the updated list.

9. UPLOAD <connection id.> <file name> For example, UPLOAD 3 /local/a.txt. This will upload
the file a.txt which is located in /local/ to the host on the connection that is designated by number 3
when LIST is used. An error message is displayed if the file was inaccessible or if 3 does not represent
a valid connection. The remote machine will automatically accept the file and save it under the
original name in the same directory where your program is. When the upload is complete this
process will display a message indicating so. Also, the remote machine will display a message in its
user interface indicating that a file (called a.txt) has been downloaded. When an upload is occurring,
the user interface of the uploading process will remain unavailable until the upload is complete. Upon
completion, a message is displayed. If the upload fails for some reason, an error message should be
displayed. When an upload is occurring, a message should be displayed on the remote machine when
the upload begins. If the upload fails for some reason, an error message should be displayed on the
remote machine.
At the end of each successful upload, you will print the rate at which the Transmitter
(Sender/Uploader) uploaded the file. Similarly, you will also print at the receiver, the rate at which the
file was received. We will call these as Tx Rate and Rx Rate. Tx rate is the amount of data (in Bits)
transferred from the Tx end, divided by the time taken by Tx to read and send the complete file in
chunks of Packet Size bytes each. The Rx rate is defined similarly (in Bits/sec) as the total file size
received (in bits) divided by the time taken to receive the file over the socket and write it. For example,
if a file was uploaded from embankment to euston, the format for printing this information is:
At Tx end:
Tx (embankment ): embankment -> euston, File Size: x Bytes, Time Taken: y seconds, Tx Rate: z
bits/second.
At Rx end:
Rx (euston): embankment -> euston, File Size: a Bytes, Time Taken: b seconds, Rx Rate: c
bits/second.


NOTE: You should read the file in chunks of packet size-byte buffers and send those buffers using the
send socket call, instead of reading the whole file at once. You can use UNIX utility function
gettimeofday to know the time taken for receiving and sending the file at the two ends. Make sure to
appropriately call this function in your program to account only for the time taken for uploading
(reading and sending) and downloading (receiving and writing) the file. After printing this information,
flush the standard output using fflush, to immediately print this information.


NOTE: UPLOAD command on a server should display an error message. No files should be uploaded
to the server. Any such attempt should display suitable error messages.


10. DOWNLOAD <connection id 1> <file1> <connection id 2> <file2> <connection id 3> <file3>
This will download a file from each host specified in the command. Please note that the total number
of hosts may be 1, 2 or 3. The hosts will be part of the list of hosts displayed by the LIST command.
E.g., if a command DOWNLOAD 2 file1 3 file2 4 file3 is entered to a process running on the server
UNDERGROUND, then the process running on UNDERGROUND receives this command, it will

request file1 from embankment, file2 from highgate and file3 euston. The local machine will automatically accept the file and save it in the same directory where your program is under the original name. When the download is complete
this process will display a message indicating so. Also, the remote machine will display a message in
its user interface indicating that a file (e.g. a.txt, b.txt or c.txt) has been downloaded along with the
hostname of the host from which the file was downloaded. Upon completion, a message is displayed.
If the download fails for some reason, an error message should be displayed. When a download is
occurring, a message should be displayed on the local machine. If the download fails for some reason,
an error message should be displayed on the remote machine.


Similarly as what UPLOAD requires, at the end of each successful download, you will print Rx rate at
which the Receiver downloaded those files and the Tx rates at which each Sender sent those files.


NOTE: Both notifications and data sending should use TCP connections. 


NOTE: DOWNLOAD command on a server should display an error message. No files should be
downloaded from the server.



11. CREATOR Display your (the students) full name, your UBIT name and UB email address.
