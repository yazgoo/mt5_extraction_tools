export PVR2PNG="wine ~/dev/shenmuesubs-code/Common/Binaries/pvrx2png.exe";
for f in $(find ~/Downloads/s2/out -iname ${mt7_prefix}*.PKF)
do
    ./extract_pkf.py $f
done
