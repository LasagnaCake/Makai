import os
import sys
import inspect

import struct
import json
import base64

import bpy

from bpy import types as bt
from bpy import props as bp
from bpy import utils as bu

bl_info = {
    "name": "Makai SceneMaker",
    "author": "LasagnaCake",
    "version": (0, 0, 1),
    "blender": (3, 4, 0),
    "location": "3D Viewport > Sidebar > [Scene Object, Scene]",
    "description": "Custom scene builder & exporter for Makai's scene format",
    "category": "Development"
}


"""
    ----------

    Properties

    ----------
"""

def BoolProperty(prop_name, prop_default=False):
    return bp.BoolProperty(
        name=prop_name,
        default=prop_default,
        options=set()
    )

def IntProperty(prop_name, prop_default=0, prop_min=sys.float_info.min, prop_max=sys.float_info.max):
    return bp.IntProperty(
        name=prop_name,
        default=prop_default,
        min=prop_min,
        max=prop_max,
        options=set()
    )

def FloatProperty(prop_name, prop_default=0, prop_min=sys.float_info.min, prop_max=sys.float_info.max):
    return bp.FloatProperty(
        name=prop_name,
        default=prop_default,
        min=prop_min,
        max=prop_max,
        options=set()
    )

def Vector2Property(prop_name, prop_default=(0, 0), prop_subtype="XYZ"):
    return bp.FloatVectorProperty(
        name=prop_name,
        size=2,
        default=prop_default,
        subtype=prop_subtype,
        options=set()
    )

def Vector3Property(prop_name, prop_default=(0, 0, 0), prop_subtype="XYZ"):
    return bp.FloatVectorProperty(
        name=prop_name,
        size=3,
        default=prop_default,
        subtype=prop_subtype,
        options=set()
    )

def ImageProperty(prop_name = "Image"):
    return bp.PointerProperty(
        name=prop_name,
        type=bt.Image,
        options=set()
    )

def CameraProperty(prop_name = "Camera"):
    def is_camera(self, obj):
        return obj.type=="CAMERA"
    return bp.PointerProperty(
        name=prop_name,
        type=bt.Object,
        options=set(),
        poll=is_camera
    )

def RGBAColorProperty(prop_name = "Color", prop_default=(1,1,1,1)):
    return bp.FloatVectorProperty(
        name=prop_name,
        size=4,
        default=prop_default,
        min=0,
        max=1,
        subtype="COLOR",
        options=set()
    )

def RGBColorProperty(prop_name = "Color", prop_default=(1,1,1)):
    return bp.FloatVectorProperty(
        name=prop_name,
        size=3,
        default=prop_default,
        min=0,
        max=1,
        subtype="COLOR",
        options=set()
    )

def RangeProperty(prop_name, prop_default=0):
    return FloatProperty(prop_name, prop_default, 0, 1)

def AlphaClipProperty(prop_name):
    return RangeProperty(prop_name + " Alpha Clip", 0.2)

def EnumProperty(prop_name, prop_values, prop_default=0, prop_update=None, prop_index_offset=0):
    prop_items = [(str(i+prop_index_offset), prop_values[i], "", i+prop_index_offset) for i in range(len(prop_values))]
    return bp.EnumProperty(
        name=prop_name,
        items=prop_items,
        default=prop_default,
        update=prop_update,
        options=set()
    )

def ChannelProperty(prop_name, prop_default=0, composite_channel=False):
    if composite_channel:
        return EnumProperty(
            prop_name,
            ["RGB Average", "Red", "Green", "Blue", "Alpha"],
            prop_default,
            None,
            -1
        )
    return EnumProperty(
        prop_name,
        ["Red", "Green", "Blue", "Alpha"],
        prop_default,
        None,
        0
    )

blend_equations = [
    "Zero",
    "One",
    "Add",
    "Subtract",
    "Reverse Subtract",
    "Min",
    "Max"
]

blend_functions = [
    "Zero",
    "One",
    "SRC color",
    "One minus SRC Color",
    "DST Color",
    "One Minus DST Color",
    "SRC Alpha",
    "One Minus SRC Alpha",
    "DST Alpha",
    "One Minus DST Alpha",
    "Constant Color",
    "One Minus Constant color",
    "Constant Alpha",
    "One Minus Constant alpha",
    "SRC Alpha Saturate"
]

