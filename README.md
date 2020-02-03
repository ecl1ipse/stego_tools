# stego_tools
A command line tool to encode and decode messages in .jpeg files

## Usage
### If the binary doesn't work
`gcc -o jpeg_stego jpeg_stego.c`
This simply compiles the code into an executable.

### To encode a message
`./jpeg_stego -e name_of_image`
Afterwards, the program will tell you the possible lengths of your message and ask you to enter the message.
After you've entered a message that can fit in the jpeg, you may be given one or multiple options about how many bits per byte you want. Select a valid option and hte message will then be encoded. (For this set of options, the smaller the number the better)

### To decode messages
`./jpeg_stego -e name_of_image`
Afterwards, the program will ask you how many bits per byter you want. Select the same bit as you used when encoding the message and the message will be printed out.

## How it works?
The program looks for metadata within jpeg files known as quantisation tables. Quantisation tables are used to compress image data and messages are encoded in images by overwriting the quantisation table.