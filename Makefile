CC = gcc
CFLAGS = -Wall

JSON_OBJS = json.o json_new.o json_clear.o json_show.o json_select.o json_set.o json_parse.o

sync: sync.o $(JSON_OBJS)
test_cmp: test_cmp.o $(JSON_OBJS)
readable: readable.o $(JSON_OBJS)

clean:
	rm -f *~ $(JSON_OBJS) sync.o test_cmp.o readable.o
