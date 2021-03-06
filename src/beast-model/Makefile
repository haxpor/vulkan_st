# require VULKAN_SDK env variable set on your system (should be set via LunarG SDK)
STB_LIB_INCLUDE = -I../../externals/include
TINYOBJLOADER_LIB_INCLUDE = -I../../externals/include
CFLAGS_DEBUG = -std=c++17 -ggdb -Wall -Wextra -pedantic -I$(VULKAN_SDK)/include -I./ $(STB_LIB_INCLUDE) $(TINYOBJLOADER_LIB_INCLUDE)
CFLAGS_RELEASE = -std=c++17 -O2 -Wall -Wextra -pedantic -DNDEBUG -I$(VULKAN_SDK)/include -I./ $(STB_LIB_INCLUDE) $(TINYOBJLOADER_LIB_INCLUDE)
LDFLAGS = -lglfw -L$(VULKAN_SDK)/lib -lvulkan -lm
OUT_DEBUG = BeastModel-Debug.out
OUT_RELEASE = BeastModel.out

.PHONY: debug release test clean pre-check

release: pre-check compile-shaders VkBase.o main.o 
	g++ $(word 3, $^) $(word 4, $^) -o $(OUT_RELEASE) $(LDFLAGS)

debug: pre-check compile-shaders VkBase-d.o main-d.o
	g++ $(word 3, $^) $(word 4, $^) -o $(OUT_DEBUG) $(LDFLAGS)

main.o: main.cpp 
	g++ -c $< $(CFLAGS_RELEASE) -o $@

VkBase.o: VkBase.cpp
	g++ -c $< $(CFLAGS_RELEASE) -o $@

main-d.o: main.cpp
	g++ -c $< $(CFLAGS_DEBUG) -o $@

VkBase-d.o: VkBase.cpp
	g++ -c $< $(CFLAGS_DEBUG) -o $@

compile-shaders: shaders/main.vert shaders/main.frag
	cd shaders && ./compile.sh

# sourcing within Makefile is only possible if it's an action line only
test:
	LD_LIBRARY_PATH=$(VULKAN_SDK)/lib VK_INSTANCE_LAYERS=VK_LAYER_LUNARG_standard_validation VK_LAYER_PATH=$(VULKAN_SDK)/etc/vulkan/explicit_layer.d ./$(OUT_RELEASE)

# same
test-debug:
	LD_LIBRARY_PATH=$(VULKAN_SDK)/lib VK_INSTANCE_LAYERS=VK_LAYER_LUNARG_standard_validation VK_LAYER_PATH=$(VULKAN_SDK)/etc/vulkan/explicit_layer.d ./$(OUT_DEBUG)

clean:
	rm -f *.out *.o

pre-check:
ifndef VULKAN_SDK
	$(error VULKAN_SDK is undefined. Please set it to your vulkan SDK directory.)
endif
