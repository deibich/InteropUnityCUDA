#include "source_volume_loader.h"

#include <iostream>
#include <string>

#include "deviceCode.h"

#include "tinyxml2.h"
#include "cfitsio/fitsio.h"

size_t extractSourceRegionsFromCatalogueXML(const std::string& filePath, std::vector<SourceRegion>& sourceBoxes)
{
    struct SourceRegionLocation {
        std::map<std::string, int> boxMinMax = {
          {"x_min", -1},
          {"x_max", -1},
          {"y_min", -1},
          {"y_max", -1},
          {"z_min", -1},
          {"z_max", -1}
        };

        box3i toBox3i() {
            return { {boxMinMax["x_min"],boxMinMax["y_min"],boxMinMax["z_min"]}, {boxMinMax["x_max"],boxMinMax["y_max"],boxMinMax["z_max"]} };
        }

        box3f toBox3f() {
            const auto x_min = static_cast<float>(boxMinMax["x_min"]);
            const auto x_max = static_cast<float>(boxMinMax["x_max"]);
            const auto y_min = static_cast<float>(boxMinMax["y_min"]);
            const auto y_max = static_cast<float>(boxMinMax["y_max"]);
            const auto z_min = static_cast<float>(boxMinMax["z_min"]);
            const auto z_max = static_cast<float>(boxMinMax["z_max"]);
            return { {x_min, y_min, z_min}, {x_max,y_max,z_max} };
        }
    };

    size_t voxels = 0;
    tinyxml2::XMLDocument doc;
    auto xmlError = doc.LoadFile(filePath.c_str());
    if(xmlError != tinyxml2::XML_SUCCESS)
    {
        return -1;
    }
    
      //<VOTABLE>
      //  <RESOURCE>
      //    ...
      //    ...
      //    <TABLE>
      //      <FIELD name="fieldname1" datatype="datatype" .... />
      //      <FIELD name="fieldname2" datatype="datatype" .... />
      //      ...
      //      <FIELD name="fieldnameX" datatype="datatype" .... />
      //      <DATA>
      //        <TABLEDATA>
      //          <TR>
      //            <TD> value for fieldname1 </TD>
      //            <TD> value for fieldname2 </TD>
      //            ...
      //            <TD> value for fieldnameX </TD>
      //          </TR>
      //          <TR>
      //            ...
      //          </TR>
      //            ...
      //          <TR>
      //          </TR>
      //        </TABLEDATA>
      //      </DATA>

      //    </TABLE>
      //  </RESOURCE>
      //</VOTABLE>
    
    auto tableElement = doc.FirstChildElement("VOTABLE")->FirstChildElement("RESOURCE")->FirstChildElement("TABLE");
    auto tableChildNameEntrys = tableElement->FirstChild();

    auto dataEntry = tableElement->LastChildElement("DATA")->FirstChildElement("TABLEDATA")->FirstChild();

    for (; dataEntry != nullptr; dataEntry = dataEntry->NextSibling())
    {
        SourceRegionLocation srl{};
        auto tdEntry = dataEntry->FirstChild();
        for (auto currTableChildNameEntry = tableChildNameEntrys; currTableChildNameEntry != nullptr; currTableChildNameEntry = currTableChildNameEntry->NextSibling())
        {
            auto tableChildElement = currTableChildNameEntry->ToElement();

            if (tableChildElement == nullptr || !tableChildElement->NoChildren())
            {
                break;
            }
            std::string attributeName = tableChildElement->Attribute("name");
            if (!attributeName.empty() && srl.boxMinMax.find(attributeName) != srl.boxMinMax.end())
            {
                auto entryValue = tdEntry->ToElement()->IntText();
                srl.boxMinMax[attributeName] = entryValue;
            }
            tdEntry = tdEntry->NextSibling();
        }
        SourceRegion sr;
        sr.gridSourceBox = srl.toBox3i();
        sr.bufferOffset = voxels;
        voxels += sr.gridSourceBox.volume();
        sr.sourceBoxNormalized = srl.toBox3f();
        sourceBoxes.push_back(sr);
    }

    return voxels;
}

vec3i loadDataForSources(const std::string& filePath, std::vector<SourceRegion>& sourceBoxes, std::vector<float>& dataBuffer)
{
    long firstPx[3];
    long lastPx[3];
    long inc[3] = { 1,1,1 };
    size_t nextDataIdx = 0;

    fitsfile* fitsFile;
    int fitsError;
    ffopen(&fitsFile, filePath.c_str(), READONLY, &fitsError);
    assert(fitsError == 0);

    int axisCount;
    int imgType;
    long axis[3];
    fits_get_img_param(fitsFile, 3, &imgType, &axisCount, &axis[0], &fitsError);
    assert(fitsError == 0);
    assert(axisCount == 3);
    assert(imgType == FLOAT_IMG);


    const box3i dataCubeVolume({ 0,0,0 }, { axis[0] - 1, axis[1] - 1, axis[2] - 1 });

    float nan = NAN;

    for (auto& sourceBox : sourceBoxes)
    {
        sourceBox.bufferOffset = nextDataIdx;
        firstPx[0] = sourceBox.gridSourceBox.lower.x + 1;
        firstPx[1] = sourceBox.gridSourceBox.lower.y + 1;
        firstPx[2] = sourceBox.gridSourceBox.lower.z + 1;

        lastPx[0] = sourceBox.gridSourceBox.upper.x;
        lastPx[1] = sourceBox.gridSourceBox.upper.y;
        lastPx[2] = sourceBox.gridSourceBox.upper.z;

        fits_read_subset(fitsFile, TFLOAT, firstPx, lastPx, inc, &nan, dataBuffer.data() + nextDataIdx, 0, &fitsError);
        assert(fitsError == 0);
        nextDataIdx += sourceBox.gridSourceBox.volume();
    }

    ffclos(fitsFile, &fitsError);
    return { axis[0], axis[1], axis[2] };
}

extern "C" {
	UNITY_INTERFACE_EXPORT int UNITY_INTERFACE_API loadSources(const char* fitsFilePath, const char* catalogFilePath)
	{
		try
		{
            std::vector<SourceRegion> sourceRegions;
            std::vector<float> sourcesDataBuffer;

            const auto fitsFilePathS = std::string(fitsFilePath);
            const auto catalogFilePathS = std::string(catalogFilePath);

            const auto bufferSize = extractSourceRegionsFromCatalogueXML(catalogFilePathS, sourceRegions);
            if(bufferSize < 1)
            {
                return -1;
            }
            sourcesDataBuffer.resize(bufferSize);
            const auto volumeDimension = loadDataForSources(fitsFilePathS, sourceRegions, sourcesDataBuffer);
            return bufferSize;
		}
		catch (...)
		{
            return -1;
		} 
	}
}
