while true
do
./ffplay  ../media/video/H.264+AAC_1920X816.mkv &
sleep 15
killall ffplay
done
