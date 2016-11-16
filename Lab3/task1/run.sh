rmmod consumer
rmmod producer
rmmod queue
dmesg --clear
insmod queue.ko queue_size=3
dmesg
echo -n "0,1234567891,Message 1 - Echo 1 " > /dev/fifo
echo -n "0,2323423423,Message 2 - Echo 2 " > /dev/fifo
insmod producer.ko
insmod consumer.ko
