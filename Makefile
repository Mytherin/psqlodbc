
GENERATOR=
ifeq ($(GEN),ninja)
	GENERATOR=-G "Ninja"
endif

debug:
	mkdir -p build/debug && \
	cd build/debug && \
	cmake $(GENERATOR) -DCMAKE_BUILD_TYPE=Debug ../.. && \
	cmake --build .
