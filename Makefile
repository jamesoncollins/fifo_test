
OBJS = fifo_make.o
CFLAGS += -ggdb3 -O0 -std=c++11 `pkg-config --cflags --libs gtk+-3.0 gio-2.0 gmodule-export-2.0`
CPPFLAGS += $(CFLAGS)

all:	fifo_make

fifo_make:	$(OBJS)
	$(CXX) -o $@ $^ $(CFLAGS)

clean:
	rm -rf fifo_make $(OBJS)