/**
* This file is part of ORB-SLAM2.
*
* Copyright (C) 2014-2016 Raúl Mur-Artal <raulmur at unizar dot es> (University of Zaragoza)
* For more information see <https://github.com/raulmur/ORB_SLAM2>
*
* ORB-SLAM2 is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* ORB-SLAM2 is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with ORB-SLAM2. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef OPTIMIZER_H
#define OPTIMIZER_H

#include <WAIMap.h>
#include <WAIMapPoint.h>
#include <WAIKeyFrame.h>
#include <WAIFrame.h>
#include <WAIHelper.h>
#include <LocalMap.h>

#include <orb_slam/LoopClosing.h>

#include <g2o/core/block_solver.h>
#include <g2o/core/optimization_algorithm_levenberg.h>
#include <g2o/solvers/eigen/linear_solver_eigen.h>
#include <g2o/types/sba/types_six_dof_expmap.h>
#include <g2o/core/robust_kernel_impl.h>
#include <g2o/solvers/dense/linear_solver_dense.h>
#include <g2o/types/sim3/types_seven_dof_expmap.h>
#include <g2o/types/sba/types_six_dof_expmap.h>


namespace ORB_SLAM2
{
struct OptimizerStruct
{
    g2o::SparseOptimizer                    optimizer;
    g2o::BlockSolver_6_3::LinearSolverType* linearSolver;
    g2o::BlockSolver_6_3*                   solver_ptr;
    g2o::OptimizationAlgorithmLevenberg*    solver;

    vector<g2o::EdgeSE3ProjectXYZ*> vpEdgesMono;
    vector<WAIKeyFrame*>            vpEdgeKFMono;
    vector<WAIMapPoint*>            vpMapPointEdgeMono;
    int maxKFid;
    LocalMap lmap;
};
class WAI_API Optimizer
{
public:
    void static BundleAdjustment(const std::vector<WAIKeyFrame*>& vpKF, const std::vector<WAIMapPoint*>& vpMP, int nIterations = 5, bool* pbStopFlag = NULL, const unsigned long nLoopKF = 0, const bool bRobust = true);
    void static GlobalBundleAdjustemnt(WAIMap* pMap, int nIterations = 5, bool* pbStopFlag = NULL, const unsigned long nLoopKF = 0, const bool bRobust = true);

    void static initOptimizerStruct(OptimizerStruct* os, WAIKeyFrame* pKF, WorkingSet &wc);
    void static LocalBundleAdjustment(OptimizerStruct* os, bool* pbStopFlag);
    void static applyBundleAdjustment(OptimizerStruct* os, WAIMap* pMap);

    void static optimizerLocalMap(LocalMap &lmap, WAIKeyFrame* pKF);
    void static LocalBundleAdjustment(WAIKeyFrame* pKF, bool* pbStopFlag, WAIMap* pMap);


    int static PoseOptimization(WAIFrame* pFrame, vector<bool> &vbOutliers);
    int static PoseOptimization(WAIFrame* pFrame);

    // if bFixScale is true, 6DoF optimization (stereo,rgbd), 7DoF otherwise (mono)
    void static OptimizeEssentialGraph(WAIMap* pMap, WAIKeyFrame* pLoopKF, WAIKeyFrame* pCurKF, const LoopClosing::KeyFrameAndPose& NonCorrectedSim3, const LoopClosing::KeyFrameAndPose& CorrectedSim3, const map<WAIKeyFrame*, set<WAIKeyFrame*>>& LoopConnections, const bool& bFixScale);

    // if bFixScale is true, optimize SE3 (stereo,rgbd), Sim3 otherwise (mono)
    static int OptimizeSim3(WAIKeyFrame* pKF1, WAIKeyFrame* pKF2, std::vector<WAIMapPoint*>& vpMatches1, g2o::Sim3& g2oS12, const float th2, const bool bFixScale);
};

} //namespace ORB_SLAM

#endif // OPTIMIZER_H
