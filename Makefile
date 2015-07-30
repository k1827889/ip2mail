source := main.cpp
target := ip2mail

CC = g++

all: $(objects)
	$(CC) -o $(target) $(source)
install: 
	cp -f $(target) /etc/init.d/$(target)
	chmod 0775 /etc/init.d/$(target)
uninstall:
	rm -f /etc/init.d/$(target)
clean: 
	@rm -f *.o
	@rm $(target)
