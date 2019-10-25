#ifndef MAP_CREATOR_H
#define MAP_CREATOR_H

#include <CVCapture.h>
#include <WAIHelper.h>
#include <Utils.h>
#include <AppWaiSlamParamHelper.h>
#include <WAIModeOrbSlam2.h>
#include <WAIMapStorage.h>

class MapCreator
{
    typedef std::string Location;
    typedef std::string Area;
    typedef struct VideoAndCalib
    {
        std::string videoFile;
        //std::string calibFile;
        CVCalibration calibration;
    };
    typedef std::vector<VideoAndCalib> Videos;
    typedef std::map<Area, Videos>     Areas;

public:
    MapCreator(std::string erlebARDir, std::string configFile);

    void loadSites(const std::string& erlebARDir, const std::string& configFile);
    void createNewWaiMap(const Location& location, const Area& area, Videos& videos);
    bool createNewDenseWaiMap(Videos&            videos,
                              const std::string& mapFile,
                              const std::string& mapDir,
                              const float        cullRedundantPerc,
                              std::string&       currentMapFileName);
    void thinOutNewWaiMap(const std::string& mapDir,
                          const std::string& inputMapFile,
                          const std::string  outputMapFile,
                          CVCalibration&     calib,
                          const float        cullRedundantPerc);
    void cullKeyframes(std::vector<WAIKeyFrame*>& kfs, const float cullRedundantPerc);
    void decorateDebug(WAI::ModeOrbSlam2* waiMode, CVCapture* cap, const int currentFrameIndex, const int numOfKfs);
    void saveMap(WAI::ModeOrbSlam2* waiMode, const std::string& mapDir, const std::string& currentMapFileName);
    void loadMap(WAI::ModeOrbSlam2* waiMode, const std::string& mapDir, const std::string& currentMapFileName, bool fixKfsForLBA);
    void execute();

private:
    MapCreator() {}
    std::map<Location, Areas> _erlebAR;
    std::string               _erlebARDir;
    std::string               _vocFile;
    std::string               _calibrationsDir;
    std::string               _outputDir;
};

#endif //MAP_CREATOR_H