def BlendFunctionProperty(prop_name, prop_default, prop_update=None):
    return EnumProperty(prop_name, blend_functions, prop_default, prop_update)

def BlendEquationProperty(prop_name, prop_default, prop_update=None):
    return EnumProperty(prop_name, blend_equations, prop_default, prop_update)

def DirectoryPathProperty(prop_name, prop_default="", prop_options=set()):
    return bp.StringProperty(
        name=prop_name,
        subtype="DIR_PATH",
        default=prop_default,
        options=prop_options
    )

def FileNameProperty(prop_name, prop_default="", prop_options=set()):
    return bp.StringProperty(
        name=prop_name,
        subtype="FILE_NAME",
        default=prop_default,
        options=prop_options
    )
    
"""
    ----------------

    Helper Functions
    
    ----------------
"""

def make_if_not_exists(dir: str):
    if not os.path.isdir(dir):
        os.makedirs(dir)

def save_image_to_file(image, path):
    if image is None:
        return "ERR_IMAGE_DOES_NOT_EXIST"
    # Check if the image is not packed
    image_packed = False
    if image.packed_file is not None:
        image_packed = True
        try:
            image_texture.unpack()
        except:
            return "ERR_IMAGE_UNPACKING_FAILED"
    # Save the image to a file (adjust the path as needed)
    image_texture.file_format = "PNG"
    image_texture.save_render(image_path, scene=None, quality=75)
    if image_packed:
        image_texture.pack()
    return "OK"


def image_to_base64(path):
    with open(path, "rb") as image_file:
        return str(base64.b64encode(image_file.read()))[2:-1]

def as_hex_string(color):
    def to255(x): 
        return int(max(0, min(x*255, 255)))

    return "#{0:02x}{1:02x}{2:02x}{2:02x}".format(to255(color[0]), to255(color[1]), to255(color[2]), to255(color[3]))

def as_hex_string_rgb(color):
    def to255(x): 
        return int(max(0, min(x*255, 255)))

    return "#{0:02x}{1:02x}{2:02x}".format(to255(color[0]), to255(color[1]), to255(color[2]))


def process_image_file(embed_texture, image, path, temp_path, relative_path = "", enabled=True, alpha_clip=0.2):
    if not embed_texture:
        result = save_texture_to_image(image, path)
        print(f"Texture: {result}")
        if result == "OK":
            file = {
                "enabled": enabled,
                "image": {"path": relative_path},
            }
            if alpha_clip is not None:
                file["alphaClip"] = alpha_clip
            return file
    else:
        result = save_image_to_file(image, temp_path)
        if result == "OK":
            imgstr = str(image_to_base64(temp_path))
            os.remove(temp_path)
            if imgstr is None:
                return None
            file = {
                "enabled": enabled,
                "image": imgstr,
                "encoding": "base64"
            }
            if alpha_clip is not None:
                file["alphaClip"] = alpha_clip
            return file
    return None

def get_binary_data(mesh):
    # iterate through the mesh's loop triangles to collect the vertex data
    vertex_data = []
    component_data = "x,y,z"
    if mesh.uv_layers.active: component_data += ",u,v"
    if mesh.vertex_colors.active: component_data += ",r,g,b,a"
    component_data += ",nx,ny,nz"
    for loop_tri in mesh.loop_triangles:
        for loop_index in loop_tri.loops:
            vertex = mesh.vertices[mesh.loops[loop_index].vertex_index]
            normal = vertex.normal
            uv = mesh.uv_layers.active.data[loop_index].uv if mesh.uv_layers.active else None
            color = mesh.vertex_colors.active.data[loop_index].color if mesh.vertex_colors.active else None
            vertex_pos = vertex.co
            # Copy position data
            vtxdat = [-vertex_pos.x, vertex_pos.z, vertex_pos.y]
            # Get UV & color data, if applicable
            if mesh.uv_layers.active:
                vtxdat.extend([uv[0], 1-uv[1]])
            if mesh.vertex_colors.active:
                vtxdat.extend([color[0], color[1], color[2], color[3]])
            # Append normal data
            vtxdat.extend([-normal.x, normal.z, normal.y])
            # Append to array
            vertex_data.extend(vtxdat)
    # Pack binary
    vertex_binary = struct.pack("<" + "f"*len(vertex_data), *vertex_data)
    return (vertex_binary, component_data)

