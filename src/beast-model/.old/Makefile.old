# require VULKAN_SDK env variable set on your system (should be set via LunarG SDK)
CFLAGS_DEBUG = -std=c++17 -ggdb -Wall -Wextra -pedantic -I$(VULKAN_SDK)/include
CFLAGS_RELEASE = -std=c++17 -O2 -Wall -Wextra -pedantic -DNDEBUG -I$(VULKAN_SDK)/include
LDFLAGS = -lglfw -L$(VULKAN_SDK)/lib -lvulkan
OUT = BasicApp.out

.PHONY: debug release test clean pre-check

release: pre-check main.o
	g++ $(word 2, $^) -o $(OUT) $(LDFLAGS)

debug: pre-check main-d.o
	g++ $(word 2, $^) -o $(OUT) $(LDFLAGS)

main.o: main.cpp
	g++ -c $< $(CFLAGS_RELEASE) -o main.o

main-d.o: main.cpp
	g++ -c $< $(CFLAGS_DEBUG) -o main-d.o

test:
	LD_LIBRARY_PATH=$(VULKAN_SDK)/lib VK_INSTANCE_LAYERS=VK_LAYER_LUNARG_standard_validation:VK_LAYER_LUNARG_monitor VK_LAYER_PATH=$(VULKAN_SDK)/etc/vulkan/explicit_layer.d ./$(OUT)

test-gdb:
	LD_LIBRARY_PATH=$(VULKAN_SDK)/lib VK_INSTANCE_LAYERS=VK_LAYER_LUNARG_standard_validation VK_LAYER_PATH=$(VULKAN_SDK)/etc/vulkan/explicit_layer.d gdb ./$(OUT)

clean:
	rm -f $(OUT) *.o

pre-check:
ifndef VULKAN_SDK
	$(error VULKAN_SDK is undefined. Please set it to your vulkan SDK directory.)
endif
