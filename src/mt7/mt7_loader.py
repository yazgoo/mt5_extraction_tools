#!/usr/bin/python
import struct
import bpy
import os
import tempfile
def render_to_image(path):
    bpy.data.scenes['Scene'].render.filepath = path
    bpy.ops.render.render( write_still=True ) 
def load_mt7(path):
    f = open(path, "rb")
    f.read(4)
    f.read(4)
    f.read(4)
    offset = struct.unpack('I', f.read(4))[0]
    print("   offset " + str(offset))
    f.seek(0x10 + 24 * offset)
    count = struct.unpack('H', f.read(2))[0]
    print("   count " + str(count))
    f.read(2)
    positions = [struct.unpack('I', f.read(4))[0] for i in range(count)]
    print("   there are " + str(len(positions)) + " sections")
    xb01s = []
    position0 = scale = position = None
    for pos in positions:
        if not pos == 0:
            f.seek(pos)
            position0 = [ struct.unpack('f', f.read(4))[0] for i in range(10)]
            #f.read(40 - 16)
            xb01 = struct.unpack('I', f.read(4))[0]
            lol = [ struct.unpack('f', f.read(4))[0] for i in range(5)]
            f.read(4) # should be mdcx
            floats3 = [ struct.unpack('f', f.read(4))[0] for i in range(9)]
            m = 0
            # xb01 position scale rotation
            xb01s.append([xb01, floats3[m:m+3], floats3[m+3:m+6], floats3[m+6:m+9]])
            print("position0")
            print(position0)
            print("lol")
            print(lol)
            print("floats3")
            print(floats3)
    print("   there are " + str(len(xb01s)) + " xb01s")
    i = 0
    for xb01, position, scale, rotation in xb01s:
        float2 = float4 = []
        if not xb01 == 0:
            i += 1
            f.seek(xb01)
            print("   xb01 " + str(i) + " @" + hex(f.tell()))
            f.read(4) # should be xb01
            float2 = [struct.unpack('f', f.read(4))[0] for i in range(6)]
            floats_start = f.tell() + struct.unpack('I', f.read(4))[0] * 4
            print("      floats start " + hex(floats_start))
            f.read(5 * 4)
            faces = []
            pukuk = []
            while True:
                pukuk.append([struct.unpack('f', f.read(4))[0] for i in range(11 * 4)])
                if (f.tell()) > floats_start: break
                size = struct.unpack('I', f.read(4))[0]
                if (f.tell() + size * 2) > floats_start: break
                print("      size @" + hex(f.tell()) + " " + str(size))
                for k in range(size): 
                    a = struct.unpack('h', f.read(2))[0]
                    faces.append(a)
            f.seek(floats_start)
            floats_n = struct.unpack('I', f.read(4))[0] >> 8
            floats_n -= 4
            f.read(3 * 4)
            print("      floats @" + hex(f.tell()) + "? " + str(floats_n))
            verts = []
            norms = []
            texture_coordinates = []
            vert = []
            norm = []
            text = []
            for j in range(floats_n):
                _f = struct.unpack('f', f.read(4))[0]
                if j % 8 == 0:
                    norm = []
                    vert = []
                    text = []
                if j % 8 < 3:
                    vert.append(_f)
                elif j % 8 < 6:
                    norm.append(_f)
                else:
                    text.append(_f)
                    if j % 8 == 7:
                        norms.append(norm)
                        verts.append(vert)
                        texture_coordinates.append(text)
            #print(verts)
            mesh = bpy.data.meshes.new("mesh datablock name" + str(i))
            faces = [faces[x:x+3] for x in range(0, len(faces), 3)]
            mesh.from_pydata(verts, [], faces)
            mesh.update()
            o = bpy.data.objects.new("object" + str(i), mesh)
            o.data = mesh 
            bpy.context.scene.objects.link(o)
            o.location = position
            #o.scale = scale
    f.close()
def remove_cube():
    scene = bpy.context.scene
    for ob in scene.objects:
        if ob.type == 'MESH' and ob.name.startswith("Cube"):
            ob.select = True
        else: 
            ob.select = False
    bpy.ops.object.delete()
if __name__ == "__main__":
    remove_cube()
    mt7 = os.environ.get("MT7")
    load_mt7(mt7)
    render_to_image(tempfile.gettempdir() + "/" + mt7.split("/")[-1])

