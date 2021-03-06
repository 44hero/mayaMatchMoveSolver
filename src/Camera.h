/*
 * Copyright (C) 2018, 2019 David Cattermole.
 *
 * This file is part of mmSolver.
 *
 * mmSolver is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * mmSolver is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with mmSolver.  If not, see <https://www.gnu.org/licenses/>.
 * ====================================================================
 *
 * Camera class represents a viewable camera with a projection matrix.
 */

#ifndef MAYA_MM_SOLVER_CAMERA_H
#define MAYA_MM_SOLVER_CAMERA_H

#include <maya/MStatus.h>
#include <maya/MObject.h>
#include <maya/MMatrix.h>
#include <maya/MPoint.h>
#include <maya/MVector.h>
#include <maya/MString.h>
#include <maya/MPlug.h>

#include <cmath>
#include <vector>
#include <unordered_map>  // unordered_map
#include <memory>

#include <utilities/numberUtils.h>

#include <Attr.h>

typedef std::pair<double, MMatrix> DoubleMatrixPair;
typedef std::unordered_map<double, MMatrix> DoubleMatrixMap;
typedef DoubleMatrixMap::const_iterator DoubleMatrixMapCIt;
typedef DoubleMatrixMap::iterator DoubleMatrixMapIt;


MStatus getAngleOfView(
        const double filmBackSize,
        const double focalLength,
        double &angleOfView,
        bool asDegrees = true);


MStatus getCameraPlaneScale(
        const double filmBackSize,
        const double focalLength,
        double &scale);


MStatus computeFrustumCoordinates(
        const double focalLength,     // millimetres
        const double filmBackWidth,   // inches
        const double filmBackHeight,  // inches
        const double filmOffsetX,     // inches
        const double filmOffsetY,     // inches
        const double nearClipPlane,   // centimetres
        const double cameraScale,
        double &left, double &right,
        double &top, double &bottom);


MStatus applyFilmFitLogic(
        const double frustumLeft, const double frustumRight,
        const double frustumTop, const double frustumBottom,
        const double imageAspectRatio, const double filmAspectRatio,
        const int filmFit,  // 0=fill, 1=horizontal, 2=vertical, 3=overscan
        double &filmFitScaleX, double &filmFitScaleY,
        double &screenSizeX, double &screenSizeY,
        double &screenRight, double &screenLeft,
        double &screenTop, double &screenBottom);


MStatus computeProjectionMatrix(
        const double filmFitScaleX,
        const double filmFitScaleY,
        const double screenSizeX,
        const double screenSizeY,
        const double screenLeft,
        const double screenRight,
        const double screenTop,
        const double screenBottom,
        const double nearClipPlane, // centimetres
        const double farClipPlane,  // centimetres
        MMatrix &projectionMatrix);


MStatus getProjectionMatrix(
        const double focalLength,     // millimetres
        const double filmBackWidth,   // inches
        const double filmBackHeight,  // inches
        const double filmOffsetX,     // inches
        const double filmOffsetY,     // inches
        const double imageWidth,      // pixels
        const double imageHeight,     // pixels
        const int filmFit,  // 0=fill, 1=horizontal, 2=vertical, 3=overscan
        const double nearClipPlane,
        const double farClipPlane,
        const double cameraScale,
        MMatrix &projectionMatrix);


class Camera {
public:
    Camera();

    MString getTransformNodeName();

    void setTransformNodeName(MString value);

    MObject getTransformObject();

    MString getShapeNodeName();

    void setShapeNodeName(MString value);

    MObject getShapeObject();

    // TODO: Use 'Projection Dynamic' to tell the solver that
    // the projection matrix of the camera will change over time.
    // For example, we can tell if the projection matrix is dynamic
    // over time if any of the necessary input variables vary over
    // time. This flag could help speed up solving.
    bool getProjectionDynamic() const;

    MStatus setProjectionDynamic(bool value);

    Attr &getMatrixAttr();

    Attr &getFilmbackWidthAttr();

    Attr &getFilmbackHeightAttr();

    Attr &getFilmbackOffsetXAttr();

    Attr &getFilmbackOffsetYAttr();

    Attr &getFocalLengthAttr();

    Attr &getCameraScaleAttr();

    Attr &getNearClipPlaneAttr();

    Attr &getFarClipPlaneAttr();

    Attr &getFilmFitAttr();

    Attr &getRenderWidthAttr();

    Attr &getRenderHeightAttr();

    Attr &getRenderAspectAttr();

    double getFilmbackWidthValue(const MTime &time);

    double getFilmbackHeightValue(const MTime &time);

    double getFilmbackOffsetXValue(const MTime &time);

    double getFilmbackOffsetYValue(const MTime &time);

    double getFocalLengthValue(const MTime &time);

    double getCameraScaleValue();

    double getNearClipPlaneValue();

    double getFarClipPlaneValue();

    int getFilmFitValue();

    int getRenderWidthValue();

    int getRenderHeightValue();

    double getRenderAspectValue();

    MStatus getFrustum(
            double &left, double &right,
            double &top, double &bottom,
            const MTime &time);

    MStatus getProjMatrix(MMatrix &value, const MTime &time);

    MStatus getProjMatrix(MMatrix &value);

    MStatus getWorldPosition(MPoint &value, const MTime &time);

    MStatus getWorldPosition(MPoint &value);

    MStatus getForwardDirection(MVector &value, const MTime &time);

    MStatus getForwardDirection(MVector &value);

    MStatus getWorldProjMatrix(MMatrix &value, const MTime &time);

    MStatus getWorldProjMatrix(MMatrix &value);

    MStatus clearAuxilaryAttrsCache();

    MStatus clearProjMatrixCache();

    MStatus clearWorldProjMatrixCache();

    MStatus clearAttrValueCache();

private:
    MString m_transformNodeName;
    MObject m_transformObject;

    MString m_shapeNodeName;
    MObject m_shapeObject;

    // TODO: Use this variable in the solver.
    bool m_isProjectionDynamic;

    Attr m_matrix;
    Attr m_filmbackWidth;
    Attr m_filmbackHeight;
    Attr m_filmbackOffsetX;
    Attr m_filmbackOffsetY;
    Attr m_focalLength;
    Attr m_cameraScale;
    Attr m_nearClipPlane;
    Attr m_farClipPlane;
    Attr m_filmFit;
    Attr m_renderWidth;
    Attr m_renderHeight;
    Attr m_renderAspect;

    bool m_cameraScaleCached;
    bool m_nearClipPlaneCached;
    bool m_farClipPlaneCached;
    bool m_filmFitCached;
    bool m_renderWidthCached;
    bool m_renderHeightCached;
    bool m_renderAspectCached;

    double m_cameraScaleValue;
    double m_nearClipPlaneValue;
    double m_farClipPlaneValue;
    int    m_filmFitValue;
    int    m_renderWidthValue;
    int    m_renderHeightValue;
    double m_renderAspectValue;

    DoubleMatrixMap m_projMatrixCache;
    DoubleMatrixMap m_worldProjMatrixCache;
};

typedef std::vector<Camera> CameraList;
typedef CameraList::iterator CameraListIt;

typedef std::shared_ptr<Camera> CameraPtr;
typedef std::vector<std::shared_ptr<Camera> > CameraPtrList;
typedef CameraPtrList::iterator CameraPtrListIt;

#endif //MAYA_MM_SOLVER_CAMERA_H
