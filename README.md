# ip2mail
Send IPs of your raspberry pi to a specific email address automatically.

### Post
[树莓派启动之后自动发送ip到指定邮箱](https://blog.0xbbc.com/2015/07/树莓派启动之后自动发送ip到指定邮箱/)

## How to install
1. Write your own email info in main.cpp, line 327 around.
2. make && sudo make install
3. sudo vim /etc/rc.local, and add ```/etc/init.d/ip2mail``` to a proper position

#### Wanna display these info in serial?
sudo vim /etc/rc.local, and append ``` > ttyAMA0 ``` to ```/etc/init.d/ip2mail```
   
It goes like ```/etc/init.d/ip2mail > /dev/ttyAMA0```

You can replace 'ttyAMA0' with the serial port you like (If you're using another serial port)

## How to uninstall
- sudo make uninstall
- Or just run ```rm -f /etc/init.d/ip2mail```
