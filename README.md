# CRC 2005 Tools

This is a collection of tools for Cross Racing Championship 2005. Currently, this repository only contains tools required to edit shaders.

## Requirements

- `ishdtool` requires Python 3.x.
- `crc2005cmd` relies on game's executable to pack/unpack LZW archives. Currently, only v1.2.4 (Build 907) is supported.
Moreover, [Ultimate ASI Loader](https://github.com/ThirteenAG/Ultimate-ASI-Loader/) (included in the archive) is required to load the plugin.

## Tools included

### CRC2005Cmd
This ASI plugin for the game adds several commandline arguments to the game, effectively turning it into a CLI tool.
Run the game with a `--cmd` argument followed by tool parameters to run the game in tool mode.

Supported features:
- Decompressing LZW files (`shader.dat`) - `-u input_file [-o output_file]`
- Compressing LZW files (`shader.dat`) - `-p input_file [-o output_file]`

### ISHD Tool
This script allows unpacking a ISHD shader archive to separate shader sources, as well as packing it back.

Supported features:
- Unpacking ISHD archive to a directory - `-u input_file [output_dir]`
- Packing a directory into an ISHD archive - `-p input_dir [output_file]`


## Modifying shaders

Combining both steps allows to freely edit any shader file in game, originally packed in a `shader.dat` archive.
Both unpacking and packing are two step processes - first, a LZW file needs to be decompressed,
then a resulting ISHD archive needs to be unpacked to text files.

### Example

Assuming `ishdtool.py` is placed in game directory and CRC2005Cmd is installed correctly,
use the following commands to extract all shaders to `system\shaders` directory:

```
crc.exe --cmd -u system\shader.dat -o system\shader.ishd
python ishdtool.py -u system\shader.ishd system\shaders
```

After you're done editing shaders, use the following commands to pack them back:

```
python ishdtool.py -p system\shaders system\shader.ishd
crc.exe --cmd -p system\shader.ishd -o system\shader.dat
```
