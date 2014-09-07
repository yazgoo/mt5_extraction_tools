#!/usr/bin/python
import struct
import bpy
import os
import tempfile
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
                print(coords[y])
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

def load_mt7(path):
    print("parsing " + path)
    f = open(path, "rb")
    f.read(4)
    texture_start = struct.unpack('I', f.read(4))[0]
    load_texture(f, texture_start, path)
    f.seek(8)
    f.read(4)
    offset = struct.unpack('I', f.read(4))[0]
    positions = []
    xb01s = []
    position0 = scale = position = None
    if offset == 0:
        positions = [0x10]
    else:
        print("   offset " + str(offset))
        f.seek(0x10 + 24 * offset)
        count = struct.unpack('H', f.read(2))[0]
        print("   count " + str(count))
        f.read(2)
        positions = [struct.unpack('I', f.read(4))[0] for i in range(count)]
        print("   there are " + str(len(positions)) + " sections")
    for pos in positions:
        if not pos == 0:
            f.seek(pos)
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
                # originaly pukuk.append([struct.unpack('f', f.read(4))[0] for i in range(7 * 11)])
                # start new
                pukuk.append([struct.unpack('f', f.read(4))[0] for i in range(7 * 4)])
                f.read(4)
                print("UGUU0 " + hex(f.tell()))
                count = struct.unpack('I', f.read(4))[0] >> 8
                print("UGUU0 " + str(count))
                if (f.tell() + 4 * (count - 1)) > floats_start: break
                f.read(4)
                texture = struct.unpack('I', f.read(4))[0]
                print("texture " + hex(texture))
                f.read(4 * (count - 2 - 1))
                print("UGUU1 " + hex(f.tell()))
                count = struct.unpack('I', f.read(4))[0] >> 8
                print("UGUU1 " + str(count))
                f.read(4 * (count - 1))
                print("UGUU2 " + hex(f.tell()))
                count = struct.unpack('I', f.read(4))[0] >> 8
                print("UGUU2 " + str(count))
                f.read(4 * (count + 1 - 2))
                next_section = struct.unpack('I', f.read(4))[0] & 0xffff
                print("UGUU3 " + hex(next_section))
                f.read(4)
                # new
                if (f.tell()) > floats_start: break
                size = struct.unpack('I', f.read(4))[0]
                print("      size @" + hex(f.tell()) + " " + str(size))
                if (f.tell() + size * 2) > floats_start: break
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
            mesh = bpy.data.meshes.new("mesh datablock name" + str(i))
            faces = [faces[x:x+3] for x in range(0, len(faces), 3)]
            mesh.from_pydata(verts, [], faces)
            mesh.update()
            o = bpy.data.objects.new("object" + str(i), mesh)
            o.data = mesh 
            bpy.context.scene.objects.link(o)
            o.location = position
            if load_image(mesh, path + "#01.png"):
                set_texture_coordinates(o, mesh, texture_coordinates, faces)
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
    render_to_image(tempfile.gettempdir() + "/" + mt7.replace("/", "_"))

