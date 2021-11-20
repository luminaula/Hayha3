#include "htracker.hpp"
#include "trackermodule.hpp"

HTracker::HTracker(HCore::HCore *core) : Threadable(500, "Tracker", false, core) {}

void HTracker::work() {}