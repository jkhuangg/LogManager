# C++ Log Manager
This C++ program reads in log files with the format: 
```
timestamp|category|log_message
```
Follow by CLI options to retrieve and modify the log file as wished
# Instructions
1. To compile this program, run ```make all```
2. This should generate the ```logman``` executable 

The logman program reads in log files as such:
```
>./logman log_file
```
The commands ```logman``` support are:
<ul>
<li><strong>Timestamps Search</strong></li>
t &lttimestamp1>|&lttimestamp2>&l
<li><strong>Matching Timestamp</strong></li>
m &lttimestamp>
<li><strong>Category Search</strong></li>
c &lstring>
<li><strong>Keyword Search</strong></li>
k &lstring>
<li><strong>Append Log Entry (by entryID)</strong></li>
a &linteger>
<li><strong>Append Search Results</strong></li>
r
<li><strong>Delete Log Entry</strong></li>
d &linteger>
<li><strong>Move to Beginning</strong></li>
b &linteger>
<li><strong>Move to End</strong></li>
e &linteger>
<li><strong>Sort Excerpt List</strong></li>
s
<li><strong>Clear Excerpt List</strong></li>
l
<li><strong>Print Most Recent Search Results</strong></li>
g
<li><strong>Print Excerpt List</strong></li>
p
<li><strong>Quit</strong></li>
q
<li><strong>No Operation (comment)</strong></li>
&#35
</ul>

You can also batch commands together into a file. For example:
```
>./logman spec-log.txt < spec-cmds.txt
```

Finally, run ```make clean``` to clean up the directory</br>

Thanks for stopping by!
