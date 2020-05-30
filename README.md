# DFA Simulator

### Description
This is a C++17 implementation of a DFA. It can read two types of files: `.dfa` and `.json`. If the given file is an
NFA, it will be converted to a DFA on construction.

### Install
Please note that this library is only designed to be built on Linux, and has only been tested with GCC 10.1. 
#### Arch Linux Package
Arch Linux users may install with the included `PKGBUILD`:
```bash
mkdir build && cd build
wget https://raw.githubusercontent.com/aokellermann/dfa/master/PKGBUILD
makepkg -si
```

#### Manual Build

##### Dependencies
Building from source requires the installation of all necessary dependencies:
* [cmake](https://github.com/Kitware/CMake) (build only)
* [json](https://github.com/nlohmann/json)
* [doxygen](https://github.com/doxygen/doxygen) (optional documentation)

##### Clone and Build
After dependencies are installed, run from a shell:
```bash
git clone https://github.com/aokellermann/dfa.git
mkdir dfa/build && cd dfa/build
cmake -DCMAKE_INSTALL_PREFIX:PATH=/usr ..
sudo make install
```
Substitute `/usr` with your desired install location.

### Usage
This package provides both a library and executable.

To use the library, simply include `dfa/dfa.h`. CMake users can link against `dfa::dfa`. 

Run `dfash -h` to see the list of options that can be specified.
You can pass in either a DFA or JSON file.

Then, the program will read from `stdin` any language you input. This library recognizes the special keyword
`epsilon` for the empty string.

```
$ cat m1.in
00000
11111
00100
001001
001000
0010001
epsilon
```

```bash
$ cat m1.in | dfash -d m1.dfa
00000 -> NOT ACCEPT
11111 -> ACCEPT
00100 -> ACCEPT
001001 -> ACCEPT
001000 -> NOT ACCEPT
0010001 -> ACCEPT
epsilon -> NOT ACCEPT
```

##### DFA Format
The input DFA file should adhere to this specification:
```
$ cat m1.dfa
states: q1 q2 q3
alphabet: 0 1
startstate: q1
finalstate: q2
transition: q1 0 q1
transition: q1 1 q2
transition: q2 0 q3
transition: q2 1 q2
transition: q3 0 q2
transition: q3 1 q2 
```

It must use the `.dfa` extension.

##### JSON Format
The input JSON file should adhere to this specification:

```bash
$ cat m1.json
```

```json
{
  "states": [
    "q1",
    "q2",
    "q3"
  ],
  "alphabet": [
    "0",
    "1"
  ],
  "start_state": "q1",
  "final_states": [
    "q2"
  ],
  "transitions": [
    {
      "s1": "q1",
      "symbol": "0",
      "s2": "q1"
    },
    {
      "s1": "q1",
      "symbol": "1",
      "s2": "q2"
    },
    {
      "s1": "q2",
      "symbol": "0",
      "s2": "q3"
    },
    {
      "s1": "q2",
      "symbol": "1",
      "s2": "q2"
    },
    {
      "s1": "q3",
      "symbol": "0",
      "s2": "q2"
    },
    {
      "s1": "q3",
      "symbol": "1",
      "s2": "q2"
    }
  ]
}
```

It must use the `.json` extension.