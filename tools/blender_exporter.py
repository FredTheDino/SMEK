# The edanim exporter, exports skinned meshes to a special
# format that is easy to parse and is written in plain text.
#
# This code is heavily based on the standard blender exporter
# and gives a very handy way to export data.
#
# There is a lot of work left to do if this is to become a
# "fully featured" exporter.
#
# The file format this generates, specifies what each line
# does, currently its "geo:" for geometry data. "arm:" for
# armature "anim:##name##:" is the animation with the name
# "##name##"
#
# For more details, I'd recomend you to just read the code,
# it will probably be faster.
#
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
        verts = []
        for face in mesh.faces:
            for loop in face.loops:
                vert = loop.vert
                verts.extend(vert.co[:])
                verts.extend(vert.normal[:])
                uv = loop[uv_lay].uv
                verts.extend(uv[:])
                weights = only_three(vert[layer].items())
                for i, w in weights:
                    verts.append(float(i))
                    verts.append(w)
        # Writes the position data
        f.write("geo:")
        f.write(" ".join(map(str, verts)))
        f.write("\n")
        mesh.free()

        bones = armature.pose.bones
        all_bones = {x[0]: [x[0], x[1], []] for x in bones.items()}
        root = ""
        for name, bone in list(all_bones.items()):
            parent = bone[1].parent
            if parent:
                all_bones[parent.name][2].append(bone)
            else:
                root = name

        # Exports bone structure.
        index_counter = 0
        def order_bones(node, parent=-1):
            nonlocal index_counter
            index = index_counter
            index_counter += 1

            name, bone, children = node
            order = [(parent, index, name, bone)]
            for child in children:
                order += order_bones(child, index)
            return order

        def serialize_bone(node):
            parent, index, name, bone = node
            mat = bone.bone.matrix_local
            translation, rotation, scale = mat.decompose()
            return f"{parent} {index} {name.replace(' ', '_')} "\
                   f"{scale.x} {scale.y} {scale.z} "\
                   f"{rotation.x} {rotation.y} {rotation.z} {rotation.w} "\
                   f"{translation.x} {translation.y} {translation.z}"

        ordered_bones = order_bones(all_bones[root])
        f.write("arm:")
        f.write("|".join(map(serialize_bone, ordered_bones)))
        f.write("\n")

        # Exporting of animations
        def write_animation(action, nodes):
            def bake_transform(frame, bone):
                # TODO(ed): This can be moved out. But meh...
                bpy.context.scene.frame_set(frame)
                mat = bone.matrix.copy()
                if bone.parent is not None and bone.parent.matrix is not None:
                    other = bone.parent.matrix.copy()
                    other.invert()
                    mat = other @ mat
                translation, rotation, scale = mat.decompose()
                return f"{scale.x} {scale.y} {scale.z} "\
                       f"{rotation.x} {rotation.y} {rotation.z} {rotation.w} "\
                       f"{translation.x} {translation.y} {translation.z}"

            times = set()
            for c in action.fcurves:
                for p in c.keyframe_points:
                    times.add(int(p.co.x))

            start_frame = bpy.context.scene.frame_current
            transforms = [(t, [bake_transform(t, bone)
                           for _, _, _, bone in nodes])
                           for t in sorted(times)]
            bpy.context.scene.frame_set(start_frame)

            output = ";".join([f"{t}=" + "|".join(trans) for t, trans in transforms])
            f.write(f"anim:{action.name_full}:{output}\n")

        # TODO(ed): Find the other actions
        write_animation(armature.animation_data.action, ordered_bones)
        # TODO(ed):
        # TL;DR: We need bezier curve support.
        #
        # If we want more control, we could export each
        # channel separately, by parsing the fcurves directly.
        # This would give a lot more control over the animation,
        # but that would be substantially more work. I have some
        # sketches of a storage format, which I think would be
        # quite optimal in the engine as well.
        #
        # The key is
        # parsing the string from this value:
        #
        # action.fcurves[0].data_path <- Name ends with
        # paired with:
        # action.fcurves[0].array_index
        #
        # This gives us a way to map each datapoint to a meaningfull
        # value, there might be another way, but I haven't found it.
        # (Don't ask how long this took to find.)
        #
        # The problem right now, is that the view in blender will
        # not match what is going on in the engine, since the
        # bezier values aren't being passed on.
        #
        # We could also just, dump every frame of animation, and hope
        # we can handle it. It shouldn't affect the animation workflow
        # if we switch exporter mid development.
        #
        # TODO(ed):
        # It would also be cool if we could save the animations to
        # a seperate file format

    return {'FINISHED'}


# ExportHelper is a helper class, defines filename and
# invoke() function which calls the file selector.
from bpy_extras.io_utils import ExportHelper
from bpy.props import StringProperty, BoolProperty, EnumProperty
from bpy.types import Operator


class EdAnimExporter(Operator, ExportHelper):
    """Exports a skeleton and animations in a easy to read format. (For me)"""
    bl_idname = "edanim_exporter.all"  # important since its how bpy.ops.import_test.some_data is constructed
    bl_label = "Exports a skeleton and animations"

    filename_ext = ".edan"

    filter_glob: StringProperty(
        default="*" + filename_ext,
        options={'HIDDEN'},
        maxlen=255,  # Max internal buffer length, longer would be clamped.
    )

    def execute(self, context):
        objs = context.selected_objects
        if len(objs) != 2:
            error_box(context, "Error exporting", "Can only export if you select the bone and the armature.")
            return {'CANCELLED'}

        armature, obj = sorted(objs, key=lambda x: x.type)
        return write_animation(context, self.filepath, obj, armature)


# Only needed if you want to add into a dynamic menu
def menu_func_export(self, context):
    self.layout.operator(EdAnimExporter.bl_idname, text="Ed Animation (edan)")


def register():
    bpy.utils.register_class(EdAnimExporter)
    bpy.types.TOPBAR_MT_file_export.append(menu_func_export)


def unregister():
    bpy.utils.unregister_class(EdAnimExporter)
    bpy.types.TOPBAR_MT_file_export.remove(menu_func_export)


if __name__ == "__main__":
    register()

    # test call
    bpy.ops.edanim_exporter.all('INVOKE_DEFAULT')
