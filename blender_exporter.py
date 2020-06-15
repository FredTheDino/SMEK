import bpy
import bmesh
import mathutils

def error_box(context, title, text):
    icon = "ERROR"
    def draw(self, context):
        self.layout.label(text=text)
    context.window_manager.popup_menu(draw, title=title, icon=icon)

def to_tris(obj):
    mesh = bmesh.new()
    mesh.from_mesh(obj.data)

    bmesh.ops.triangulate(mesh, faces=mesh.faces[:])
    return mesh

def only_three(weights):
    most_important = (sorted(weights, reverse=True, key=lambda x: x[1])
                      + [(0, 0)] * 3)[:3]
    total_weight = sum(map(lambda x: x[1], most_important))
    normalized = map(lambda x: (x[0], x[1] / total_weight), most_important)
    return normalized

def write_animation(context, filepath, obj, armature):
    mesh = to_tris(obj)
    uv_lay = mesh.loops.layers.uv.active
    layer = mesh.verts.layers.deform.verify()
    with open(filepath, 'w', encoding='utf-8') as f:
        for face in mesh.faces:
            for loop in face.loops:
                vert = loop.vert
                f.write(f"p:%f %f %f " % vert.co[:])
                f.write(f"n:%f %f %f " % vert.normal[:])
                uv = loop[uv_lay].uv
                weights = only_three(vert[layer].items())
                for i, w in weights:
                    f.write("w: %d %f " % (i, w))
                f.write("\n")
        mesh.free()

        all_bones = {x[0]: [i, x[0], x[1], []] for i, x in enumerate(armature.pose.bones.items())}
        root = ""
        for name, bone in list(all_bones.items()):
            parent = bone[2].parent
            if parent:
                all_bones[parent.name][3].append(bone)
            else:
                root = name
        
        def write_bones(node, parent=-1):
            index, name, bone, children = node
            mat = bone.matrix
            scale = mat.to_scale()
            rotation = mat.to_quaternion()
            translation = mat.to_translation()
            f.write(f"b {parent} {index} {name} {scale} {rotation} {translation}\n")
            for child in children:
                write_bones(child, index)

        write_bones(all_bones[root])
        
        def write_animation(nodes):
            sorted_bones = list(sorted(nodes.values()))
            frame_range = armature.animation_data.action.frame_range
            frames = int(frame_range.y - frame_range.x + 1)
            
            def bake_transform(frame, bone):
                bpy.context.scene.frame_set(frame)
                mat = bone.matrix
                scale = mat.to_scale()
                rotation = mat.to_quaternion()
                translation = mat.to_translation()
                return f"{scale} {rotation} {translation}"
            
            transforms = [[bake_transform(frame, bone)
                            for _, _, bone, _ in sorted_bones]
                            for frame in range(frames)]
            
            f.write(f"a {transforms}")
            
        write_animation(all_bones) 
        
    return {'FINISHED'}


# ExportHelper is a helper class, defines filename and
# invoke() function which calls the file selector.
from bpy_extras.io_utils import ExportHelper
from bpy.props import StringProperty, BoolProperty, EnumProperty
from bpy.types import Operator


class ExportSomeData(Operator, ExportHelper):
    """This appears in the tooltip of the operator and in the generated docs"""
    bl_idname = "export_test.some_data"  # important since its how bpy.ops.import_test.some_data is constructed
    bl_label = "Export Some Data"

    # ExportHelper mixin class uses this
    filename_ext = ".edme"

    filter_glob: StringProperty(
        default="*" + filename_ext,
        options={'HIDDEN'},
        maxlen=255,  # Max internal buffer length, longer would be clamped.
    )

    def execute(self, context):
        objs = context.selected_objects
        if not objs:
            error_box(context, "Error exporting", "Cannot export if nothing is selected")
            return {'CANCELLED'}
        # Only export first object
        obj = objs[0]
        armature = objs[1]
        return write_animation(context, self.filepath, obj, armature)


# Only needed if you want to add into a dynamic menu
def menu_func_export(self, context):
    self.layout.operator(ExportSomeData.bl_idname, text="Text Export Operator")


def register():
    bpy.utils.register_class(ExportSomeData)
    bpy.types.TOPBAR_MT_file_export.append(menu_func_export)


def unregister():
    bpy.utils.unregister_class(ExportSomeData)
    bpy.types.TOPBAR_MT_file_export.remove(menu_func_export)


if __name__ == "__main__":
    register()

    # test call
    bpy.ops.export_test.some_data('INVOKE_DEFAULT')
