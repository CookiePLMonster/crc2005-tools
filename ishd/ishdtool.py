import sys
import struct
import os

def read_c_str(f):
    firstchar = f.read(1)
    if ( firstchar == b'' ): return ''
    return firstchar.decode('ascii') + ''.join(iter(lambda: f.read(1).decode('ascii'), '\x00'))


def unpack_ishd(ishdName, outDir):
    with open(ishdName, 'rb') as f:
        magic = f.read(4).decode('ascii')
        if magic == 'ISHD':
            while True:
                filename = read_c_str(f)
                if ( filename == '' ): break
                filesize = struct.unpack('I', f.read(4)) [0]
                with open(os.path.join(outDir, filename), 'wb') as out:
                    out.write(f.read(filesize))

def pack_ishd(inputDir, ishdName):
    files = [f for f in os.listdir(inputDir) if os.path.isfile(os.path.join(inputDir, f))]
    with open(ishdName, 'wb') as f:
        f.write(str.encode('ISHD'))
        for file in files:
            filepath = os.path.join(inputDir, file)
            f.write(str.encode(file))
            f.write(struct.pack('B', 0))

            f.write(struct.pack('I', os.path.getsize(filepath)))
            with open(filepath, 'rb') as shfile:
                f.write(shfile.read())


if len(sys.argv) < 3:
    exit()

if sys.argv[1] == '-u':
    outDir = sys.argv[3] if len(sys.argv) > 3 else 'output'
    if not os.path.exists(outDir):
        os.makedirs(outDir)

    unpack_ishd(sys.argv[2], outDir)
elif sys.argv[1] == '-p':
    ishdName = sys.argv[3] if len(sys.argv) > 3 else 'out.ishd'
    pack_ishd(sys.argv[2], ishdName)
else:
    print('Unvalid usage mode! Use -u to unpack and -p to repack.')