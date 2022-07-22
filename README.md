# procfnt
A naive but opensource and copyleft PES 2014 font file packer/unpacker.

## Usage
using -? option to view the available options.

## Notes
* In file FontFile.h, a macro named FNT_TEX_GRP_COUNT is defined, for different font file, you should modify this value to fit the specific file. Because some files holds different header, contains more/less possible groups in the file. for now, two headers have been found, which contains 36 or 0x36 groups. Check the font_header_t in FontFile.h for details.

## Scripts