def create_render_definition(context, obj, file_name, folder_path, tx_folder, mesh_folder, embed_texture, embed_mesh, apply_modifiers, hexcolor):
    def hex_color_or_array(color):
        return as_hex_string(color) if hexcolor else [x for x in color]
    mrodpath = folder_path + "\\" + file_name 
    txpath = mrodpath + "\\" + tx_folder
    meshpath = mrodpath + "\\" + mesh_folder
    if not embed_texture:
        make_if_not_exists(txpath)
    if not embed_mesh:
        make_if_not_exists(meshpath)
    dg = context.evaluated_depsgraph_get()
    mesh = None
    #TODO: fix this
    if apply_modifiers:
        mesh = obj.to_mesh(preserve_all_data_layers=False, depsgraph=dg)
    else:
        mesh = obj.to_mesh(preserve_all_data_layers=True, depsgraph=dg)
    verts = mesh.vertices
    # iterate through the mesh's loop triangles to collect the vertex data
    vertex_binary, component_data = get_binary_data(mesh)
    # Do appropriate mesh procedure
    strfile = {
        "version": 0,
        "mesh": {
            "data": None,
            "components": component_data
        }
    }
    if embed_mesh:
        strfile["mesh"]["data"] = str(base64.b64encode(vertex_binary))[2:-1]
        strfile["mesh"]["encoding"] = "base64"
    else:
        with open(f"{meshpath}\\{obj.name}.mesh", "wb") as f:
            # write the binary data to the file
            f.write(vertex_binary)
        strfile["mesh"]["data"] = {"path": f"{mesh_folder}\\{obj.name}.mesh"}
    # Get transforms
    pos, rot, scale = obj.matrix_world.decompose()
    rot = rot.to_euler('YXZ')
    strfile["trans"] = {
        "position": [pos.x, pos.y, pos.z],
        "rotation": [rot.x, rot.y, rot.z],
        "scale": [scale.x, scale.y, scale.z]
    }
    # Set preliminary material data
    strfile["material"] = {
        "color": hex_color_or_array(obj.material_props.color),
        "shaded": obj.material_props.shaded,
        "illuminated": obj.material_props.illuminated,
        "hue": obj.material_props.h,
        "saturation": obj.material_props.s,
        "luminosity": obj.material_props.l,
        "brightness": obj.material_props.b,
        "contrast": obj.material_props.c,
        "instances": [
            [0.0, 0.0, 0.0],
        ],
        "culling": int(obj.material_props.culling),
        "fill": int(obj.material_props.fill)
    }
    strfile["blend"] = {}
    # set blend stuff
    if obj.blend_props.func_sep:
        strfile["blend"]["function"] = {
            "srcColor": int(obj.blend_props.func_src_rgb), 
            "dstColor": int(obj.blend_props.func_dst_rgb),
            "srcAlpha": int(obj.blend_props.func_src_alpha),
            "dstAlpha": int(obj.blend_props.func_dst_alpha)
        }
    else:
        strfile["blend"]["function"] = {
            "src": int(obj.blend_props.func_src), 
            "dst": int(obj.blend_props.func_dst),
        }
        
    if obj.blend_props.eq_sep:
        strfile["blend"]["equation"] = {
            "color": int(obj.blend_props.eq_rgb),
            "alpha": int(obj.blend_props.eq_alpha)
        }
    else:
        strfile["blend"]["equation"] = int(obj.blend_props.eq)
    # Set active (TODO: Check if object visible in scene)
    strfile["active"] = True
    # Set texture
    if obj.material_props.texture_0_enabled and obj.material_props.texture_1_image is not None:
        strfile["material"]["texture"] = process_image_file(
            embed_texture,
            obj.material_props.texture_1_image,
            f"{txpath}\\texture.png",
            f"{mrodpath}\\_tx_TMP.png",
            f"{tx_folder}\\texture.png",
            obj.material_props.texture_0_enabled,
            obj.material_props.texture_2_alpha_clip
        )
    # Set emission
    if obj.material_props.emission_0_enabled and obj.material_props.emission_1_image is not None:
        strfile["material"]["emission"] = process_image_file(
            embed_texture,
            obj.material_props.emission_1_image,
            f"{txpath}\\emission.png",
            f"{mrodpath}\\_em_TMP.png",
            f"{tx_folder}\\emission.png",
            obj.material_props.emission_0_enabled,
            None
        )
        strfile["material"]["emission"]["strength"] = obj.material_props.emission_2_strength
    # Set warp
    if obj.material_props.warp_0_enabled and obj.material_props.warp_1_image is not None:
        strfile["material"]["warp"] = process_image_file(
            embed_texture,
            obj.material_props.warp_1_image,
            f"{txpath}\\warp.png",
            f"{mrodpath}\\_wp_TMP.png",
            f"{tx_folder}\\warp.png",
            obj.material_props.warp_0_enabled,
            None
        )
        strfile["material"]["warp"]["trans"] = {
            "position": [x for x in obj.material_props.warp_2_position],
            "rotation": obj.material_props.warp_3_rotation,
            "scale": [x for x in obj.material_props.warp_4_scale]
        }
        strfile["material"]["warp"]["channelX"] = int(obj.material_props.warp_5_channelX)
        strfile["material"]["warp"]["channelY"] = int(obj.material_props.warp_5_channelY)
    # Set negative
    if obj.material_props.negative_0_enabled:
        strfile["material"]["negative"] = {
            "enabled": obj.material_props.negative_0_enabled,
            "strenagth": obj.material_props.negative_1_strength
        }
    # Set Gradient
    if obj.material_props.gradient_0_enabled:
        strfile["material"]["gradient"] = {
            "enabled": obj.material_props.gradient_0_enabled,
            "channel": int(obj.material_props.gradient_1_channel),
            "begin": hex_color_or_array(obj.material_props.gradient_2_begin),
            "end": hex_color_or_array(obj.material_props.gradient_3_end),
            "invert": obj.material_props.gradient_4_invert
        }
    return strfile;

