require 'opal'
class MT7
    def initialize path
        @file_size = File.stat(path).size
        @f = File.open path, "rb"
        yield self
        @f.close
    end
    def read what
        type, size, convert = {
            unsigned_integer:   ['I', 4, :to_i],
            unsigned_short:     ['S', 2, :to_i],
            float:              ['f', 4, :to_f]
        }[what]
        @f.read(size).unpack(type)[0].send convert
    end
    def get_positions
        first_position = read :unsigned_integer
        offset = read :unsigned_integer
        @f.seek 0x10 + 24 * offset
        count = read :unsigned_short
        read :unsigned_short
        [first_position] + count.times.collect { read :unsigned_integer }
    end
    def get_xb01s positions
        positions.delete_if { |x| x == 0 }.collect do |position|
            @f.seek position
            read :unsigned_integer
            res = {
                position:   3.times.collect { read :float },
                rotation:   3.times.collect { read :unsigned_integer },
                scale:      3.times.collect { read :float },
                xb01:       read(:unsigned_integer)
            }
            next_position = 0
            while next_position == 0
                next_position = read :unsigned_integer
            end
            if next_position != 0 and not positions.include? next_position 
                positions << next_position
            end
            res
        end
    end
    def extract_faces floats_start
        @f.read(6 * 4)
        faces = []
        textures = []
        while true
            break if @f.tell() > floats_start
            counter = read :unsigned_integer
            type = counter & 0xff
            next_count = counter >> 8
            delta = 4 * (next_count - 1)
            _next = @f.tell + delta
            if type == 0
                current = @f.read 5 * 4
            elsif type == 0x10
                read :unsigned_integer
                size = read :unsigned_integer
                faces << (size.times.collect { read :unsigned_short })
                @f.seek(_next)
            elsif type == 4
                (next_count - 1).times { read :float }
            elsif type == 0xb
                read :unsigned_integer
                textures << read(:unsigned_integer)
                @f.read(delta - 8)
            else
                @f.read(delta)
            end
        end
        [faces, textures]
    end
    def extract_floats floats_start
        @f.seek floats_start
        n = (read(:unsigned_integer) >> 8) - 4
        @f.read 3 * 4
        vertices = []
        normals = []
        texture_coordinates = []
        (n/8).times do
            vertices << 3.times.collect { read :float }
            normals << 3.times.collect { read :float }
            texture_coordinates << 2.times.collect { read :float }
        end
        [vertices, normals, texture_coordinates]
    end
    def parse_xb01 xb01
        @f.seek xb01[:xb01]
        return if (xb01[:xb01] + 7 * 4) > @file_size
        read :unsigned_integer
        6.times { read :float }
        floats_start = @f.tell + read(:unsigned_integer) * 4
        @f.read(20)
        faces, textures = extract_faces floats_start
        vertices, normals,
            texture_coordinates = extract_floats floats_start
        {faces: faces, textures: textures,
         vertices: vertices, normals: normals,
         texture_coordinates: texture_coordinates}.merge xb01
    end
    def each_object
        read :unsigned_integer
        @texture_start = read :unsigned_integer
        positions = get_positions
        get_xb01s(positions).each do |xb01|
            yield parse_xb01(xb01)
        end
    end
    def operation a, b
        res = []
        a.each_with_index do |item, i|
            res << yield(item, b[i])
        end
        res
    end
    def p a, b
        operation(a, b) { |x, y| x + y }
    end
    def x a, b
        operation(a, b) { |x, y| x * y }
    end
    def each_texture_with_index
        @f.seek @texture_start
        a, b, count = 3.times.each.collect { read :unsigned_integer }
        texture_addresses = count.times.each.collect { read :unsigned_integer }
        texture_addresses.each_with_index do |texture_address, i|
            @f.seek texture_address
            @f.read 16
            yield @f.read(28 + read(:unsigned_integer)), i
        end
    end
    def to_obj
        i = 0
        delta = 1
        mt7 = self
        texture_max = 0
        obj = Wavefront.new do
            mt7.each_object do |object|
                o "object_#{i}"
                object[:vertices].each do |vertex|
                    v mt7.p(mt7.x(vertex,
                                  object[:scale]), object[:position])
                end
                object[:texture_coordinates].each { |coords| vt coords }
                object[:normals].each { |normal| n normal }
                object[:faces].each.with_index do |face, t|
                    usemtl "material_#{object[:textures][t]}"
                    texture_max = t if t > texture_max
                    face.each_slice(3).to_a.each do |a|
                        f a, delta
                    end
                end
                delta += object[:vertices].size
            end
            i += 1
        end
        pngs = []
        mtl = Wavefront::Material.new do
            mt7.each_texture_with_index do |data, i|
                png_name = "name_#{i}.png"
                pngs << Pvrx2png.new(data, png_name)
                material "material_#{i}", png_name 
            end
        end
        [obj, mtl, pngs]
    end
end
class Pvrx2png
    def initialize data, name
        @name = name
        @data = data
        @png = %x{ pvrx2png(data, name) }
    end
end
class Wavefront
    def puts a
        @contents << a
    end
    class Material
        def material name, image
            @contents += ["newmtl name",
            "Ns 96.078431",
            "Ka 0.000000 0.000000 0.000000",
            "Kd 0.640000 0.640000 0.640000",
            "Ks 0.500000 0.500000 0.500000",
            "Ni 1.000000",
            "d 1.000000",
            "illum 2",
            "map_Kd #{image}",
            "map_d #{image}"]
        end
        def initialize &block
            @contents = []
            instance_eval &block
        end
    end
    def f items, delta
        puts "f #{items.collect { |x| x + delta }.collect { |x| "#{x}/#{x}/#{x}" }.join " "}"
    end
    def method_missing name, *args, &block
        args = args.collect do |x|
            if x.instance_of? Array
                x.collect { |y| y.to_s }.join " "
            else
                x
            end
        end.join "-"
        puts "#{name} #{args}"
    end
    def initialize &block
        @contents = []
        instance_eval &block
    end
end
#MT7.new(ARGV[0]) do |mt7|
#    p mt7.to_obj
#end
