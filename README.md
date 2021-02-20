# Sxiaobaibai's shell (xs_shell)

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

- redirect and pipe
```
$: cat $(path to file) | grep $(search keyword) > $(output file name for keyword search in the file)
```
