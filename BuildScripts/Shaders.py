import subprocess, json, os
from pathlib import Path

ignore_file_extensions = []

def compile(project_name : str, vulkan_sdk : str, shader_extensions : list[str], compiler : json, directories : json, script_dir : Path):
    shader_dir = directories["shaders"]
    content_dir = directories["content"]
    solution_dir = script_dir / directories["solution"]

    input_dir = Path(solution_dir) / Path(project_name) / Path(content_dir) / Path(shader_dir)
    collected_files = [
        file for file in input_dir.rglob("*")
        if file.is_file() and file.suffix.lower()[1:] in shader_extensions
    ]

    vulkan_bin = Path(os.path.expandvars(vulkan_sdk)) / 'Bin/'
    compiler_executable = vulkan_bin / Path(str(compiler["name"]) + '.exe')

    for file in collected_files:
        out_file = Path(input_dir) / Path(str(file.stem) + ".spv")

        file_contents = file.read_text(encoding="utf-8")
        # this is a little hacky, but given the way slang works...
        if file_contents.find('[shader("') == -1:
            print('[INFO] Skipping: ' + file.stem + file.suffix.lower() + ' due to not containing an entry-point!')
            continue

        print('[INFO] Compiling: ' + file.stem + file.suffix.lower() + ' -> ' + file.stem + '.spv')

        run = [
            str(compiler_executable)
        ]

        run.extend(compiler["flags"])
        run.extend(["-o", str(out_file),
            str(file)])

        subprocess.run(run)
