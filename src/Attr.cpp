/*
 * Attribute class holds functions for getting and setting an attribute, as well as state information.
 */

#include <maya/MGlobal.h>
#include <maya/MStatus.h>
#include <maya/MObject.h>
#include <maya/MObjectArray.h>
#include <maya/MPlug.h>
#include <maya/MPlugArray.h>
#include <maya/MString.h>
#include <maya/MStringArray.h>
#include <maya/MMatrix.h>
#include <maya/MFnMatrixData.h>
#include <maya/MAnimCurveChange.h>
#include <maya/MAnimUtil.h>
#include <maya/MFnAnimCurve.h>

#include <mayaUtils.h>
#include <Attr.h>

Attr::Attr() :
        m_nodeName(""),
        m_attrName(""),
        m_object(),
        m_plug(),
        m_dynamic(false),
        m_animated(-1),
        m_isFreeToChange(-1){
}

MString Attr::getName() const {
    return m_nodeName + "." + m_attrName;
}

MStatus Attr::setName(MString value) {
    MStatus status;
    MStringArray values;
    status = value.split('.', values);
    if (status != MStatus::kSuccess) {
        ERR("Attr::setName: Error splitting name. " << value);
        return MS::kFailure;
    }
    if (values.length() == 2) {
        Attr::setNodeName(values[0]);
        Attr::setAttrName(values[1]);
    } else {
        ERR("Attr::setName: Value given has more than one dot character. " << value);
        return MS::kFailure;
    }
    return status;
}

MString Attr::getNodeName() const {
    return m_nodeName;
}

MStatus Attr::setNodeName(MString value) {
    if (value != m_nodeName) {
        m_object = MObject();
        m_plug = MPlug();
        m_animated = -1;
        m_isFreeToChange = -1;
    }
    m_nodeName = value;
    return MS::kSuccess;
}

MString Attr::getAttrName() const {
    return m_attrName;
}

MStatus Attr::setAttrName(MString value) {
    if (value != m_attrName) {
        m_object = MObject();
        m_plug = MPlug();
        m_animated = -1;
        m_isFreeToChange = -1;
    }
    m_attrName = value;
    return MS::kSuccess;
}

MObject Attr::getObject() {
    MString name = Attr::getNodeName();
    getAsObject(name, m_object);
    return m_object;
}

MPlug Attr::getPlug() {
    MStatus status;
    if (m_plug.isNull() == true) {
        MObject nodeObj = Attr::getObject();
        MString attrName = Attr::getAttrName();
        MFnDependencyNode dependsNode(nodeObj, &status);
        MPlug plug = dependsNode.findPlug(attrName, true, &status);
        CHECK_MSTATUS(status);
        if (status != MStatus::kSuccess) {
            MString name = Attr::getName();
            ERR("Attribute cannot be found; " << name);
            return m_plug;
        }

        // For attributes like 'worldMatrix', where we need the first element of the array,
        // not the attribute itself.
        if (plug.isArray()){
            // NOTE: Here we need 'evaluateNumElements' in case Maya hasn't already computed
            // how many elements the array plug is expected to have.
            unsigned int num = plug.evaluateNumElements(&status);
            CHECK_MSTATUS(status);
            num = plug.numElements(&status);
            CHECK_MSTATUS(status);
            if (num > 0) {
                plug = plug.elementByPhysicalIndex(0, &status);
                if (status != MStatus::kSuccess) {
                    ERR("Could not get first plug element.");
                    return m_plug;
                }
            }
        }
        m_plug = plug;
    }
    return m_plug;
}

//MStatus Attr::getPlug(MPlug &plug) {
//    MStatus status;
//    if (m_plug.isNull()){
//        MObject nodeObj = Attr::getObject();
//        MString attrName = Attr::getAttrName();
//        MFnDependencyNode dependsNode(nodeObj, &status);
//        MPlug tmpPlug = dependsNode.findPlug(attrName, true, &status);
//        if (status != MStatus::kSuccess) {
//            MString name = Attr::getName();
//            ERR("Attribute cannot be found; " << name);
//            plug = m_plug;
//            return status;
//        }
//
//        // For attributes like 'worldMatrix', where we need the first element of the array,
//        // not the attribute itself.
//        if (plug.isArray()){
//            // NOTE: Here we need 'evaluateNumElements' in case Maya hasn't already computed
//            // how many elements the array plug is expected to have.
//            unsigned int num = plug.evaluateNumElements(&status);
//            CHECK_MSTATUS(status);
//            num = plug.numElements(&status);
//            CHECK_MSTATUS(status);
//            if (num > 0) {
//                plug = plug.elementByPhysicalIndex(0, &status);
//                if (status != MStatus::kSuccess) {
//                    ERR("Could not get first plug element.");
//                    plug = m_plug;
//                    return status;
//                }
//            }
//        }
//
//        m_plug = tmpPlug;
//        plug = m_plug;
//    }
//    return status;
//}


MObject Attr::getAttribute() {
    MPlug plug = Attr::getPlug();
    MObject attrObj;
    if (!plug.isNull()) {
        attrObj = plug.attribute();
    }
    return attrObj;
}

/*
 * Find out if a plug can be changed or not.
 *
 * Attributes that cannot change could be:
 * - Locked
 * - Already have an incoming connection
 */
bool Attr::isFreeToChange() {
    if (m_isFreeToChange < 0) {
        MPlug plug = Attr::getPlug();
        m_isFreeToChange = (int) (plug.isFreeToChange() == MPlug::kFreeToChange);
    }
    return (bool) m_isFreeToChange;
}

