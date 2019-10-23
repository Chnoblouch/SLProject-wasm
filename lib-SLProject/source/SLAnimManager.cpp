//#############################################################################
//  File:      SLAnimManager.cpp
//  Author:    Marc Wacker
//  Date:      Autumn 2014
//  Codestyle: https://github.com/cpvrlab/SLProject/wiki/SLProject-Coding-Style
//  Copyright: Marcus Hudritsch
//             This software is provide under the GNU General Public License
//             Please visit: http://opensource.org/licenses/GPL-3.0
//#############################################################################

#include <stdafx.h> // Must be the 1st include followed by  an empty line

#ifdef SL_MEMLEAKDETECT    // set in SL.h for debug config only
#    include <debug_new.h> // memory leak detector
#endif

#include <SLScene.h>

//-----------------------------------------------------------------------------
//! destructor
SLAnimManager::~SLAnimManager()
{
    clear();
}

//-----------------------------------------------------------------------------
//! Clears and deletes all node animations and skeletons
void SLAnimManager::clear()
{
    for (auto it : _nodeAnimations)
        delete it.second;
    _nodeAnimations.clear();

    for (auto it : _nodeAnimPlaybacks)
        delete it.second;
    _nodeAnimPlaybacks.clear();

    for (auto skeleton : _skeletons)
        delete skeleton;
    _skeletons.clear();

    _allAnimNames.clear();
    _allAnimPlaybacks.clear();
}
//-----------------------------------------------------------------------------
//! Add a skeleton to the skeleton vector
void SLAnimManager::addSkeleton(SLSkeleton* skel)
{
    _skeletons.push_back(skel);
}
//-----------------------------------------------------------------------------
/*! Creates a new node animation
    @param  duration    length of the animation
*/
SLAnimation* SLAnimManager::createNodeAnimation(SLfloat duration)
{
    SLuint        index = (SLuint)_nodeAnimations.size();
    ostringstream oss;

    do
    {
        oss.clear();
        oss << "Node_" << index;
        index++;
    } while (_nodeAnimations.find(oss.str()) != _nodeAnimations.end());

    return createNodeAnimation(oss.str(), duration);
}
//-----------------------------------------------------------------------------
/*! Creates a new node animation
    @param  name        the animation name
    @param  duration    length of the animation
*/
SLAnimation* SLAnimManager::createNodeAnimation(const SLstring& name,
                                                SLfloat         duration)
{
    assert(_nodeAnimations.find(name) == _nodeAnimations.end() &&
           "node animation with same name already exists!");

    SLAnimation* anim     = new SLAnimation(name, duration);
    _nodeAnimations[name] = anim;

    SLAnimPlayback* playback = new SLAnimPlayback(anim);
    _nodeAnimPlaybacks[name] = playback;

    // Add node animation to the combined vector
    _allAnimNames.push_back(name);
    _allAnimPlaybacks.push_back(playback);

    return anim;
}
//-----------------------------------------------------------------------------
//! Returns the playback of a node animation by name if it exists.
SLAnimPlayback* SLAnimManager::nodeAnimPlayback(const SLstring& name)
{
    if (_nodeAnimPlaybacks.find(name) != _nodeAnimPlaybacks.end())
        return _nodeAnimPlaybacks[name];

    SL_WARN_MSG("*** Playback found in SLAnimManager::getNodeAnimPlayack ***");
    return nullptr;
}
//-----------------------------------------------------------------------------
//! Advances the time of all enabled animation plays.
SLbool SLAnimManager::update(SLfloat elapsedTimeSec)
{
    SLbool updated = false;

    // advance time for node animations and apply them
    // @todo currently we can't blend between normal node animations because we
    // reset them per animation playback. so the last playback that affects a
    // node will have its animation applied.
    // We need to save the playback differently if we want to blend them.

    for (auto it : _nodeAnimPlaybacks)
    {
        SLAnimPlayback* playback = it.second;
        if (playback->enabled())
        {
            playback->parentAnimation()->resetNodes();
            playback->advanceTime(elapsedTimeSec);
            playback->parentAnimation()->apply(playback->localTime(),
                                               playback->weight());
            updated = true;
        }
    }

    // update the skeletons separately
    for (auto skeleton : _skeletons)
        updated |= skeleton->updateAnimations(elapsedTimeSec);

    return updated;
}
//-----------------------------------------------------------------------------
//! Draws the animation visualizations.
void SLAnimManager::drawVisuals(SLSceneView* sv)
{
    for (auto it : _nodeAnimPlaybacks)
    {
        SLAnimPlayback* playback = it.second;
        playback->parentAnimation()->drawNodeVisuals(sv);
    }

    // skeletons are drawn from within SLSceneView per node
}
//-----------------------------------------------------------------------------
