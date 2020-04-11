# require VULKAN_SDK env variable set on your system (should be set via LunarG SDK)
CFLAGS_DEBUG = -std=c++17 -ggdb -Wall -Wextra -pedantic -I$(VULKAN_SDK)/include -I./
CFLAGS_RELEASE = -std=c++17 -O2 -Wall -Wextra -pedantic -DNDEBUG -I$(VULKAN_SDK)/include -I./
LDFLAGS = -lglfw -L$(VULKAN_SDK)/lib -lvulkan
OUT_DEBUG = Triangle-Debug.out
OUT_RELEASE = Triangle.out

.PHONY: debug release test clean pre-check

release: pre-check VkBase.o main.o
	g++ $(word 2, $^) $(word 3, $^) -o $(OUT_RELEASE) $(LDFLAGS)

debug: pre-check VkBase-d.o main-d.o
	g++ $(word 2, $^) $(word 3, $^) -o $(OUT_DEBUG) $(LDFLAGS)

main.o: main.cpp 
	g++ -c $< $(CFLAGS_RELEASE) -o $@

VkBase.o: VkBase.cpp
	g++ -c $< $(CFLAGS_RELEASE) -o $@

main-d.o: main.cpp
	g++ -c $< $(CFLAGS_DEBUG) -o $@

VkBase-d.o: VkBase.cpp
	g++ -c $< $(CFLAGS_DEBUG) -o $@

source:
	export LD_LIBRARY_PATH=$(VULKAN_SDK)/lib
	export VK_INSTANCE_LAYERS=VK_LAYER_LUNARG_standard_validation
	export VK_LAYER_PATH=$(VULKAN_SDK)/etc/vulkan/explicit_layer.d

clean:
	rm -f *.out *.o

pre-check:
ifndef VULKAN_SDK
	$(error VULKAN_SDK is undefined. Please set it to your vulkan SDK directory.)
endif