/*
 * Determine if a plug is being animated using an animCurve.
 */
bool Attr::isAnimated() {
    MStatus status;
    if (m_animated < 0) {
        MPlug plug = Attr::getPlug();

        bool animated = false;
        bool isDest = plug.isDestination(&status);
        CHECK_MSTATUS(status);
        if (isDest){
            MPlugArray connPlugs;
            bool asDest = true;  // get the source plugs on the other end of 'plug'.
            bool asSrc = false;
            plug.connectedTo(connPlugs, asDest, asSrc, &status);
            CHECK_MSTATUS(status);
            for (unsigned int i=0; i < connPlugs.length(); ++i){
                MPlug connPlug = connPlugs[i];
                MObject connObj = connPlug.node(&status);
                CHECK_MSTATUS(status);
                if (connObj.hasFn(MFn::Type::kAnimCurve)){
                    animated = true;
                    break;
                }
            }
        }

        if (status != MS::kSuccess) {
            MString name = Attr::getName();
            ERR("Attr::isAnimated failed; " << name);
            animated = false;  // lets assume that if it failed, the plug cannot be animated.
        }
        m_animated = (int) animated;
    }
    return (bool) m_animated;
}

bool Attr::getDynamic() const {
    return m_dynamic;
}

void Attr::setDynamic(bool value) {
    m_dynamic = value;
}

MStatus Attr::getValue(bool &value, const MTime &time) {
    MStatus status;
    bool dyn = Attr::getDynamic();
    bool animated = Attr::isAnimated();
    MPlug plug = Attr::getPlug();

    if (animated) {
        // Animated Attribute
        MFnAnimCurve curveFn(plug, &status);
        CHECK_MSTATUS_AND_RETURN_IT(status);
        double curveValue;
        status = curveFn.evaluate(time, curveValue);
        CHECK_MSTATUS_AND_RETURN_IT(status);
        value = (bool) trunc(curveValue);
    }  else if (dyn)  {
        // Dynamic Attribute
        MDGContext ctx(time);
        value = plug.asBool(ctx, &status);
//        MAnimControl::setCurrentTime(time);
//        value = plug.asBool(MDGContext::fsNormal, &status);
        CHECK_MSTATUS_AND_RETURN_IT(status);
        // INFO(Attr::getName() << " dynamic " << value);
    } else {
        // Static Attribute
        value = plug.asBool();
    }
    return MS::kSuccess;
}


MStatus Attr::getValue(double &value, const MTime &time) {
    MStatus status;
    bool dyn = Attr::getDynamic();
    bool animated = Attr::isAnimated();
    MPlug plug = Attr::getPlug();

    if (animated) {
        MFnAnimCurve curveFn(plug);
        curveFn.evaluate(time, value);
    } else if (dyn) {
        MDGContext ctx(time);
        value = plug.asDouble(ctx, &status);

//        MAnimControl::setCurrentTime(time);
//        value = plug.asDouble(MDGContext::fsNormal, &status);
    } else {
        value = plug.asDouble();
    }
    return MS::kSuccess;
}

MStatus Attr::getValue(MMatrix &value, const MTime &time) {
    MStatus status;
    MPlug plug = Attr::getPlug();

//    MAnimControl::setCurrentTime(time);
//    MObject matrixObj = plug.asMObject(MDGContext::fsNormal, &status);

    // Do we change the behaviour for a dynamic attribute?
    MDGContext ctx(time);
    MObject matrixObj = plug.asMObject(ctx, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    MFnMatrixData matrixData(matrixObj, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    value = matrixData.matrix(&status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    return status;
}

MStatus Attr::getValue(bool &value) {
    MTime time = MAnimControl::currentTime();
    return Attr::getValue(value, time);
}

MStatus Attr::getValue(double &value) {
    MTime time = MAnimControl::currentTime();
    return Attr::getValue(value, time);
}

MStatus Attr::getValue(MMatrix &value) {
    MTime time = MAnimControl::currentTime();
    return Attr::getValue(value, time);
}

MStatus Attr::setValue(double value, const MTime &time,
                       MDGModifier &dgmod, MAnimCurveChange &animChange) {
    MStatus status;
    bool dyn = Attr::getDynamic();
    bool animated = Attr::isAnimated();
    MPlug plug = Attr::getPlug();

    if (animated) {
        MFnAnimCurve curveFn(plug);
        unsigned int keyIndex;
        bool found = curveFn.find(time, keyIndex);
        if (found) {
            curveFn.setValue(keyIndex, value, &animChange);
        } else {
            curveFn.addKeyframe(time, value, &animChange);
        }
    } else if (dyn) {
        // TODO: What do we do??? Just error?
        MString name = Attr::getName();
        MString plugName = plug.name(&status);
        ERR("Dynamic attributes that aren't animated cannot be set; "
                    << "name=" << name << " "
                    << "plug=" << plugName);
        CHECK_MSTATUS_AND_RETURN_IT(status);
    } else {
        dgmod.newPlugValueDouble(plug, value);
        // plug.setValue(value);
        // dgmod.doIt(); // TODO: Should we 'doit' here? Or should we make the caller 'doit'?
    }
    status = MS::kSuccess;
    return status;
}

MStatus Attr::setValue(double value, MDGModifier &dgmod, MAnimCurveChange &animChange) {
    MTime time = MAnimControl::currentTime();
    return Attr::setValue(value, time, dgmod, animChange);
}