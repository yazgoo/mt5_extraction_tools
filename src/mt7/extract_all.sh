extract()
{
    #md5=/tmp/$(md5sum ${f} | cut -d ' ' -f 1).md5
    file_path=$(echo $f | sed 's,/,_,g' | sed 's,\(.*\),/tmp/\1.png,')
    #if [ -e $md5 ]
    #then
    #    cp $(cat $md5) $file_path
    #else
        MT7=$f blender --background --python ./mt7_loader.py 1>/dev/null 2>&1
        #echo $file_path > $md5
    #fi
    test -e $file_path
}
failed()
{
    echo -n x
    echo $f >> /tmp/error
}
rm /tmp/*.md5;
rm /tmp/error;
export PVR2PNG="wine ~/dev/shenmuesubs-code/Common/Binaries/pvrx2png.exe";
for f in $(find ~/Downloads/s2/out -iname ${mt7_prefix}*.MT7); do extract || failed && echo -n "-"; done; echo
