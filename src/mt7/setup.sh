dest="$HOME/.config/blender/2.69/scripts/addons/mt7"
mkdir -p "$dest"
cp __init__.py  mt7_loader.py  mt7_ui.py "$dest" && \
    echo setup done to $dest
