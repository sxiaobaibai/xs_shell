# Sxiaobaibai's shell (xs_shell)
This is my simple shell build through a project. This shell is written in C language.

## abstract
function
- execute command whose binary file is located in $PATH
- shell build in command
	- cd
- pipe
- redirect

## how to build
```
make
./xs_shell
```

## how to use
- list directory
```
$: ls
```
- change directory
```
$: cd ..
$: cd ~
$: cd /
```
- execute cat command
```
$: cat $(path to file)
```

- ~~redirect and pipe~~(worked in older version)
- pipe
```
$: ls | cat | wc -l
```

- quit
```
$: exit
```
or just press Ctrl + D
