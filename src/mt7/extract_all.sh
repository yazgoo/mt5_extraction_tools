extract()
{
    MT7=$f blender --background --python ./mt7_loader.py 1>/dev/null 2>&1
    test -e $(echo $f | sed 's,/,_,g' | sed 's,\(.*\),/tmp/\1.png,')
}
failed()
{
    echo -n x
    echo $f >> /tmp/error
}
rm /tmp/error;
export PVR2PNG="wine ~/dev/shenmuesubs-code/Common/Binaries/pvrx2png.exe";
for f in $(find ~/Downloads/s2/out -iname *.MT7); do extract || failed && echo -n "-"; done; echo