"""
    ------------

    Base Classes
    
    ------------
"""

class BaseProperties(bt.PropertyGroup):
    def render_child(self, target, child):
        layout = target.layout
        props = [x for x,_ in inspect.getmembers(self)]
        if f"{child}0_enabled" in props:
            layout.prop(self, f"{child}0_enabled")
            if self.get(f"{child}0_enabled", False):
                for name in props:
                    if child in name and name != f"{child}0_enabled":
                        layout.prop(self, name)
        else:
            for name, _ in inspect.getmembers(self):
                if child in name:
                    layout.prop(self, name)
        layout.column()

class PropertyPanel(bt.Panel):
    bl_space_type = "VIEW_3D"   
    bl_region_type = "UI"
    bl_context = "objectmode" 
    bl_options = {"DEFAULT_CLOSED", "HEADER_LAYOUT_EXPAND"}

    @classmethod
    def poll(self, context):
        return context.object is not None
 
"""
    ---------------

    Property Groups
    
    ---------------

"""
    
class ObjectMaterialProperties(BaseProperties):
    color: RGBAColorProperty("Color")
    shaded: BoolProperty("Shaded")
    illuminated: BoolProperty("Illuminated")
    h: FloatProperty("Hue", 0, (-3.1415) * 2, (3.1415) * 2)
    s: FloatProperty("Saturation", 1, 0, 2)
    l: FloatProperty("Luminosity", 1, 0, 2)
    b: FloatProperty("Brightness", 0, -1, 1)
    c: FloatProperty("Contrast", 1, 0, 2)
    
    # Please avert your eyes in the following lines, I needed getmembers() to be ordered in this way
    # Why? because lazy (render_child function)
    
    texture_0_enabled: BoolProperty("Enable Texture")
    texture_1_image: ImageProperty("Texture Image")
    texture_2_alpha_clip: AlphaClipProperty("Texture")
    
    emission_0_enabled: BoolProperty("Enable Emission")
    emission_1_image: ImageProperty("Emission Image")
    emission_3_strength: RangeProperty("Emission Strength")
    
    warp_0_enabled: BoolProperty("Enable Texture Warp")
    warp_1_image: ImageProperty("Warp Image")
    warp_2_position: Vector2Property("Warp Position")
    warp_3_rotation: FloatProperty("Warp Rotation")
    warp_4_scale: Vector2Property("Warp Scale", (0, 0))
    warp_5_channelX: ChannelProperty("Warp Channel for X", 0)
    warp_6_channelY: ChannelProperty("Warp Channel for Y", 1)
    
    negative_0_enabled: BoolProperty("Enable Negative")
    negative_1_strength: RangeProperty("Negative Strength")
    
    gradient_0_enabled: BoolProperty("Enable Gradient")
    gradient_1_channel: ChannelProperty("Gradient Channel", -1, True)
    gradient_2_begin: RGBAColorProperty("Gradient Begin", (0,0,0,1))
    gradient_3_end: RGBAColorProperty("Gradient End", (1,1,1,1))
    gradient_4_invert: BoolProperty("Gradient Invert")
    
    # Ugliness over
    # Hopefully that won't happen again (clueless)
    
    culling: EnumProperty("Culling", ["Front and Back", "Front", "Back"])
    
    fill: EnumProperty("Fill", ["Fill", "Line", "Point"])              
    
    def render(self, target):
        layout = target.layout
        layout.label(text="Albedo")
        layout.prop(self, "color")
        layout.label(text="Shading")
        layout.prop(self, "shaded")
        layout.prop(self, "illuminated")
        layout.label(text="HSL Adjustments")
        layout.prop(self, "h")
        layout.prop(self, "s")
        layout.prop(self, "l")
        layout.label(text="Brightness & Contrast")
        layout.prop(self, "b")
        layout.prop(self, "c")
        layout.column()
        layout.label(text="Texture Effect")
        self.render_child(target, "texture_")
        layout.label(text="Emission Effect")
        self.render_child(target, "emission_")
        layout.label(text="Texture Warping Effect")
        self.render_child(target, "warp_")
        layout.label(text="Negative Effect")
        self.render_child(target, "negative_")
        layout.label(text="Gradient Effect")
        self.render_child(target, "gradient_")
        layout.label(text="Culling & Fill")
        layout.prop(self, "culling")
        layout.prop(self, "fill")

