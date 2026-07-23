import subprocess, json, os
from pathlib import Path

ignore_file_extensions = []

def transcode(project_name : str, ktx_sdk : str, texture_extensions : list[str], transcoder : json, directories : json, script_dir : Path):
    texture_dir = directories["textures"]
    content_dir = directories["content"]
    solution_dir = script_dir / directories["solution"]

    input_dir = Path(solution_dir) / Path(project_name) / Path(content_dir) / Path(texture_dir)
    collected_files = [
        file for file in input_dir.rglob("*")
        if file.is_file() and file.suffix.lower()[1:] in texture_extensions
    ]

    transcoder_executable = Path(os.path.expandvars(ktx_sdk)) / Path("bin") / Path(transcoder["name"])

    for file in collected_files:
        out_file = Path(input_dir) / Path(str(file.stem) + ".ktx2")
        
        print('[INFO] Transcoding: ' + file.stem + file.suffix.lower() + ' -> ' + file.stem + '.ktx2')

        run = [
            str(transcoder_executable)
        ]

        run.extend(["create"])
        run.extend(transcoder["flags"])
        run.extend([str(file), str(out_file)])

        subprocess.run(run)