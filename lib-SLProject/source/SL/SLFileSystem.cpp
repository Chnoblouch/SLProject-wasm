//#############################################################################
//  File:      SL/SLFileSystem.cpp
//  Author:    Marcus Hudritsch
//  Date:      July 2014
//  Codestyle: https://github.com/cpvrlab/SLProject/wiki/Coding-Style-Guidelines
//  Copyright: Marcus Hudritsch
//             This software is provide under the GNU General Public License
//             Please visit: http://opensource.org/licenses/GPL-3.0
//#############################################################################

#include <stdafx.h> // Must be the 1st include followed by  an empty line

#ifdef SL_MEMLEAKDETECT    // set in SL.h for debug config only
#    include <debug_new.h> // memory leak detector
#endif

#include <SLFileSystem.h>

#ifdef SL_OS_WINDOWS
#    include <direct.h> //_getcwd
#elif defined(SL_OS_MACOS)
#    include <unistd.h>
#elif defined(SL_OS_MACIOS)
#    include <unistd.h> //getcwd
#elif defined(SL_OS_ANDROID)
#    include <unistd.h> //getcwd
#    include <sys/stat.h>
#elif defined(SL_OS_LINUX)
#    include <unistd.h> //getcwd
#endif

//-----------------------------------------------------------------------------
SLstring SLFileSystem::_externalDir = "";
//-----------------------------------------------------------------------------
/*! Returns true if the directory exists. Be aware that on some OS file and
paths are treated case sensitive.
*/
SLbool SLFileSystem::dirExists(SLstring& path)
{
    struct stat info;
    if (stat(path.c_str(), &info) != 0)
        return false;
    else if (info.st_mode & S_IFDIR)
        return true;
    else
        return false;
}
//-----------------------------------------------------------------------------
/*! Make a directory with given path
*/
void SLFileSystem::makeDir(const string& path)
{
#ifdef SL_OS_WINDOWS
    _mkdir(path.c_str());
#else
    mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#endif
}
//-----------------------------------------------------------------------------
/*! Remove a directory with given path. DOES ONLY WORK FOR EMPTY DIRECTORIES
*/
void SLFileSystem::removeDir(const string& path)
{
#ifdef SL_OS_WINDOWS
    int ret = _rmdir(path.c_str());
    if (ret != 0)
    {
        errno_t err;
        _get_errno(&err);
        SL_LOG("Could not remove directory: %s\nErrno: %s\n", path.c_str(), strerror(errno));
    }
#else
    rmdir(path.c_str());
#endif
}
//-----------------------------------------------------------------------------
/*! Returns true if the file exists.Be aware that on some OS file and
paths are treated case sensitive.
*/
SLbool SLFileSystem::fileExists(const SLstring& pathfilename)
{
    struct stat info;
    if (stat(pathfilename.c_str(), &info) == 0)
    {
        return true;
    }
    return false;
}
//-----------------------------------------------------------------------------
SLstring SLFileSystem::getAppsWritableDir()
{
#ifdef SL_OS_WINDOWS
    SLstring appData   = getenv("APPDATA");
    SLstring configDir = appData + "/SLProject";
    SLUtils::replaceString(configDir, "\\", "/");
    if (!dirExists(configDir))
        _mkdir(configDir.c_str());
    return configDir + "/";
#elif defined(SL_OS_MACOS)
    SLstring home      = getenv("HOME");
    SLstring appData   = home + "/Library/Application Support";
    SLstring configDir = appData + "/SLProject";
    if (!dirExists(configDir))
        mkdir(configDir.c_str(), S_IRWXU);
    return configDir + "/";
#elif defined(SL_OS_ANDROID)
    // @todo Where is the app data path on Andoroid?
#elif defined(SL_OS_LINUX)
    // @todo Where is the app data path on Linux?
    SLstring home      = getenv("HOME");
    SLstring configDir = home + "/.SLProject";
    if (!dirExists(configDir))
        mkdir(configDir.c_str(), S_IRWXU | S_IRWXG | S_IRWXO);
    return configDir + "/";
#else
#    error "SL has not been ported to this OS"
#endif
}
//-----------------------------------------------------------------------------
SLstring SLFileSystem::getCurrentWorkingDir()
{
#ifdef SL_OS_WINDOWS
    SLint size   = 256;
    char* buffer = (char*)malloc(size);
    if (_getcwd(buffer, size) == buffer)
    {
        SLstring dir = buffer;
        SLUtils::replaceString(dir, "\\", "/");
        return dir + "/";
    }

    free(buffer);
    return "";
#else
    size_t size   = 256;
    char*  buffer = (char*)malloc(size);
    if (getcwd(buffer, size) == buffer)
        return SLstring(buffer) + "/";

    free(buffer);
    return "";
#endif
}
//-----------------------------------------------------------------------------
SLbool SLFileSystem::deleteFile(SLstring& pathfilename)
{
    if (SLFileSystem::fileExists(pathfilename))
        return remove(pathfilename.c_str()) != 0;
    return false;
}
//-----------------------------------------------------------------------------
//! Set the path to the external directory
void SLFileSystem::externalDir(const SLstring& dir)
{
    _externalDir = SLUtils::unifySlashes(dir);
}
//-----------------------------------------------------------------------------
std::string SLFileSystem::getFileName(const std::string& pathFilename)
{
    size_t i = 0, i1, i2;
    i1       = pathFilename.rfind('\\', pathFilename.length());
    i2       = pathFilename.rfind('/', pathFilename.length());

    if (i1 != std::string::npos && i2 != std::string::npos)
        i = std::max(i1, i2);
    else if (i1 != std::string::npos)
        i = i1;
    else if (i2 != std::string::npos)
        i = i2;

    return pathFilename.substr(i + 1, pathFilename.length() - i);
}
//-----------------------------------------------------------------------------
std::vector<std::string> SLFileSystem::getFileNamesInDir(const std::string dirName)
{
    std::vector<std::string> fileNames;
    DIR*                     dir;
    dir = opendir(dirName.c_str());

    if (dir)
    {
        struct dirent* dirContent;
        int            i = 0;

        while ((dirContent = readdir(dir)) != nullptr)
        {
            i++;
            std::string name(dirContent->d_name);
            if (name != "." && name != "..")
                fileNames.push_back(dirName + "/" + name);
        }
        closedir(dir);
    }
    return fileNames;
}
//-----------------------------------------------------------------------------
bool SLFileSystem::contains(const std::string container, const std::string search)
{
    return (container.find(search) != std::string::npos);
}
//-----------------------------------------------------------------------------
void SLFileSystem::split(const std::string& s, char delimiter, std::vector<std::string>& splits)
{
    std::string::size_type i = 0;
    std::string::size_type j = s.find(delimiter);

    while (j != std::string::npos)
    {
        splits.push_back(s.substr(i, j - i));
        i = ++j;
        j = s.find(delimiter, j);
        if (j == std::string::npos)
            splits.push_back(s.substr(i, s.length()));
    }
}
//-----------------------------------------------------------------------------
std::string SLFileSystem::unifySlashes(const std::string& inputDir)
{
    std::string copy = inputDir;
    std::string curr;
    std::string delimiter = "\\";
    size_t      pos       = 0;
    std::string token;
    while ((pos = copy.find(delimiter)) != std::string::npos)
    {
        token = copy.substr(0, pos);
        copy.erase(0, pos + delimiter.length());
        curr.append(token);
        curr.append("/");
    }

    curr.append(copy);
    if (curr.size() && curr.back() != '/')
        curr.append("/");

    return curr;
}
