#pragma once

#include <util/string.h>

namespace thc {
namespace core {
namespace visualizer {

class Visualizer {
public:
	static bool VisualizeSPIRV(const utils::String& filename);
	static bool VisualizeSPIRV(utils::List<uint32> code);
};

}
}
}