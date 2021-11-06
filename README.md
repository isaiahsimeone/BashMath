# BashMath
BashMath allows for the evaluation of mathematical expressions directly from the command line.

## Installation
Currently, the BashMath project comes included with all source necessary to build Bash. 

`git clone github.com/isaiahsimeone/BashMath.git`

`cd BashMath`

`./configure && make`
Then,
`sudo make install`

## Usage
Once installed, BashMath will 'hijack' commands sent into bash that start with either: a digit (0-9 inclusive), or an '=' character.
Shell scripts are not affected by this. Any shell script that worked in bash before, will still work.

## Features
* Supported operators include: +, -, /, *, %, |, ^, &, ~, <<, >> and **
* Summation of expressions over a range, e.g. =sum x over 1...3 in 3 * x
* Variable assignment and use in expressions
* Proper order of operations, and operation associativity
* Error detection of malformed brackets, unrecognised/unexpected symbols

## Todo
* Improve performance of summation statement (re-parses expression for each pass of the summation.)
* Allow for base conversions. Currently, all bases convert into decimal. Add functions like bin(), hex(), oct() to convert numeric bases.
* It would be nice to use a BigNumber type or something similar, so huge numbers are supported.

## Changelog

### 2.0 - 25/06/21
* Added variable assignment and use within expression
* Added the summation statement which can be used for example like: =sum x over 1...3 in 3*x
* Added a help display which can be shown by typing '=help' in Bash
* Refactored math_parser.c into seperate files: mp_scanner.c, mp_parser.c, mp_error.c and mp_main.c.

### 1.0 - 1/12/20
. Created BashMath

## Gallery


![alt text](/g1.png?raw=true)
