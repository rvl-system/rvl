# Wave Protocol

This is based off of the [main Protocol](./protocol.md)

## Payload Description

The payload for a wave function contains 82 bytes as described below

* Byte 1: The time period for each wave
* Byte 2: The distance period for each wave
* Bytes 3-22: Wave 1, the Hue wave
* Bytes 23-42: Wave 2, the Saturation wave
* Bytes 43-62: Wave 3, the Value wave
* Bytes 63-82: Wave 4, the Alpha wave

### Wave breakdown

A wave is described in 5 4-byte chunks:

* Bytes 1-4: a
* Bytes 5-8: b
* Bytes 9-12: w_t
* Bytes 13-16: w_x
* Bytes 17-20: phi