class ObjectBlendProperties(BaseProperties):  
    def update_src_func(self, context):
        self.func_src_alpha = self.func_src
        self.func_src_rgb = self.func_src
        
    def update_dst_func(self, context):
        self.func_dst_alpha = self.func_dst
        self.func_dst_rgb = self.func_dst
        
    def update_eq(self, context):
        self.eq_alpha = self.eq
        self.eq_rgb = self.eq
    
    func_sep: BoolProperty("Separate Blend Functions")
    eq_sep: BoolProperty("Separate Blend Equations")
    
    func_src: BlendFunctionProperty("SRC Function", 6, update_src_func)
    func_src_rgb: BlendFunctionProperty("SRC Color Function", 6)
    func_src_alpha: BlendFunctionProperty("SRC Alpha Function", 7)
    
    func_dst: BlendFunctionProperty("DST Function", 7, update_dst_func)
    func_dst_rgb: BlendFunctionProperty("DST Color Function", 0)
    func_dst_alpha: BlendFunctionProperty("DST Alpha Function", 1)
    
    eq: BlendEquationProperty("Equation", 2, update_eq)
    eq_rgb: BlendEquationProperty("Color Equation", 2)
    eq_alpha: BlendEquationProperty("Alpha Equation", 2)
    
    def render(self, target):
        layout = target.layout
        layout.label(text="Blend Function")
        layout.prop(self, "func_sep")
        if self.func_sep:
            layout.prop(self, "func_src_rgb")
            layout.prop(self, "func_src_alpha")
            layout.prop(self, "func_dst_rgb")
            layout.prop(self, "func_dst_alpha")
        else:
            layout.prop(self, "func_src")
            layout.prop(self, "func_dst")
        layout.column()
        layout.label(text="Blend Equation")
        layout.prop(self, "eq_sep")
        if self.eq_sep:
            layout.prop(self, "eq_rgb")
            layout.prop(self, "eq_alpha")
        else:
            layout.prop(self, "eq")

