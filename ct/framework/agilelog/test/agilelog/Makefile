OBJS := agilelog_test.o agilelog.o agilelog_inf.o hashTable.o tinyxml2.o
CFLAGS += -fpermissive -I../../inc -I../../inc/pub -I../../external/tinyxml2/

all: agiletest
agiletest: $(OBJS)
	g++ -o agiletest $(OBJS)

agilelog.o: ../../src/agilelog.cpp ../../inc/agilelog_priv.h
	g++ -c $(CFLAGS) ../../src/agilelog.cpp


agilelog_inf.o: ../../src/agilelog_inf.cpp ../../inc/agilelog_priv.h
	g++ -c $(CFLAGS) ../../src/agilelog_inf.cpp

hashTable.o: ../../src/hashTable.cpp ../../inc/hashTable.h
	g++ -c $(CFLAGS) ../../src/hashTable.cpp

tinyxml2.o: ../../external/tinyxml2/tinyxml2.cpp ../../external/tinyxml2/tinyxml2.h
	g++ -c $(CFLAGS) ../../external/tinyxml2/tinyxml2.cpp

agilelog_test.o: agilelog_test.cpp ../../inc/pub/agilelog.h
	g++ -c $(CFLAGS) agilelog_test.cpp

clean:
	rm *.o agiletest
