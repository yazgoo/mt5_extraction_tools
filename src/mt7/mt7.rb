class MT7
    def initialize path
        @file_size = File.stat(path).size
        @f = File.open path, "rb"
        yield self
        @f.close
    end
    def read what
        type, size = {unsigned_integer:['I', 4],
            unsigned_short:['H', 2],
            float: ['f', 4]}[what]
        (@f.read size).unpack(type)[0].to_i
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
            res = {position: 3.times.collect { read :float },
             rotation: 3.times.collect { read :unsigned_integer },
             scale: 3.times.collect { read :float },
            xb01: read(:unsigned_integer)}
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
                faces << size.times.collect { read :unsigned_short}
                @f.seek(_next)
            elsif type == 4
                (next_count - 1).times { read :float }
            elsif type == 0xb
                read :unsigned_integer
                texture = read :unsigned_integer
                current = @f.read(delta - 8)
            else
                current = @f.read(delta)
            end
        end
        faces
    end
    def extract_floats floats_start
        @f.seek floats_start
        n = (read(:unsigned_integer) >> 8) - 4
        @f.read 3 * 4
        vertices = []
        normals = []
        texture_coordinates = []
        (n/9).times do
            vertices << 3.times.collect { read :float }
            normals << 3.times.collect { read :float }
            texture_coordinates << 3.times.collect { read :float }
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
        faces = extract_faces floats_start
        p faces
        vertices, normals, texture_coordinates = extract_floats floats_start
    end
    def to_html
        read :unsigned_integer
        texture_start = read :unsigned_integer
        positions = get_positions
        xb01s = get_xb01s positions 
        xb01s.each do |xb01|
            parse_xb01 xb01
        end
    end
end
MT7.new(ARGV[0]) do |mt7|
    p mt7.to_html
end
