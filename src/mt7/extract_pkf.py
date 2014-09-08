#!/usr/bin/python
import sys, struct, os
path = sys.argv[1]
f = open(sys.argv[1])
f.seek(0x34 - 8)
i = 0
while True:
    f.read(8)
    start = f.tell()
    if f.read(16) == '': break
    size = struct.unpack('I', f.read(4))[0] + 28
    print(" " + str(size) + " " + hex(start))
    f.seek(start)
    pvr = path + "#%02d.pvr" % i
    if not os.path.isfile(pvr):
        dest = open(pvr, "wb")
        dest.write(f.read(size))
        dest.close()
    png = os.path.splitext(pvr)[0] + ".png"
    if not os.path.isfile(png):
        os.system(os.environ.get("PVR2PNG") + " " + pvr)
    i += 1
