#define CATCH_CONFIG_RUNNER
#include "catch.hpp"
#include "src/common.h"
#include <string>

int main(int argc, char *argv[]) {
	for(int i = 0; i < argc; i++) {
		if (std::string(argv[i]) == "--fix") {
			test_fix_results();
			argc--;
		}
	}
	int result = Catch::Session().run(argc, argv);
	return result;
}
