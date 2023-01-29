# Linux driver for MAX6682MUA 
constructed using Matt Ranostay's maxim termocouple driver
## Renode
Set temperature with:
```
litespi0.maximtemp Temperature 20
```
## Linux
insert a module into the kernel:
```
insmod my_driver.ko
```

read temp from driver :
```
cat /sys/bus/iio/devices/iio\:device0/in_temp_raw
```
