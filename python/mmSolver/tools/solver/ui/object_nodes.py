# Copyright (C) 2018, 2019 David Cattermole.
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
Object nodes for the mmSolver Window UI.
"""

import mmSolver.ui.uimodels as uimodels
import mmSolver.ui.nodes as nodes


class ObjectNode(nodes.Node):
    def __init__(self, name,
                 parent=None,
                 data=None,
                 icon=None,
                 enabled=True,
                 editable=False,
                 selectable=True,
                 checkable=False,
                 neverHasChildren=False):
        if icon is None:
            icon = ':/mmSolver_object.png'
        super(ObjectNode, self).__init__(
            name,
            data=data,
            parent=parent,
            icon=icon,
            enabled=enabled,
            selectable=selectable,
            editable=editable,
            checkable=checkable,
            neverHasChildren=neverHasChildren)
        self.typeInfo = 'object'


class MarkerNode(ObjectNode):
    def __init__(self, name,
                 data=None,
                 parent=None):
        icon = ':/mmSolver_marker.png'
        super(MarkerNode, self).__init__(
            name,
            data=data,
            parent=parent,
            icon=icon,
            selectable=True,
            editable=False)
        self.typeInfo = 'marker'


class CameraNode(ObjectNode):
    def __init__(self, name,
                 data=None,
                 parent=None):
        icon = ':/mmSolver_camera.png'
        super(CameraNode, self).__init__(
            name,
            data=data,
            parent=parent,
            icon=icon,
            selectable=True,
            editable=False)
        self.typeInfo = 'camera'


class BundleNode(ObjectNode):
    def __init__(self, name,
                 data=None,
                 parent=None):
        icon = ':/mmSolver_bundle.png'
        super(BundleNode, self).__init__(
            name,
            data=data,
            parent=parent,
            icon=icon,
            selectable=True,
            editable=False)
        self.typeInfo = 'bundle'


class ObjectModel(uimodels.ItemModel):
    def __init__(self, root, font=None):
        super(ObjectModel, self).__init__(root, font=font)
        self._column_names = {
            0: 'Node',
        }
        self._node_attr_key = {
            'Node': 'name',
        }

    def defaultNodeType(self):
        return MarkerNode

    def columnNames(self):
        column_names = {
            0: 'Node',
        }
        return column_names

    def getGetAttrFuncFromIndex(self, index):
        get_attr_dict = {
            'Node': 'name',
        }
        return self._getGetAttrFuncFromIndex(index, get_attr_dict)

    def getSetAttrFuncFromIndex(self, index):
        set_attr_dict = {
            'Node': 'setName',
        }
        return self._getGetAttrFuncFromIndex(index, set_attr_dict)
