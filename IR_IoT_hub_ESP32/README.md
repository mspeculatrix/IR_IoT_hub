# IR_IoT_hub_ESP32

This is the code I'm currently working on. It has the option of using multitasking by running the receiver functionality on one core and the sender on the other.

You use `#define USE_MULTITASKING` at the beginning to decide whether the multitasking or non-multitasking version is compiled. Comment-out that line to get the non-multitasking version.

The non-multitasking version is working.

## ISSUES

**Multitasking version not currently working.**

It receives and interprets IR signals fine. Also handled MQTT messages (incoming and outgoing) without problem.

The issue is with sending IR signals.

I don't think it's sending a properly formed IR signal.

When an IR signal is sent, it also gets read by the sensor. And the latter reports a completely different signal to the one I'm trying to send. Often the raw data is 0xFFFFFFFF or something very close (sometimes one of those Fs is an E) and on those occasions the program crashes.

Other times it will report a protocol of 0 or 9 when I was sending 8.

I tried with a signal I know works (turning a lamp on/off) and it fails to work.

From within the mqttSubCallback() function I tried sending a signal with constant values - eg, `IrSender.sendNEC(1, 0, 5);` - but this has the same issues. _Something_ is being sent, so `IrSender()` is running. But the signal is wrong. Some kind of scope issue using IrSender() inside a core task.???

Example of what happens:

```
[signal report by sensor]
Protocol=NEC2 Address=0xFFFF Command=0xFFFE Raw-Data=0xFFFEFFFF 32 bits LSB first Repeat Gap=14250us Duration=89350us
Send with: IrSender.sendNEC2(0xFFFF, 0xFFFE, <numberOfRepeats>);
Proto: 9  +  Addr: 65535  +  Cmd: 65534  +  Flags: 33  +  Raw data: FFFEFFFF

[crash]
Guru Meditation Error: Core  0 panic'ed (InstrFetchProhibited). Exception was unhandled.

```

I'm assuming occasional crashes happen because the values coming back from the sensor somehow can't be handled by the Core 0 process. But I think the problem is the sender sending a wrong or malformed signal.
