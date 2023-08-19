# Pure-Data externals

* [pd-demo](https://github.com/metamystical/pure-data/tree/main#pd-demo)
* [pd-purdie](https://github.com/metamystical/pure-data/tree/main#pd-purdie)
* [pd-memorex](https://github.com/metamystical/pure-data/tree/main#pd-memorex)
* [pd-seq](https://github.com/metamystical/pure-data/tree/main#pd-seq)
* [pd-shuffle](https://github.com/metamystical/pure-data/tree/main#pd-shuffle)
* [pd-lotus](https://github.com/metamystical/pure-data/tree/main#pd-lotus)
* [pd-comma](https://github.com/metamystical/pure-data/tree/main#pd-comma)

## pd-demo
Demonstration of Pure Data (Pd) objects connected together for experimental musical composition using random notes.

### Overview

This project began with some bug fixes to the well known Pd [[shuffle ]](https://github.com/metamystical/pd-shuffle) object, which is useful for incorporating randomness into musical composition. A request for an enhancement led to the developement of three new objects to facilitate such composition:

* [[seq ]](https://github.com/metamystical/pd-seq) -- outputs a consequtive sequence of integers.
* [[purdie ]](https://github.com/metamystical/pd-purdie) -- outputs a series of integers (optionally obtained from [seq ]) in random order.
* [[memorex ]](https://github.com/metamystical/pd-memorex) -- records and plays back the output of [purdie ].

This repository contains a demo patch (pd-demo.pd) showing how to connect together these three objects to experiment with random note sequences. A short video of the demo patch in operation (pd-demo.mkv) is also included .

### Description

The heart of the demo patch is [purdie ], which outputs a random series of integers without repeating any of them, then repeats the series after shuffling the integers. The integers in the series can be preloaded as a consequtive sequence from [seq ] then augmented by inputting arbitrary individual integers, which can have the effect of removing those integers from the series if they were previously added. Like [shuffle ], [purdie ] has an optional setting to avoid repeating the end of one series at the beginning of the next.

If [memorex ] is attached to the output of [purdie ] before sending the notes to a MIDI synthesizer, one can listen to the random series until an interesting note combination is heard. By quickly toggling the **replay** input, the combination can be captured.

## pd-purdie
Related to [[shuffle ] object](https://github.com/metamystical/pd-shuffle) for Pure Data.

### Description

[purdie ] outputs a series of integers in random order without repeating any, then reshuffles. 

Initially the series is empty and **bang** has no effect. A **reset** message to the leftmost input returns the series to its initial state. 

Add an integer to the series by sending it to the left input. If the integer is already in the series, it is removed from the series until restored by adding it again. In either case, a reshuffle occurs. If a list of numbers is input, they will be added one at a time in order from left to right.

Inputs are described in the following table. All selectors can be sent as messages to the left input with arguments as specified:

| Selector     | Input | Message argument | Effect                            |
|--------------|-------|------------------|-----------------------------------|
| **bang**     | left  | none             | Output next integer in the series. |
| **number**   | left  | float (truncated to integer)  | Add an integer to the series and reshuffle. If the integer is already in the series, it is removed. |
| **reset**    | left  | none             | Message to empty the series. |
| **fraction** | right | float            | Update **fraction** (0 <= **fraction** <= 0.5) - if nonzero, ensures that the last fraction (portion) of the series is not repeated at the beginning of the next series. Takes effect on the next reshuffle. Default is zero. |

Note: the initial value of **fraction** can be specified within the object box itself [purdie fraction ]. Default is zero.

Note:  Use in combination with [[seq ]](https://github.com/metamystical/pd-seq) to initialize to a range of integers similar to [shuffle ]. Connect the output of [seq ] to the **number** input.

### Installation

The purdie.c file can be compiled into a Pure Data external using [pure-data/pd-lib-builder](https://github.com/pure-data/pd-lib-builder). Simply place the linked file (e.g. purdie.pd_linux) along with the help patch purdie-help.pd into the same directory as your patch, or somewhere on the Pd search path (e.g., externals/purdie folder).

## pd-memorex
[memorex ] object for Pure Data -- play back recent integer inputs.

### Description

[memorex ] records input history in a memory buffer of size **size**. Each input can be a single integer **note** or a list of integers **chord** numbering up to **num_notes**, the second argument of the object [memorex size num_notes]. The default value of **num_notes** is one.

There are two modes of operation controlled by the **replay** input. When **replay** is off (0), the **note** input is passed through to the output
while recording it in the memory buffer. When **replay** is on (1), the recorded **note** input history is replayed to the outputs using the changing
**note** input only as a trigger to sequence through the buffer while ignoring its value.

The **size** input sets the size of the memory buffer. If it is positive, the buffer is FIFO (first in, first out). If negative, the buffer
is LIFO (last in, first out). Changing it erases the buffer history. Its initial value can be set as the first object argumment [memorex size num_notes]

Inputs are described in the following table. All selectors can be sent as messages to the left input with arguments as specified:

| Selector      | Input  | Message argument | Effect |
|---------------|--------|------------------|--------|
| **note**      | left   | float (truncated to integer) | Integer input. Acts as a trigger during replay. |
| **replay**    | right  | float (truncated to integer) | Send the contents of the buffer to the ouput. |
| **size**      |        | float (truncated to integer) | Size of the buffer. If negative, replay in reverse order. Changing it erases the buffer. |
| **num_notes** |        | float (truncated to integer) | Number of notes in a chord. Default one. Minimum 1. Changing it erases the buffer.|

Note: the initial values of **size** and **num_notes** can be specified within the object box itself [memorex size num_notes]. Defaults are zero and one respectively.

Note: a **list** input consisting of a series of inteters following the **list** selector will be treated as a chord. All the notes of the chord will be stored in the same memory buffer location and will be output or replayed to the rightmost output as a message. The first note in the chord (the first integer in the series), which may be the only non-zero note if only a single integer was input, is simultaneously sent as a float to the leftmost output.

### Installation

The memorex.c file can be compiled into a Pure Data external using [pure-data/pd-lib-builder](https://github.com/pure-data/pd-lib-builder). Simply place the linked file (e.g. memorex.pd_linux) along with the help patch memorex-help.pd into the same directory as your patch, or somewhere on the Pd search path (e.g., externals/memorex folder).

## pd-seq
Accessory for [[purdie ] object](https://github.com/metamystical/pd-purdie) for Pure Data.

### Description

[seq ] ouputs a consecutive sequence of integers of size **size** beginning with **base**. 

Inputs are described in the following table. All selectors can be sent as messages to the left input with arguments as specified:

| Selector     | Input   | Message argument | Effect |
|--------------|---------|------------------|--------|
| **bang**     | left    | none             | Output the full integer sequence. |
| **reset**    | left    | none             | Message to restore the initial values of **base** and **size**. |
| **fast**     | left    | float (0 or 1)   | When toggled, output the series rapid-fire instead of one bang at a time. |
| **base**     | middle  | float (truncated to integer)  | Set the starting integer. |
| **size**     | right   | float (truncated to integer)  | Set the size of the series; if negative, sequence is decreasing. |

Note: the initial values of **base** and **size** can be specified within the object box itself [seq base size ]. Defaults are zero.

Note: changing any of the inputs (besides **bang**) causes the sequence to restart from **base**.

### Installation

The seq.c file can be compiled into a Pure Data external using [pure-data/pd-lib-builder](https://github.com/pure-data/pd-lib-builder). Simply place the linked file (e.g. seq.pd_linux) along with the help patch seq-help.pd into the same directory as your patch, or somewhere on the Pd search path (e.g., externals/seq folder).

## pd-shuffle
Improved [shuffle ] object for [Pure Data](https://github.com/pure-data/pure-data) - a free real-time computer music system. 

### Overview

This is a drop-in replacement for the object of the same name in the [Motex library](https://puredata.info/downloads/motex) by Iain Mott, which can crash and uses an inefficient shuffle algorithm. It is rewritten from scratch using proper memory allocation and the Fisherâ€“Yates shuffle algorithm.

### Description

[shuffle ] outputs all of the integers in a specified range in random order without repeating any, then reshuffles. Inputs are described in the following table. All selectors can be sent as messages to the left input with arguments as specified:

| Selector     | Input  | Message argument | Effect                               |
|--------------|--------|------------------|---------------------------------------|
| **bang**     | left   | none  | output next integer in the series |
| **lower**    | left   | float (truncated to integer) | update lower bound of range, trigger reset/reshuffle |
| **upper**    | middle | float (truncated to integer) | update upper bound of range |
| **fraction** | right  | float | update **fraction** (0 <= **fraction** <= 0.5) - if nonzero, ensures that the last fraction of the series is not repeated at the beginning of the next series |

Note: **upper** and **fraction** inputs do not take full effect until **lower** is input and a reset/reshuffle is triggered.

### Installation

The shuffle.c file can be compiled into a Pure Data external using [pure-data/pd-lib-builder](https://github.com/pure-data/pd-lib-builder). Simply place the linked file (e.g. shuffle.pd_linux) along with the help patch shuffle-help.pd into the same directory as your patch, or somewhere on the Pd search path (e.g., externals/shuffle folder).

## pd-lotus
[lotus ] object for Pure Data - simple message database

### Description

[lotus ] stores messages in a database file in the format "key message-string", where the key is a symbol or number and message-string can have multiple elements separated by whitespace. The database file can be created and and edited outside of Pd but can also be updated from within Pd by sending a message to the active input of the [lotus ] object with the same format used in the database file, "key message-string". Each time the file is updated, it is re-sorted
according to the keys, one line for each message.

The database filename can be specified as a parameter when the object is created, like so: [lotus filename]. If the filename is left unspecified, it is set to 'db' by default. The database file exists in the directory where the patch resides.

Stored message-strings are retrieved by sending messages to the active input of the [lotus ] object containing only a key (a symbol or number). If the passive input is toggled to '1', the message-string is removed from the database file rather than retrieved.

To store a message-string, present a store messge composed of a key (symbol or number) followed by a message-string to the active input.

If a store message contains any special characters such as comma, semicolon or dollar-sign, use a symbol box instead of a message box so that the special characters will be automatically escaped with a backslash, causing the entire store message to be treated as a single symbol. After 'Enter' is hit, the symbol box sends a message beginning with the selector 'symbol' to [lotus ] which, upon detecting this selector, unescapes the composite symbol and parses it into the "key message-string" format, including any unescaped special characters. (A message box can be used to imitate a symbol box with correct formating.)

### Installation

The lotus.c file can be compiled into a Pure Data external using [pure-data/pd-lib-builder](https://github.com/pure-data/pd-lib-builder). Simply place the linked file (e.g. lotus.pd_linux) along with the help patch lotus-help.pd into the same directory as your patch, or somewhere on the Pd search path (e.g., externals/lotus folder).

## pd-comma
[comma ] object for [Pure Data](https://github.com/pure-data/pure-data) - a free real-time computer music system. 

### Overview

A utility object that adds or removes commas from a message.

### Description

[comma ] has just one input and one output. The input accepts any message but processes messages differently depending on the number of atoms in addition to the selector.

Reminder: messages that begin with a number are automatically given the selector 'list' unless there is only one number in the message, in which case that number is given the selector 'float'. In all other cases, the selector is set to the first element in the message box.

Reminder: if the elements of a message are separated by commas, they are treated as separate messages and are output one at a time, rapid-fire.

If an incoming message contains just one atom, then that atom is stored in an internal memory array without clearing previously stored atoms, thus accumulating atoms in internal memory. Each time the internal memory is modified, the array of atoms is output along with the selector 'set' so that the atoms can be stored in a message box. If the atoms originated from a single message box with multiple atoms separated by commas, than the output message box will end up containing all of the atoms but without the commas, effectively removing the commas.

If the incoming message contains more than one atom, a 'list' selector is checked for. If missing, the message is ignored. Otherwise, the atoms are stored in internal memory separated by comma atoms, after clearing any previously stored atoms. The stored array is then ouput with a 'set' selector so that it can be stored in a message box. The new message consists of comma-separated atoms.

A 'bang' sent to the input will clear the internal memory array and output a 'set' selector without any atoms, which has the effect of clearing the message box connected to the output.

### Installation

The comma.c file can be compiled into a Pure Data external using [pure-data/pd-lib-builder](https://github.com/pure-data/pd-lib-builder). Simply place the linked file (e.g. comma.pd_linux) along with the help patch comma-help.pd into the same directory as your patch, or somewhere on the Pd search path (e.g., externals/comma folder).

