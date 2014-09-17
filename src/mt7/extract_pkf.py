#!/usr/bin/python
import sys, struct, os, gzip, StringIO
def gzip_to_io(path):
    print("this file is zipped")
    f = gzip.open(path)
    output = StringIO.StringIO()
    output.write(f.read())
    f.close()
    return output
def extract_pks(path):
    k = 1
    path = sys.argv[1]
    f = open(path)
    if struct.unpack('H', f.read(2))[0] == 0x8b1f:
        f.close()
        f = gzip_to_io(path)
    f.seek(0)
    PAKS = f.read(4)
    if PAKS != 'PAKS':
        f.close()
        print "PAKS missing"
        return 
    paks_size = struct.unpack('I', f.read(4))[0]
    f.seek(paks_size)
    IPAC = f.read(4)
    if IPAC != 'IPAC':
        f.close()
        print "IPAC missing"
        return 
    ipacs_section_size = struct.unpack('I', f.read(4))[0]
    number_of_inner_sections = struct.unpack('I', f.read(4))[0]
    print("there are " + str(number_of_inner_sections) + " sections")
    f.seek(-number_of_inner_sections * 20, 2)
    for i in range(number_of_inner_sections):
        character_id = f.read(4)
        unknown = struct.unpack('I', f.read(4))[0]
        section_type = f.read(4)
        section_offset = struct.unpack('I', f.read(4))[0]
        section_size = struct.unpack('I', f.read(4))[0]
        if section_type == 'CHRM':
            f.seek(section_offset + 16)
            output_path = (path + "#%02d.MT7")  % k
            print("generating " + output_path)
            output = open(output_path, "wb")
            output.write(f.read(section_size))
            output.close()
            k += 1
        print("character id " + character_id, 
                "unknown flag " + hex(unknown),
                "section type " + section_type,
                "section size " + str(section_size),
                "section_offset " + str(section_offset))
    f.close()
if __name__ == "__main__":
    for path in sys.argv[1:]:
        extract_pks(path)
