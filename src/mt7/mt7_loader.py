#!/usr/bin/python
import struct
import bpy
import os
import tempfile
import re
from mathutils import Vector
def look_at(obj_camera, point):
    loc_camera = obj_camera.matrix_world.to_translation()
    direction = point - loc_camera
    #rot_quat = direction.to_track_quat('-Z', 'Y')
    rot_quat = direction.to_track_quat('-Z', 'Y')
    obj_camera.rotation_euler = rot_quat.to_euler()
def scale_objects():
    _max = 0.0
    for obj in bpy.data.objects:
        if obj.name != "Camera" and obj.name != "Lamp":
            l = [_max]
            m1 = max(obj.dimensions)
            if m1 > _max: _max = m1
    maxDimension = 5.0
    print("=========== max " + str(_max))
    scaleFactor = maxDimension / _max
    for obj in bpy.data.objects:
        if obj.name != "Camera" and obj.name != "Lamp":
            obj.scale = (scaleFactor,scaleFactor,scaleFactor)
            obj.location = obj.location * scaleFactor
def center_objects():
    s = Vector((0, 0, 0))
    locations = []
    count = 0
    for obj in bpy.data.objects:
        if obj.name != "Camera" and obj.name != "Lamp":
            s += obj.location
            count += 1
    print("==s==")
    print(s)
    center = s / count
    print(obj.location)
    print(center)
    print("==a==")
    for obj in bpy.data.objects:
        if obj.name != "Camera" and obj.name != "Lamp":
            obj.location -= center
            print(obj.location)
def render_to_image(path):
    bpy.data.scenes['Scene'].render.filepath = path
    camera = bpy.data.objects["Camera"]
    scale_objects()
    center_objects()
    look_at(camera, Vector((0, 0, 0)))
    lamp = bpy.data.objects["Lamp"]
    lamp.location = camera.location
    lamp.location[1] += 2

    bpy.ops.render.render( write_still=True ) 
def load_image(mesh, img_name):
    print(img_name)
    if not os.path.isfile(img_name): return False
    img = bpy.data.images.load(img_name)
    tex = bpy.data.textures.new('TexName', type = 'IMAGE')
    tex.image = img
    mat = bpy.data.materials.new('MatName')
    mtex = mat.texture_slots.add()
    mtex.texture = tex
    mtex.texture_coords = 'UV'
    mtex.use_map_color_diffuse = True 
    mtex.use_map_color_emission = True 
    mtex.emission_color_factor = 0.5
    mtex.use_map_density = True 
    mtex.mapping = 'FLAT' 
    mesh.materials.append(mat)
    return True
def set_texture_coordinates(o, me, coords, faces):
    uvtex = me.uv_textures.new()
    uvtex.name = "UVMain"
    count = 0
    for uv_layer in me.uv_layers:
        for x in faces:
            for y in x:
                uv_layer.data[count].uv = Vector((coords[y][0], 1 - coords[y][1]))
                count += 1
def load_texture(f, texture_start, path):
    f.seek(texture_start)
    # end of file: no texture
    if f.read(4) == '': return
    if f.read(4) == '': return
    count = f.read(4)
    if count == '' or len(count) < 4: return
    texture_count = struct.unpack('I', count)[0]
    texture_addresses = [texture_start + struct.unpack('I', f.read(4))[0] for i in range(texture_count)]
    i = 1
    for texture_address in texture_addresses:
        f.seek(texture_address)
        f.read(16)
        size = struct.unpack('I', f.read(4))[0] + 28
        f.seek(texture_address)
        pvr = path + "#%02d.pvr" % i
        if not os.path.isfile(pvr):
            dest = open(pvr, "wb")
            dest.write(f.read(size))
            dest.close()
        png = os.path.splitext(pvr)[0] + ".png"
        if not os.path.isfile(png):
            os.system(os.environ.get("PVR2PNG") + " " + pvr)
        i += 1

def get_xb01s(f, positions, xb01s, file_size, offset):
    for pos in positions:
        if not pos == 0:
            f.seek(pos)
            if pos + 10 > file_size: break
            print("test " + hex(f.tell()))
            position0 = [ struct.unpack('f', f.read(4))[0] for i in range(10)]
            #f.read(40 - 16)
            xb01 = struct.unpack('I', f.read(4))[0]
            next_pos = struct.unpack('I', f.read(4))[0] # sometime the initial pos table misses elements, they are around there
            print("===> " + hex(xb01) + " " + hex(f.tell()))
            if offset == 0:
                while next_pos == 0:
                    next_pos = struct.unpack('I', f.read(4))[0]
            if next_pos != 0 and not next_pos in positions:
                positions.append(next_pos) # so we add it
            lol = [ struct.unpack('f', f.read(4))[0] for i in range(4)]
            f.read(4) # should be mdcx
            floats3 = [ struct.unpack('f', f.read(4))[0] for i in range(9)]
            m = 0
            # xb01 position scale rotation
            xb01s.append([xb01, position0[m+1:m+4], floats3[m+3:m+6], floats3[m+6:m+9]])
