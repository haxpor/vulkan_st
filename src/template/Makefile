CFLAGS = -std=c++14 -ggdb -O2 -Wall -Wextra -pedantic -I../../externals/include
LDFLAGS = -lglfw -lvulkan
OUT = VulkanTest.out

$(OUT): main.cpp
	g++ $(CFLAGS) -o $(OUT) main.cpp $(LDFLAGS)

.PHONY: test clean

test: $(OUT)
	VK_INSTANCE_LAYERS=VK_LAYER_LUNARG_standard_validation ./$(OUT)

clean:
	rm -f $(OUT) *.o
