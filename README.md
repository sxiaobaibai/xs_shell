# Sxiaobaibai's shell (xs_shell)
This is my simple shell build through a project. This shell is written in C language.

## abstract
function
- execute command whose binary file is located in $PATH
- shell build in command
	- cd
	- exit
- pipe
- redirect
## how to build
```
make
```
## Example of how to use
- start the shell
```
./xs_shell
```
- list directory
```
$ ls
```
- change directory
```
$ cd ..
$ cd ~
$ cd /
```
- execute cat command
```
$: cat $(path to file)
```
- redirect
```
$ cat < $(some file) | grep
$ ls > $(file to store the output of ls)
$ ls >> $(file to store(append) the output of ls)
```
* sorry, "()" is not working
- pipe
```
$ ls | cat | wc -l
```
- quit
```
$ exit
```
~~or just press Ctrl + D~~
