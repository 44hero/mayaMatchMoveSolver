# Copyright (C) 2019 Anil Reddy, David Cattermole.
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
Functions to support smoothing animation curve keyframes.
"""

import maya.cmds

import mmSolver.logger
import mmSolver.utils.smooth as utils_smooth

LOG = mmSolver.logger.get_logger()


def smooth_animcurve(animcurve, selected_keyframes,
                     smooth_type, width,
                     blend_smooth_type, blend_width):
    """
    Smooth the given keyframes for an animCurve.
    """
    times = maya.cmds.keyframe(animcurve, query=True, timeChange=True)
    if len(times) < 1:
        return
    first_time = int(times[0])
    last_time = int(times[-1])
    first_time_padded = int(first_time - (width * 2))
    last_time_padded = int(last_time + (width * 2))

    all_times = range(first_time_padded, last_time_padded + 1)
    weight_array = [0.0] * len(all_times)
    value_array = [None] * len(all_times)
    plug = animcurve + '.output'
    for i, t in enumerate(all_times):
        value = maya.cmds.getAttr(plug, time=t)
        value_array[i] = value
        if t < first_time:
            weight_array[i] = 0.0
            continue
        if t > last_time:
            weight_array[i] = 0.0
            continue
        if t in selected_keyframes:
            weight_array[i] = 1.0

    new_weight_array = utils_smooth.smooth(
        blend_smooth_type,
        weight_array,
        blend_width
    )
    new_value_array = utils_smooth.smooth(
        smooth_type,
        value_array,
        width
    )
    assert len(value_array) == len(new_value_array)
    assert len(weight_array) == len(new_weight_array)

    for frame, old_value, new_value, weight in zip(all_times,
                                                   value_array,
                                                   new_value_array,
                                                   new_weight_array):
        if weight == 0.0:
            continue
        inv_weight = 1.0 - weight
        v = (new_value * weight) + (old_value * inv_weight)
        time_range = (frame, frame)
        maya.cmds.keyframe(
            animcurve,
            valueChange=v,
            time=time_range)
    return