class ObjectExportProperties(BaseProperties):
    dir_path: DirectoryPathProperty("Output Folder", "", set(["OUTPUT_PATH"])) 
    file_name: FileNameProperty("Object File Name")

    apply_modifiers: BoolProperty("Apply Modifiers", True)
    embed_meshes: BoolProperty("Embed Meshes", True)
    embed_textures: BoolProperty("Embed Images", True)
    no_hex_color: BoolProperty("Don't use Hex Color", False)
    
    tx_dir_path: DirectoryPathProperty("Mesh Folder", "tx", set(["OUTPUT_PATH"])) 
    mesh_dir_path: DirectoryPathProperty("Images Folder", "mesh", set(["OUTPUT_PATH"]))

    def render(self, target):
        layout = target.layout
        layout.label(text="Path")
        layout.prop(self, "dir_path")
        layout.prop(self, "file_name")
        layout.label(text="Settings")
        layout.prop(self, "apply_modifiers")
        layout.prop(self, "embed_meshes")
        if not self.embed_meshes:
            layout.prop(self, "mesh_dir_path")
        layout.prop(self, "embed_textures")
        if not self.embed_textures:
            layout.prop(self, "tx_dir_path")
        layout.prop(self, "no_hex_color")
        layout.column()
        pass

class SceneProperties(BaseProperties):
    camera: CameraProperty("Scene Camera")
    
    # It returns...
    
    near_fog_0_enabled: BoolProperty("Enable Near Fog")
    near_fog_1_start: FloatProperty("Near Fog Start", 0, 0)
    near_fog_2_stop: FloatProperty("Near Fog Stop", 10, 0)
    near_fog_3_color: RGBAColorProperty("Near Fog Color")
    near_fog_4_strength: RangeProperty("Near Fog Strength", 1)
    
    far_fog_0_enabled: BoolProperty("Enable Far Fog")
    far_fog_1_start: FloatProperty("Far Fog Start", 0, 0)
    far_fog_2_stop: FloatProperty("Far Fog Stop", 10, 0)
    far_fog_3_color: RGBAColorProperty("Far Fog Color")
    far_fog_4_strength: RangeProperty("Far Fog Strength", 1)
    
    ambient_0_color: RGBColorProperty("Ambient Light Color", (1, 1, 1))
    ambient_1_strength: FloatProperty("Ambient Light Strength", 1, 0)
    
    # This is why you should never be lazy
    
    def render(self, target):
        layout = target.layout
        layout.prop(self, "camera")
        layout.column()
        layout.label(text="Near Fog Effect")
        self.render_child(target, "near_fog_")
        layout.label(text="Far Fog Effect")
        self.render_child(target, "far_fog_")
        layout.label(text="Ambient Light")
        self.render_child(target, "ambient_")

class SceneExportProperties(BaseProperties):
    
    dir_path: DirectoryPathProperty("Output Folder", "", set(["OUTPUT_PATH"])) 
    file_name: FileNameProperty("Scene File Name") 
    
    over_obj_export: BoolProperty("Override Object Export Properties", True)
    apply_modifiers: BoolProperty("Apply Modifiers", True)
    embed_objects: BoolProperty("Embed Objects", True)
    embed_meshes: BoolProperty("Embed Meshes", True)
    embed_textures: BoolProperty("Embed Images", True)
    no_hex_color: BoolProperty("Don't use Hex Color", False)
    
    tx_dir_path: DirectoryPathProperty("Mesh Folder", "tx", set(["OUTPUT_PATH"])) 
    mesh_dir_path: DirectoryPathProperty("Images Folder", "mesh", set(["OUTPUT_PATH"])) 
    
    def render(self, target):
        layout = target.layout
        layout.label(text="Path")
        layout.prop(self, "dir_path")
        layout.prop(self, "file_name")
        layout.label(text="Settings")
        layout.prop(self, "embed_objects")
        layout.prop(self, "over_obj_export")
        if self.over_obj_export:
            layout.prop(self, "apply_modifiers")
            layout.prop(self, "embed_meshes")
            if not self.embed_meshes:
                layout.prop(self, "mesh_dir_path")
            layout.prop(self, "embed_textures")
            if not self.embed_textures:
                layout.prop(self, "tx_dir_path")
            layout.prop(self, "no_hex_color")
        layout.column()
        pass

"""
    ---------

    Operators

    ---------
"""

