rmmod consumer
rmmod producer
rmmod queue
dmesg --clear
insmod queue.ko queue_size=50
dmesg
insmod producer.ko msg="Sample" name="P1" items_per_second=2
insmod consumer.ko name="C1" items_per_second=1
