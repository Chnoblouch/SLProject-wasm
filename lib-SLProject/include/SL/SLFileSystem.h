//#############################################################################
//  File:      SL/SLFileSystem.h
//  Author:    Marcus Hudritsch
//  Date:      July 2014
//  Codestyle: https://github.com/cpvrlab/SLProject/wiki/Coding-Style-Guidelines
//  Copyright: Marcus Hudritsch
//             This software is provide under the GNU General Public License
//             Please visit: http://opensource.org/licenses/GPL-3.0
//#############################################################################

#ifndef SLFILESYSTEM_H
#define SLFILESYSTEM_H

#include <SL.h>

//-----------------------------------------------------------------------------
//! SLFileSystem provides basic filesystem functions
class SLFileSystem
{
    public:
    //! Returns true if a directory exists.
    static SLbool dirExists(SLstring& path);

    //! Make a directory with given path
    static void makeDir(const string& path);

    //! Remove a directory with given path
    static void removeDir(const string& path);

    //! Returns true if a file exists.
    static SLbool fileExists(const SLstring& pathfilename);

    //! Returns the writable configuration directory
    static SLstring getAppsWritableDir();

    //! Returns the working directory
    static SLstring getCurrentWorkingDir();

    //! Deletes a file on the filesystem
    static SLbool deleteFile(SLstring& pathfilename);

    static std::string getFileName(const std::string& pathFilename);

    static std::vector<std::string> getFileNamesInDir(const std::string dirName);

    static bool contains(const std::string container, const std::string search);

    static void split(const std::string& s, char delimiter, std::vector<std::string>& splits);

    static std::string unifySlashes(const std::string& inputDir);

    //!setters
    static void externalDir(const SLstring& dir);

    //!getters
    static SLstring externalDir() { return _externalDir; }

    private:
    //! Directory to save app data outside of the app
    static SLstring _externalDir;
};
//-----------------------------------------------------------------------------
#endif
