#!/usr/bin/python
import bpy
from bpy.props import (StringProperty,
                       BoolProperty,
                       CollectionProperty,
                       EnumProperty,
                       FloatProperty,
                       )
from bpy_extras.io_utils import (ImportHelper,
                                 ExportHelper,
                                 axis_conversion,
                                 )
from bpy.types import Operator, OperatorFileListElement
exec(open(os.path.dirname(__file__) + "/mt7_loader.py").read())
class ImportMT7(Operator, ImportHelper):
    bl_label = "Import MT7"
    bl_idname = "import_mesh.mt7"
    filename_ext = ".MT7"
    filter_glob = StringProperty(
            default="*.MT7",
            options={'HIDDEN'},
            )
    files = CollectionProperty(
            name="File Path",
            type=OperatorFileListElement,
            )
    directory = StringProperty(
            subtype='DIR_PATH',
            )
    def execute(self, context):
        paths = [os.path.join(self.directory, name.name)
                for name in self.files]
        for path in paths: load_mt7(path)
        return {'FINISHED'}

def menu_import(self, context):
    self.layout.operator(ImportMT7.bl_idname, text="Shenmue II mesh (.MT7)")
