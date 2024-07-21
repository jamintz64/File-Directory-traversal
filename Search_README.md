# Search Utility
==============

This is a simple search utility written in C. It allows you to search for files in a directory and its subdirectories based on various criteria.

## Usage
-----
./search [-s size] [-f search_str [max_depth]] [-S] [-e exec_cmd] [-E exec_once_cmd]


## Options
-------

* `-s size`: Specify the maximum file size to consider.
* `-f search_str [max_depth]`: Search for files containing `search_str`. If `max_depth` is specified, it limits the search to that depth.
* `-S`: Show detailed information about the files found.
* `-e exec_cmd`: Execute `exec_cmd` for each file found.
* `-E exec_once_cmd`: Execute `exec_once_cmd` once for all files found.

## Example
-------

To search for files containing "example" in the current directory and its subdirectories, and execute the `ls -l` command for each file found: ./search -f example -e "ls -l"


To search for files larger than 100 bytes in the current directory and its subdirectories, and show detailed information about the files found: ./search -s 100 -S


To search for files containing "example" in the current directory and its subdirectories, and execute the `tar -czf archive.tar.gz` command once for all files found: 
./search -f example -E "tar -czf archive.tar.gz"


## Note
----

This utility uses the `fork` and `exec` system calls to execute commands for each file found. This means that it can be slow for large numbers of files.

Also, this utility does not handle errors well. It is recommended to use it in a controlled environment.

# Citations:
https://www.scaler.com/topics/c-fork/
https://www.geeksforgeeks.org/fork-system-call/
https://www.geeksforgeeks.org/exec-family-of-functions-in-c/
https://www.geeksforgeeks.org/c-switch-statement/
https://www.geeksforgeeks.org/buffer-in-c-programming/
Source code from HW02