#ifndef H5_TOOLS_HPP
#define H5_TOOLS_HPP

#include <string>
#include <vector>
#include "H5Cpp.h"

namespace H5_Tools
{
    std::vector<std::string> split(
            const std::string & str,
            const std::string & delim = "/");

    std::string ensureStringStartsAndEndsWithAPattern(
            const std::string & str,
            const std::string & delim);

    std::string ensureStringStartsWithAPattern(
            const std::string & str,
            const std::string & delim
            );

    void writeString(
            const H5::H5File& file,
            const std::string& datasetName,
            const std::string& stringToWrite);

    std::string getBasename(
            const std::string& path,
            const std::string& delims = "/\\");

    std::string replaceString(
            const std::string& subject,
            const std::string& search,
            const std::string& replace);

    std::string& replaceStringInPlace(
            std::string& subject,
            const std::string& search,
            const std::string& replace);

    H5::DataSpace createDataSpace1DUnlimited();

    H5::DataSet createDataSet(
            const H5::H5File& file, const std::string& datasetName,
            const H5::DataType& datasetType, const H5::DataSpace& space);

    H5::Group createMissingGroups(
            const H5::H5File& file,
            const std::string& datasetName);
}

#endif