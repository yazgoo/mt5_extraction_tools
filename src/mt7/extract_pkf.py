#!/usr/bin/env python
import sys, struct, os, gzip, StringIO
def open_potentially_gz(path):
    f = open(path)
    if struct.unpack('H', f.read(2))[0] != 0x8b1f:
        return f
    f.close()
    f = gzip.open(path)
    output = StringIO.StringIO()
    output.write(f.read())
    f.close()
    return output
def read_texn(f):
    start = f.tell()
    if f.read(4) != 'TEXN':
        return False
    size = struct.unpack('I', f.read(4))[0]
    f.seek(20, 1)
    return {'next': start + size, 'size': start + size - f.tell()}
def extract_texture(f, path, size, k):
    pvr = (path + "#%02d.pvr") % k
    png = os.path.splitext(pvr)[0] + ".png"
    if not os.path.isfile(png):
        if not os.path.isfile(pvr):
            print("generating " + pvr)
            out = open(pvr, "wb")
            out.write(f.read(size))
            out.close()
        print("generating " + png)
        pvr2png = os.environ.get("PVR2PNG")
        os.system(pvr2png + " " + pvr)
    return k + 1
def extract_pkf(path):
    f = open_potentially_gz(path)
    f.seek(0x24)
    k = 1
    while True:
        texn = read_texn(f)
        if not texn: break
        k = extract_texture(f, path, texn['size'], k)
        f.seek(texn['next'])
    f.close()
    return
if __name__ == "__main__":
    for path in sys.argv[1:]:
        extract_pkf(path)
