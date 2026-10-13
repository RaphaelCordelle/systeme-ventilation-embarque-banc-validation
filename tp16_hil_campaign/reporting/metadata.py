import platform
import subprocess


def get_metadata() -> dict:
    metadata = {
        "python_version": platform.python_version(),
        "git_commit": "unknown",
    }

    try:
        git_commit = subprocess.check_output(
            [
                "git",
                "rev-parse",
                "--short",
                "HEAD",
            ],
            text=True,
        ).strip()

        metadata["git_commit"] = git_commit

    except Exception:
        pass

    return metadata