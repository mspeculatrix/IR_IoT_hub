# IR_IoT_hub_ESP32

This is a combination of the `IR_IoT_hub` and `IR_IoT_hub_multi` versions. It's the definitive version and means it's all in one codebase.

The code uses `#define USE_MULTITASKING` at the beginning to decide whether the multitasking or non-multitasking version is compiled. Comment-out that line to get the non-multitasking version.

## ISSUES

I don't think it's sending a properly formed IR signal. When an IR signal is sent, it also gets read by the sensor. And the latter reports a completely different signal to the one I'm trying to send. Often the raw data is 0xFFFFFFFF or something very close (sometimes one of those Fs is an E). It will report a protocol of 0 or 9 when I was sending 8.

I tried with a signal I know works (turning a lamp on/off) and it fails to work. Send the same MQTT message with the non-multitasking version of the code is fine. (It's not a hardware problem).

From within the mqttSubCallback() function I tried sending a signal with contsant values - eg, `IrSender.sendNEC(1, 0, 5);` - but this has the same issues. _Seomthing_ is being sent, so `IrSender()` is running. But the signal is wrong. Some kind of scope issue using IrSender() inside this function.???

Example of what happens:

```
[signal report by sensor]
Protocol=NEC2 Address=0xFFFF Command=0xFFFE Raw-Data=0xFFFEFFFF 32 bits LSB first Repeat Gap=14250us Duration=89350us
Send with: IrSender.sendNEC2(0xFFFF, 0xFFFE, <numberOfRepeats>);
Proto: 9  +  Addr: 65535  +  Cmd: 65534  +  Flags: 33  +  Raw data: FFFEFFFF

[crash]
Guru Meditation Error: Core  0 panic'ed (InstrFetchProhibited). Exception was unhandled.

```

I'm assuming the crash is because the values coming back from the sensor somehow can't be handled by the Core 0 process. But I think the problem is the sender sending a wrong or malformed signal.