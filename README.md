# crtfile -- A Linux utility program for creating file
`crtfile` is command-line utility program for creating (or truncating) file(s) with specified permission flags.

## How to use
To create a file, `example.txt`, with read, write permission for the owner, group, and others.  
(umask affects the permission)
```Bash
$ crtfile -u rw -g rw -o rw hello.txt
```
To override the umask : Add `-A (--absolute)` option during file creation.
```Bash
$ crtfile -A -u rw -g rw -o rw hello.txt
```

To truncate a file, `exampletrunc.txt`
```Bash
$ crtfile -t exampletrunc.txt
```
### Refer to the help of `crtfile` for more options
```Bash
$ crtfile --help
```
## Build the project
```Bash
$ make
```
##### Clear the object files and executable
```Bash
$ make clean
```
You have to set the `$PATH` variable of the `shell` if you want to use it from anywhere.

## License
The project `crtfile` is licensed user GNU General Public License v3.0 or later.  
Refer to [COPYING](./COPYING) for more information.
