import argparse, json, Shaders, Resources, Textures
from pathlib import Path

parser = argparse.ArgumentParser()
parser.add_argument("output", help="The output folder for the copied.", type=str)
args = parser.parse_args()

script_dir : Path = Path(__file__).resolve().parent
with open(str(script_dir) + '/options.json') as json_data:
    data = json.load(json_data)

    project = data["project_name"]
    vulkan_sdk = data["vulkan_sdk"]
    ktx_sdk = data["ktx_sdk"]
    shader_ext = data["shader_extensions"]
    texture_ext = data["texture_extensions"]
    shader_compiler = data["shader_compiler"]
    texture_transcoder = data["texture_transcoder"]
    directories = data["directories"]

    Resources.ignore_file_extensions.extend(shader_ext)
    Resources.ignore_file_extensions.extend(texture_ext)
    Textures.ignore_file_extensions.extend(texture_ext)

    Textures.transcode(project, ktx_sdk, texture_ext, texture_transcoder, directories, script_dir)
    Shaders.compile(project, vulkan_sdk, shader_ext, shader_compiler, directories, script_dir)
    Resources.pack(project, directories, script_dir, "Resources")
    Resources.copy(project, args.output, "Resources", directories, script_dir)