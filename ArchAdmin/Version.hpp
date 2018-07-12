#pragma once

#include <string>

const int majorVersion = 0, minorVersion = 1, patchVersion = 0;

const int archMajorVersion = 1, archMinorVersion = 2;

const std::string versionString = "0.1.0", archVersionString = "1.2", stageString = u8"¦Â", stageStringASCII = "Beta";
const std::string compileTime = std::string(__DATE__) + " " + std::string(__TIME__);

