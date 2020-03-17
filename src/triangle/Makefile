CFLAGS_DEBUG = -std=c++17 -ggdb -Wall -Wextra -pedantic
CFLAGS_RELEASE = -std=c++17 -O2 -Wall -Wextra -pedantic -DNDEBUG
LDFLAGS = -lglfw -lvulkan
OUT = BasicApp.out

.PHONY: debug release test clean

main.o: main.cpp
	g++ -c $< $(CFLAGS_RELEASE) -o main.o

main-d.o: main.cpp
	g++ -c $< $(CFLAGS_DEBUG) -o main-d.o

debug: main-d.o
	g++ $< -o $(OUT) $(LDFLAGS)

release: main.o
	g++ $< -o $(OUT) $(LDFLAGS)

test:
	VK_INSTANCE_LAYERS=VK_LAYER_LUNARG_standard_validation ./$(OUT)

clean:
	rm -f $(OUT) *.o
