# Bencode parser

This is an [stb style](https://github.com/nothings/stb) Bencode parser built in C based on the [Bittorrent specification](https://wiki.theory.org/BitTorrentSpecification#Bencoding).

## NOTE: This project is a Work in progress

## How to use
- Copy stb_bencode.h to your project
- You can use it as a normal header wherever you want to.
- To actually include the implementation, you have to define the BENCODE_IMPLEMENTATION macro before the include:

```c
#include <blah.h>
#include <blahblah.h>

#define BENCODE_IMPLEMENTATION
#include "stb_bencode.h"
```

## Running tests
To run the tests, you have to install the [Unity testing framework](https://github.com/ThrowTheSwitch/Unity).

```sh
$ ./run-tests.sh
```