#            print("position0")
#            print(position0)
#            print("lol")
#            print(lol)
#            print("floats3")
#            print(floats3)
def extract_faces_simple(f, faces, floats_start):
    size = struct.unpack('I', f.read(4))[0]
    print("      size @" + hex(f.tell()) + " " + str(size))
    for k in range(size): 
        a = struct.unpack('h', f.read(2))[0]
        faces.append(a)
def extract_faces(f, path, floats_start, textures, faces):
    f.read(6 * 4)
    while True:
        if (f.tell()) > floats_start: return
        counter = struct.unpack('I', f.read(4))[0]
        _type = counter & 0xff
        _next_count = counter >> 8
        delta = 4 * (_next_count - 1)
        _next = f.tell() + delta
        if _type == 0:
            f.read(5 * 4)
        elif _type == 0x10:
            print("UGUU42 @" + hex(f.tell()) + " type: " + str(_type) + " " + str(_next_count) + " " + hex(_next));
            f.read(4)
            extract_faces_simple(f, faces, floats_start)
            print("UGUU42 faces done @" + hex(f.tell()));
            f.seek(_next)
        elif _type == 0x4:
            # TODO find out what these floats are for
            print([struct.unpack('f', f.read(4))[0] for i in range(_next_count - 1)])
        elif _type == 0xb:
            f.read(4)
            textures[len(faces)] = struct.unpack('I', f.read(4))[0] 
            f.read(delta - 8)
        else:
            current = f.read(delta)
def extract_faces2(f, path, floats_start, textures, faces):
    f.read(4 * 13)
    while True:
        print("UGUU42 " + hex(f.tell()));
        [struct.unpack('f', f.read(4))[0] for i in range(14)]
        print("UGUU42 " + hex(f.tell()));
        f.read(4 * 4)
        textures[len(faces)] = struct.unpack('I', f.read(4))[0]
        f.read(4 * 9)
        print("UGUU42 " + hex(f.tell()));
        _next_count = struct.unpack('I', f.read(4))[0] >> 8 - 1
        _next = f.tell() + 4 * (_next_count)
        print("UGUU42 next_count " + str(_next_count) + "\tnext " + hex(_next) + " @" + hex(f.tell()));
        f.read(4)
        size = struct.unpack('I', f.read(4))[0]
        print("      size @" + hex(f.tell()) + " " + str(size))
        if (f.tell() + size * 2) > floats_start: break
        for k in range(size): 
            a = struct.unpack('h', f.read(2))[0]
            faces.append(a)
        f.seek(_next)
        if _next_count == 0: return
def extract_faces2(f, path, floats_start, textures, faces):
    f.read(4 * 13)
    while True:
        print("UGUU42 " + hex(f.tell()));
        [struct.unpack('f', f.read(4))[0] for i in range(14)]
        print("UGUU42 " + hex(f.tell()));
        f.read(4 * 4)
        textures[len(faces)] = struct.unpack('I', f.read(4))[0]
        f.read(4 * 9)
        print("UGUU42 " + hex(f.tell()));
        _next_count = struct.unpack('I', f.read(4))[0] >> 8 - 1
        _next = f.tell() + 4 * (_next_count)
        print("UGUU42 next_count " + str(_next_count) + "\tnext " + hex(_next) + " @" + hex(f.tell()));
        f.read(4)
        size = struct.unpack('I', f.read(4))[0]
        print("      size @" + hex(f.tell()) + " " + str(size))
        if (f.tell() + size * 2) > floats_start: break
        for k in range(size): 
            a = struct.unpack('h', f.read(2))[0]
            faces.append(a)
        f.seek(_next)
        if _next_count == 0: return
