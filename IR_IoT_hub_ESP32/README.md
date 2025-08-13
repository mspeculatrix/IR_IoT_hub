# IR_IoT_hub_ESP32

This is the code I'm currently working on. It has the option of using multitasking by running the receiver functionality on one core and the sender on the other.

You use `#define USE_MULTITASKING` at the beginning to decide whether the multitasking or non-multitasking version is compiled. Comment-out that line to get the non-multitasking version.

The non-multitasking version is working.

## ISSUES

**Multitasking version not currently working.**

I need to experiment with which functionality is run on which core.
