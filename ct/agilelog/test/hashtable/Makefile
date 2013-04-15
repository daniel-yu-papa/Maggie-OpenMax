OBJS := hashTable.o hashtable_test.o
CFLAGS += -I../../inc -fpermissive

htTest: $(OBJS)
	g++ -o htTest $(OBJS)

hashTable.o: ../../src/hashTable.cpp ../../inc/hashTable.h
	g++ -c $(CFLAGS) ../../src/hashTable.cpp

hashtable_test.o: hashtable_test.cpp ../../inc/hashTable.h
	g++ -c $(CFLAGS) hashtable_test.cpp

clean:
	rm *.o htTest
