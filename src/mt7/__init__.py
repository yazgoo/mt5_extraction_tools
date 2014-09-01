#!/usr/bin/python
bl_info = {
        "name": "MT7",
        "author": "yazgoo",
        "location": "File > Import > MT7",
        "description": "import shenmue II MT7",
        "category": "Import-Export"}
import os
exec(open(os.path.dirname(__file__) + "/mt7_ui.py").read())
import bpy
from bpy.props import *
def register():
    bpy.utils.register_module(__name__)
    bpy.types.INFO_MT_file_import.append(menu_import)

def unregister():
    bpy.utils.unregister_module(__name__)
    bpy.types.INFO_MT_file_import.remove(menu_import)

if __name__ == "__main__":
    register()
