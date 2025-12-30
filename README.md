# Pure-Data externals
The following externals include C source, help file, Linux executable and Windows 32-bit DLL (only works with 32-bit pure data)

* [pd-demo](#pd-demo)
* [pd-purdie](#pd-purdie)
* [pd-memorex](#pd-memorex)
* [pd-seq](#pd-seq)
* [pd-shuffle](#pd-shuffle)
* [pd-lotus](#pd-lotus)
* [pd-comma](#pd-comma)
* [pd-seld](#pd-seld)
* [pd-jabea](#pd-jabea)
* [pd-jabea3](#pd-jabea3)
* [Installation](#Installation)

## pd-demo
Demonstration of Pure Data (Pd) objects connected together for experimental musical composition using random notes.

This project began with some bug fixes to the well known Pd [pd-shuffle](#pd-shuffle) object, which is useful for incorporating randomness into musical composition. A request for an enhancement led to the developement of three new objects to facilitate such composition:

* [pd-seq](#pd-seq) -- outputs a consequtive sequence of integers.
* [pd-purdie](#pd-purdie) -- outputs a series of integers (optionally obtained from [seq ]) in random order.
* [pd-memorex](#pd-memorex) -- records and plays back the output of [purdie ].

This repository contains a demo patch (pd-demo.pd) showing how to connect together these three objects to experiment with random note sequences. A short video of the demo patch in operation (pd-demo.mkv) is also included .

The heart of the demo patch is [purdie ], which outputs a random series of integers without repeating any of them, then repeats the series after shuffling the integers. The integers in the series can be preloaded as a consequtive sequence from [seq ] then augmented by inputting arbitrary individual integers, which can have the effect of removing those integers from the series if they were previously added. Like [shuffle ], [purdie ] has an optional setting to avoid repeating the end of one series at the beginning of the next.

If [memorex ] is attached to the output of [purdie ] before sending the notes to a MIDI synthesizer, one can listen to the random series until an interesting note combination is heard. By quickly toggling the **replay** input, the combination can be captured.

## pd-purdie
Related to [pd-shuffle](#pd-shuffle) for Pure Data.

![purdie-help](https://raw.githubusercontent.com/metamystical/pure-data/main/pd-purdie/purdie-help.jpg)

## pd-memorex
[memorex ] object for Pure Data -- play back recent integer inputs.

![purdie-help](https://raw.githubusercontent.com/metamystical/pure-data/main/pd-memorex/memorex-help.jpg)

## pd-seq
Accessory for [pd-purdie](#pd-purdie) for Pure Data.

![seq-help](https://raw.githubusercontent.com/metamystical/pure-data/main/pd-seq/seq-help.jpg)

## pd-shuffle
Improved [shuffle ] object for [Pure Data](https://github.com/pure-data/pure-data) - a free real-time computer music system. 

This is a drop-in replacement for the object of the same name in the [Motex library](https://puredata.info/downloads/motex) by Iain Mott, which can crash and uses an inefficient shuffle algorithm. It is rewritten from scratch using proper memory allocation and the Fisherâ€“Yates shuffle algorithm.

![shuffle-help](https://raw.githubusercontent.com/metamystical/pure-data/main/pd-shuffle/shuffle-help.jpg)

## pd-lotus
[lotus ] object for Pure Data - simple message database

![lotus-help](https://raw.githubusercontent.com/metamystical/pure-data/main/pd-lotus/lotus-help.jpg)

## pd-comma
[comma ] object for [Pure Data](https://github.com/pure-data/pure-data) - a free real-time computer music system. 

A utility object that adds or removes commas from a message.

![comma-help](https://raw.githubusercontent.com/metamystical/pure-data/main/pd-comma/comma-help.jpg)

## pd-seld
[seld ] object for [Pure Data](https://github.com/pure-data/pure-data) - a free real-time computer music system. 

An updateable version of [select ] that outputs a bang if the input matches an element of the argument list.

![seld-help](https://raw.githubusercontent.com/metamystical/pure-data/main/pd-seld/seld-help.jpg)

## pd-jabea
[jabea ] object for [Pure Data](https://github.com/pure-data/pure-data) - a free real-time computer music system. 

A utility object that outputs a series of integers in random order without repeating any, then reshuffles.

![jabea-help](https://raw.githubusercontent.com/metamystical/pure-data/main/pd-jabea/jabea-help.jpg)

## pd-jabea3
[jabea3 ] object for [Pure Data](https://github.com/pure-data/pure-data) - a free real-time computer music system. 

A utility object that outputs a series of integers taken from an editable internal buffer, in random order.

![jabea3-help](https://raw.githubusercontent.com/metamystical/pure-data/main/pd-jabea/jabea3-help.jpg)

## Installation

### Linux
On a Linux system, with "x" representing one of the above external names, simply place the linked file *x.pd_linux* along with the help patch *x-help.pd* into the same directory as your patch, or somewhere on the Pd search path (e.g., in an *externals* folder as set in Pd preferences). If you wish to build your own executable: 
* Download [pure-data/pd-lib-builder](https://github.com/pure-data/pd-lib-builder) and put the file Makefile.pdlibbuilder into the "x" directory.
* Create *Makefile* in the "x" directory. It should contain at least these two lines:
  - class.sources = x.c
  - include Makefile.pdlibbuilder
* Run *make* to build the executable.

### Windows
On a Windows system, with "x" representing one of the above external names, simply place the linked file *x.dll* along with the help patch *x-help.pd* into the same directory as your patch, or somewhere on the Pd search path (e.g., in an *externals* folder as set in Pd preferences). The *x.dll* file is a 32-bit DLL and so will only work with the 32-bit version of Pure Data. If you want to or need to build your own linked file from the source code file *x.c*, following are instructions for a Windows system. (Of course you have the option to set up a 64-bit system using MinGW-w64.):

* Install 32-bit Pure Data from [this site](https://puredata.info/downloads/pure-data). Choose the version: *Installer for Microsoft Windows 32 bit executable (XP or later)*
* Install MinGW (32-bit Minimilist GNU for Windows - formerly MinGW32) from [here](https://sourceforge.net/projects/mingw/). MinGW includes a C-compiler named *gcc*. After the MinGW Installation Manager is installed and started as a regular (non-administrative) user:
   - Select *Basic Setup* in the left panel
   - Right-click on mingw32-base in the right panel, select *mark for installation*.
   - Click on the menu item *Installation*, then click on *Apply Changes*.
   - Wait for components to be downloaded and installed, then close the Installation Manager.
   - Set the path variable:
     - open the Windows settings
     - find *Edit environmental variables for your account*
     - selct and edit the Path variable for your Windows account
     - add a new path: *C:\MinGW\bin*
     - test by entering *mingw32-gcc --version* in a shell
* Create a sub-directory "x" in your Pd/externals directory and put *x.c* and *x-help.pd* in it.
* Open a shell in the "x" directory and:
  - complile x.c using: *gcc -c -I "C:\Program Files (x86)\Pd\src" -o seld.dll.o seld.c*
  - if no errors, link *x.dll.o* using: *gcc -shared --enable-auto-import "C:\Program Files (x86)\Pd\bin\pd.dll" -o seld.dll seld.dll.o*
  - if no errors the file *x.dll* will appear in the same directory
  - when you launch Pd and create an "x" object, Pd will automatically load *x.dll*
  - compiling and linking can be done with a batch file
  - *m_pd.h* and *pd.dll* can be copied from the *src* and *bin* directories into the "x" directory for compiling and linking, respectively, to shorten the commands 
