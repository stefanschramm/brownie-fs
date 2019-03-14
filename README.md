# brownie-fs

Implementation of a readonly FUSE-"file system" as used in BrowniePlayer images and brownie-configurator.

Build dependency:

sudo apt-get install libfuse-dev

Runtime dependency:

sudo apt-get install fuse

"File system" structure:

| Byte offest | Length (bytes) | Description                    |
|-------------|----------------|--------------------------------|
| 0x00        | 8              | Prefix "BROWNIE1"              |
| 0x08        | 8              | Number of entries              |
| 0x10        | 8              | Offset of first entry (bytes)  |
| 0x18        | 8              | Length of first entry (bytes)  |
| 0x20        | 8              | Offset of second entry (bytes) |
| 0x28        | 8              | Length of second entry (bytes) |
| ...         | ...            | ...                            |
| ...         | ...            | actual data                    |

Values are little endian 64 bit unsigned integers.

