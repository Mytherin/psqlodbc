
GENERATOR=
ifeq ($(GEN),ninja)
	GENERATOR=-G "Ninja"
endif

release:
	mkdir -p build/release && \
	cd build/release && \
	cmake $(GENERATOR) -DCMAKE_BUILD_TYPE=Release ../.. && \
	cmake --build . --config Release

debug:
	mkdir -p build/debug && \
	cd build/debug && \
	cmake $(GENERATOR) -DCMAKE_BUILD_TYPE=Debug ../.. && \
	cmake --build . --config Debug