def extract_faces_verbose(f, path, floats_start, textures, faces):
        print("UGUU42 " + hex(f.tell()));
        while True:
            # originaly pukuk.append([struct.unpack('f', f.read(4))[0] for i in range(7 * 11)])
            # start new
            next_int = struct.unpack('I', f.read(4))[0]
            print(" new " + hex(next_int) + " " + path)
            if next_int == 0:
                [struct.unpack('f', f.read(4))[0] for i in range(7 * 4 - 1)]
            else:
                [struct.unpack('f', f.read(4))[0] for i in range(7 * 4 - 1 - 3 * 4)]
            f.read(4)
            _i = 0
            while True:
                print("UGUU " + hex(f.tell()))
                count = struct.unpack('I', f.read(4))[0]
                if (count & 0xff) == 0x10: break
                count = count >> 8
                if (f.tell() + 4 * (count - 1)) > floats_start: break
                print("UGUU " + str(count))
                if count == 0: # if we are not yet to a count zone, skip bytes
                    f.read(4)
                else:
                    if _i == 0: # we are at the start of the, section, we should find the texture number here
                        count -= 2
                        f.read(4)
                        texture = struct.unpack('I', f.read(4))[0]
                        textures[len(faces)] = texture
                        print("texture @" + hex(f.tell() - 4) + " " + str(texture))
                    if count > 1: f.read(4 * (count - 1))
                    _i += 1
            f.read(4)
            if (f.tell()) > floats_start: break
            s = 13
            f.seek(-4 * s, 1)
            print("before @" + hex(f.tell()) + " "+ str([struct.unpack('I', f.read(4))[0] for i in range(s)]))
            size = struct.unpack('I', f.read(4))[0]
            print("      size @" + hex(f.tell()) + " " + str(size))
            if (f.tell() + size * 2) > floats_start: break
            for k in range(size): 
                a = struct.unpack('h', f.read(2))[0]
                faces.append(a)
def load_xb01(f, xb01, i, file_size, path, position):
        f.seek(xb01)
        print("   xb01 " + str(i) + " @" + hex(f.tell()))
        if xb01 + 7 * 4 > file_size: return
        f.read(4) # should be xb01
        float2 = [struct.unpack('f', f.read(4))[0] for i in range(6)]
        floats_start = f.tell() + struct.unpack('I', f.read(4))[0] * 4
        print("      floats start " + hex(floats_start))
        f.read(5 * 4)
        faces = []
        textures = {}
        extract_faces(f, path, floats_start, textures, faces)
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
        faces_starts = sorted(list(textures.keys()))
        print("textures", textures, faces_starts)
        for k in range(len(faces_starts)):
            faces_start = faces_starts[k]
            faces_end = int(len(faces) / 3) * 3
            if (k + 1) < len(faces_starts): faces_end = faces_starts[k+1]
            texture = textures[faces_start]
            actual_faces = [faces[x:x+3] for x in range(faces_start, faces_end, 3)]
            mesh = bpy.data.meshes.new("mesh datablock name" + str(i) + "_" + str(k))
            mesh.from_pydata(verts, [], actual_faces)
            mesh.update()
            o = bpy.data.objects.new("object_" + str(i) + "_" + str(k), mesh)
            o.data = mesh 
            bpy.context.scene.objects.link(o)
            o.location = position
            texture_name = (path + "#%02d.png") % (texture + 1)
            texture_name = re.sub(r"PKS....MT7", "PKF", texture_name)
            print("texture", texture_name)
            if load_image(mesh, texture_name):
                set_texture_coordinates(o, mesh, texture_coordinates, actual_faces)
            #o.scale = scale
def load_mt7(path):
    print("parsing " + path)
    file_size = os.path.getsize(path)
    f = open(path, "rb")
    f.read(4)
    texture_start = struct.unpack('I', f.read(4))[0]
    load_texture(f, texture_start, path)
    f.seek(8)
    first_position = struct.unpack('I', f.read(4))[0]
    offset = struct.unpack('I', f.read(4))[0]
    positions = []
    xb01s = []
    position0 = scale = position = None
#    if offset == 0:
#        positions = [0x10]
#    else:
    print("   offset " + str(offset))
    f.seek(0x10 + 24 * offset)
    count = struct.unpack('H', f.read(2))[0]
    print("   count " + str(count))
    f.read(2)
    if((f.tell() + count * 4) <= first_position):
        positions = [struct.unpack('I', f.read(4))[0] for i in range(count)]
        print("   there are " + str(len(positions)) + " sections")
    if len(positions) == 0:
        print("using " + hex(first_position))
        positions.append(first_position)
    print([hex(p) for p in positions])
    get_xb01s(f, positions, xb01s, file_size, offset)
    print("   there are " + str(len(xb01s)) + " xb01s")
    i = 0
    for xb01, position, scale, rotation in xb01s:
        float2 = float4 = []
        if not xb01 == 0:
            i += 1
            load_xb01(f, xb01, i, file_size, path, position)
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
    render_to_image(tempfile.gettempdir() + "/" + mt7.replace("/", "_"))

