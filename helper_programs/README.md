# Helper programs

## Why are there helper programs?
The helper programs are either previous versions of jpeg_stego or programs that were used to help debug.

## Compiling
To compile a tool
`gcc -o tool_name tool_name.c`

###  hex_reader
A tool that prints file data as hex values
#### Usage
`./hex_reader file_name`

### qt_encoder
The first version of the encoding functionality of jpeg_stego. What this program does is now contained in jpeg_stego.
#### Usage
`./qt_encoder message file_name`

Then type in the rest of the requested information.

### qt_decoder
The first version of the decoding functionality of jpeg_stego. What this program does is now contained in jpeg_stego.
`./qt_decoder file_name`

Then type in the rest of the requested information.

### uint8_t_reader
A tool that prints file data as hex values
#### Usage
`./uint8_t_reader file_name`