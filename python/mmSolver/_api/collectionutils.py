# Copyright (C) 2019 David Cattermole.
#
# This file is part of mmSolver.
#
# mmSolver is free software: you can redistribute it and/or modify it
# under the terms of the GNU Lesser General Public License as
# published by the Free Software Foundation, either version 3 of the
# License, or (at your option) any later version.
#
# mmSolver is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with mmSolver.  If not, see <https://www.gnu.org/licenses/>.
#
"""
Utilities used with Collection compiled 'kwargs'.
"""

import maya.OpenMaya as OpenMaya
import maya.cmds
import maya.mel

import mmSolver.logger
import mmSolver.utils.node
import mmSolver._api.attribute as attribute

LOG = mmSolver.logger.get_logger()


def is_single_frame(kwargs):
    """
    Logic to determine if the solver arguments will solve a single
    frame or not.
    """
    has_one_frame = len(kwargs.get('frame')) is 1
    is_interactive = maya.cmds.about(query=True, batch=True) is False
    return has_one_frame and is_interactive


def disconnect_animcurves(kwargs):
    """

    HACK: Disconnect animCurves from animated attributes,
    then re-connect afterward. This is to solve a Maya bug,
    which will not solve values on a single frame.

    :param kwargs:
    :return:
    """
    f = kwargs.get('frame')[0]
    maya.cmds.currentTime(f, edit=True, update=False)

    save_node_attrs = []
    attrs = kwargs.get('attr') or []
    for attr_name, min_val, max_val in attrs:
        attr_obj = attribute.Attribute(attr_name)
        if attr_obj.is_animated() is False:
            continue

        in_plug_name = None
        out_plug_name = attr_name
        plug = mmSolver.utils.node.get_as_plug(attr_name)
        isDest = plug.isDestination()
        if isDest:
            connPlugs = OpenMaya.MPlugArray()
            asDest = True  # get the source plugs on the other end of 'plug'.
            asSrc = False
            plug.connectedTo(connPlugs, asDest, asSrc)
            for i, conn in enumerate(connPlugs):
                connPlug = connPlugs[i]
                connObj = connPlug.node()
                if connObj.hasFn(OpenMaya.MFn.kAnimCurve):
                    in_plug_name = connPlug.name()
                    break
        if in_plug_name is not None:
            save_node_attrs.append((in_plug_name, out_plug_name))
            if maya.cmds.isConnected(in_plug_name, out_plug_name) is True:
                maya.cmds.disconnectAttr(in_plug_name, out_plug_name)
            else:
                LOG.error('Nodes are not connected. This is WRONG.')
    return save_node_attrs


def reconnect_animcurves(kwargs, save_node_attrs, force_dg_update=True):
    """

    :param kwargs:
    :param save_node_attrs:
    :param force_dg_update:
    :return:
    """
    f = kwargs.get('frame')[0]
    maya.cmds.currentTime(f, edit=True, update=False)

    # Re-connect animCurves, and set the solved values.
    update_nodes = []
    for in_plug_name, out_plug_name in save_node_attrs:
        if maya.cmds.isConnected(in_plug_name, out_plug_name) is False:
            v = maya.cmds.getAttr(out_plug_name)
            maya.cmds.connectAttr(in_plug_name, out_plug_name)
            attr_obj = attribute.Attribute(name=out_plug_name)
            tangent_type = 'linear'
            node = attr_obj.get_node()
            maya.cmds.setKeyframe(
                node,
                attribute=attr_obj.get_attr(),
                time=f, value=v,
                inTangentType=tangent_type,
                outTangentType=tangent_type,
            )
            update_nodes.append(node)
        else:
            LOG.error('Nodes are connected. This is WRONG.')
            raise RuntimeError

    # force update of Maya.
    if force_dg_update is True:
        maya.cmds.dgdirty(update_nodes)
    return


def clear_attr_keyframes(kwargs, frames):
    """
    Evaluates the animated attributes at 'frames', then deletes the
    existing animCurves.
    """
    frames = list(sorted(frames))
    attrs = kwargs.get('attr') or []
    for attr_name, min_val, max_val in attrs:
        attr_obj = attribute.Attribute(name=attr_name)
        if not attr_obj.is_animated():
            continue

        # Get Animation Curve
        animCurves = maya.cmds.listConnections(
            attr_name,
            type='animCurve'
        ) or []
        if len(animCurves) == 0:
            continue
        animCurve = animCurves[0]

        # Query AnimCurve values that we wish to keep.
        values = []
        for f in frames:
            v = maya.cmds.getAttr(
                animCurve + '.output',
                time=float(f),
            )
            values.append(v)

        # Re-create animCurve.
        maya.cmds.delete(animCurve)
        tangent_type = 'linear'
        for f, v in zip(frames, values):
            maya.cmds.setKeyframe(
                attr_name,
                time=f,
                value=v,
                respectKeyable=False,
                minimizeRotation=False,
                inTangentType=tangent_type,
                outTangentType=tangent_type
            )
    return


def generate_isolate_nodes(kwargs):
    """

    :param kwargs:
    :return:
    """
    nodes = set()
    attrs = kwargs.get('attr') or []
    for attr_name, min_val, max_val in attrs:
        attr_obj = attribute.Attribute(name=attr_name)
        node = attr_obj.get_node()
        nodes.add(node)
    markers = kwargs.get('marker') or []
    for mkr_node, cam_shp_node, bnd_node in markers:
        nodes.add(mkr_node)
        nodes.add(bnd_node)
    cameras = kwargs.get('camera') or []
    for cam_tfm_node, cam_shp_node in cameras:
        nodes.add(cam_tfm_node)
        nodes.add(cam_shp_node)
    return nodes