class EXPORT_OT_ExportSceneObjectOperator(bt.Operator):
    bl_label = "Export Scene Object to File (.mrod)"
    bl_idname = "object.export_to_mrod"

    def execute(self, context):
        export_props = context.object.object_export_props
        objdef = create_render_definition(
            context,
            context.object,
            export_props.file_name,
            export_props.dir_path,
            export_props.tx_dir_path,
            export_props.mesh_dir_path,
            export_props.embed_textures,
            export_props.embed_meshes,
            export_props.apply_modifiers,
            not export_props.no_hex_color
        )
        path = f"{export_props.dir_path}\\{export_props.file_name}.mrod"
        with open(path, "wt") as f:
            f.write(json.dumps(objdef, indent="\t"))
        return {"FINISHED"}

class EXPORT_OT_ExportSceneOperator(bt.Operator):
    bl_label = "Export Scene to File (.msd)"
    bl_idname = "scene.export_to_msd"

    def execute(self, context):
        scene_props = context.scene.scene_props
        export_props = context.scene.scene_export_props
        def hex_color_or_array(color, color_func=as_hex_string):
            return color_func(color) if not export_props.no_hex_color else [x for x in color]
        objects = [obj for obj in context.scene.objects if obj.type == "MESH"]
        print(f"Objects: {len(objects)}")
        make_if_not_exists(export_props.dir_path)
        camera = scene_props.camera.data
        cpos, crot, cscale = scene_props.camera.matrix_world.decompose()
        crot = crot.to_euler('YXZ')
        rset = context.scene.render
        caspect = [rset.resolution_x / rset.resolution_y, 1]
        ortho_size = camera.ortho_scale
        scenedef = {
            "camera": {
                "type": "GIMBAL",
                "position": [-cpos.x, cpos.y, cpos.z],
                "rotation": [crot.x, crot.y, crot.z],
                "fov": camera.angle,
                "zNear": camera.clip_start,
                "zFar": camera.clip_end,
                "aspect": caspect,
                "ortho": {
                    "strength": 1.0 if camera.type == "ORTHO" else 0.0,
                    "origin": [0.0, 0.0],
                    "size": [ortho_size * caspect[0], ortho_size * caspect[1]]
                }
            }
        }
        scenedef["world"] = {
            "ambient": {
                "color": hex_color_or_array(scene_props.ambient_0_color, as_hex_string_rgb),
                "strength": scene_props.ambient_1_strength
            }
        }
        if scene_props.near_fog_0_enabled:
            world["nearFog"] = {
                "enabled": scene_props.near_fog_0_enabled,
                "start": scene_props.near_fog_1_start,
                "stop": scene_props.near_fog_2_stop,
                "color": hex_color_or_array(scene_props.near_fog_3_color),
                "strength": scene_props.near_fog_4_strength
            }
        if scene_props.far_fog_0_enabled:
            world["farFog"] = {
                "enabled": scene_props.far_fog_0_enabled,
                "start": scene_props.far_fog_1_start,
                "stop": scene_props.far_fog_2_stop,
                "color": hex_color_or_array(scene_props.far_fog_3_color),
                "strength": scene_props.far_fog_4_strength
            }
        if export_props.embed_objects:
            scenedef["data"] = {}
        else:
            scenedef["path"] = []
        for obj in objects:
            obj_name = obj.object_export_props.file_name
            obj_name = obj_name if (obj_name is not None) and (obj_name != "") else obj.name
            if export_props.over_obj_export:
                obj_name = obj.name
            rendef = None
            if export_props.over_obj_export:
                rendef = create_render_definition(
                    context,
                    obj,
                    obj_name,
                    export_props.dir_path,
                    export_props.tx_dir_path,
                    export_props.mesh_dir_path,
                    export_props.embed_textures,
                    export_props.embed_meshes,
                    export_props.apply_modifiers,
                    not export_props.no_hex_color
                )
            else:
                rendef = create_render_definition(
                    context,
                    obj,
                    obj_name,
                    obj.object_export_props.dir_path,
                    obj.object_export_props.tx_dir_path,
                    obj.object_export_props.mesh_dir_path,
                    obj.object_export_props.embed_textures,
                    obj.object_export_props.embed_meshes,
                    obj.object_export_props.apply_modifiers,
                    not obj.object_export_props.no_hex_color
                )
            if export_props.embed_objects:
                scenedef["data"][obj.name] = rendef
            else:
                make_if_not_exists(f"{export_props.dir_path}\\{obj_name}");
                defpath = f"{export_props.dir_path}\\{obj_name}\\{obj_name}.mrod"
                with open(defpath, "wt") as f:
                    f.write(json.dumps(rendef, indent="\t"))
                scenedef["path"].append({
                    "source": f"{obj_name}\\{obj_name}.mrod",
                    "type": "MROD"
                })
            path = f"{export_props.dir_path}\\{export_props.file_name}.msd"
            with open(path, "wt") as f:
                f.write(json.dumps(scenedef, indent="\t"))
        return {"FINISHED"}

