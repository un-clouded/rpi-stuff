
PROJECT=i2cf

COMPILE=$(CC) $(CFLAGS) $(INCLUDES)


%.o: %.c *.h
	echo
	$(COMPILE) -c $< -o $@

all: $(PROJECT)

clean:
	rm $(PROJECT)

