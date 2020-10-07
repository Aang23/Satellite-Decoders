# FengYun Decoder

Small progran that can take synced symbols from a QPSK demodulator and do the Viterbi / Diff decoding for then further processing with MetFy3x or any other software that can do so.

Synced symbols can be obtained from the demodulator flowchart in the Flowcharts directory.

Usage : `./FengYun-Decoder -i symbols.bin -o outputframes.bin`
See for more detailed syntax: `./FengYun-Decoder --help`
