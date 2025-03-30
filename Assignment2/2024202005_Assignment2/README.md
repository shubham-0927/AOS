# AOS Assignemnt 2

POSIX shell implementation.

## To run program

- Download zip

- Extract zip file.
- Open shell in that directory.
- Then run:

```shell
make

```

- This will compile all cpp files and make object files.

- Then run

```shell
./output
```

- then you will be presented with shell interface like

```shell
<Name@UBUNTU:~>
```

- write and run command

```shell
<Name@UBUNTU:~> ls -a -l ; cd test
<Name@UBUNTU:~/test>
```

## About Assignemnt

- This Assignemnt includes implementation of Bash shell commands.

- commands implemented are:

     - cd

     - echo

     - pwd

     - ls (with tags -a and -l)

     - pinfo

     - search

     - ctrl+(c,z,d) signals

     - IO redirection

     - pipe

     - TAB autocomplete

     - history

     - Up arrow key

     - Down arrow key


- Here in history command shell will store history upto 20 commands, and you can retrieve 0 to 20.