"""
    ------

    Panels

    ------
"""

class SCENEOBJECT_PT_SceneObjectMaterialPanel(PropertyPanel):
    bl_label = "Material Properties"
    bl_idname = "SO0_OBJECT_PT_SceneObjectMaterialPanel"
    bl_category = "Scene Object"
    
    def draw(self, context):
        obj = context.object
        if obj.type != "MESH":
            return
        layout = self.layout
        props = context.object.material_props
        props.render(self)
        layout.column()

class SCENEOBJECT_PT_SceneObjectBlendPanel(PropertyPanel):
    bl_label = "Blend Properties"
    bl_idname = "SO1_SCENEOBJECT_PT_SceneObjectBlendPanel"
    bl_category = "Scene Object"
    
    def draw(self, context):
        obj = context.object
        if obj.type != "MESH":
            return
        layout = self.layout
        props = context.object.blend_props
        props.render(self)
        layout.column()

class SCENEOBJECT_PT_SceneObjectExportPanel(PropertyPanel):
    bl_label = "Export Properties"
    bl_idname = "SO2_SCENEOBJECT_PT_SceneObjectExportPanel"
    bl_category = "Scene Object"
    
    def draw(self, context):
        obj = context.object
        if obj.type != "MESH":
            return
        layout = self.layout
        props = context.object.object_export_props
        props.render(self)
        layout.label(text="Export")
        layout.operator("object.export_to_mrod", text="Export Object")
        layout.column()
        

class SCENE_PT_ScenePanel(PropertyPanel):
    bl_label = "Scene Properties"
    bl_idname = "TOOL_PT_ScenePanel"
    bl_category = "Scene"
    
    def draw(self, context):
        obj = context.object
        layout = self.layout
        props = context.scene.scene_props
        props.render(self)

class SCENE_PT_SceneExportPanel(PropertyPanel):
    bl_label = "Export Properties"
    bl_idname = "TOOL_PT_SceneExportPanel"
    bl_category = "Scene"
    
    def draw(self, context):
        obj = context.object
        layout = self.layout
        props = context.scene.scene_export_props
        props.render(self)
        layout.label(text="Export")
        layout.operator("scene.export_to_msd", text="Export Scene")
        layout.column()

"""
    ------------------

    Class Registration

    ------------------
"""

classes = (
    ObjectMaterialProperties,
    ObjectBlendProperties,
    ObjectExportProperties,
    SceneProperties,
    SceneExportProperties,
    EXPORT_OT_ExportSceneOperator,
    EXPORT_OT_ExportSceneObjectOperator,
    SCENEOBJECT_PT_SceneObjectMaterialPanel,
    SCENEOBJECT_PT_SceneObjectBlendPanel,
    SCENEOBJECT_PT_SceneObjectExportPanel,
    SCENE_PT_ScenePanel,
    SCENE_PT_SceneExportPanel
)
    
def register():
    from bpy.utils import register_class
    for cls in classes:
        register_class(cls)
    
    bt.Object.material_props = bp.PointerProperty(type=ObjectMaterialProperties)
    bt.Object.blend_props = bp.PointerProperty(type=ObjectBlendProperties)
    bt.Object.object_export_props = bp.PointerProperty(type=ObjectExportProperties)
    bt.Scene.scene_props = bp.PointerProperty(type=SceneProperties)
    bt.Scene.scene_export_props = bp.PointerProperty(type=SceneExportProperties)

def unregister():
    from bpy.utils import unregister_class
    for cls in reversed(classes):
        unregister_class(cls)
    
    del bt.Object.material_props
    del bt.Object.blend_props
    del bt.Object.object_export_props
    del bt.Scene.scene_props
    del bt.Scene.scene_export_props


if __name__ == "__main__":
    register()